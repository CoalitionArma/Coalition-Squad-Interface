//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-21
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Builds the top-down outline of a vehicle's hull, turret or weapon rack over a few frames by
//! 				tracing down through a grid over it; falls back to its colliders' hull, then its bounds.
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutlineBuilder
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected static const float HULL_CELL_SIZE = 0.15;			//!< m, minimum for a hull or weapon rack
	protected static const float TURRET_CELL_SIZE = 0.1;		//!< m, minimum for a turret or gun mount
	protected static const float GUN_CELL_SIZE = 0.05;			//!< m, minimum for a mounted gun (thin barrels)
	protected static const int REFINE_STEPS = 2;				//!< halvings of each edge point's cell edge (to a quarter)
	protected static const int MAX_CELLS = 64;					//!< traces along the longer side at most
	protected static const float SWEEP_RADIUS = 0.5;			//!< cells; sphere radius of turret traces, so thin barrels aren't missed
	protected static const float TRACE_MARGIN = 0.5;			//!< m above and below the bounds
	protected static const float SKID_HEIGHT = 0.5;				//!< m above a skid helicopter's bottom where traces stop, leaving out the skids
	protected static const float MAIN_ROTOR_SHARE = 0.5;		//!< of the largest rotor's radius, from which a rotor counts as main
	protected static const float ROTOR_AXIS_FIT = 0.7;			//!< min share of a rotor's axis along up (main) or sideways (tail) to keep it
	protected static const float ROTOR_HALF_THICKNESS = 0.6;	//!< m either side of a rotor's plane its blades may sag or cone
	protected static const float WHEEL_WIDTH_SHARE = 0.6;		//!< a wheel's width, in radii (the simulation has no width)
	protected static const float MIN_WHEEL_WIDTH = 0.2;			//!< m
	protected static const float MAX_WHEEL_WIDTH = 0.5;			//!< m
	protected static const float TRACK_WIDTH = 0.5;				//!< m, a track's width
	protected static const float SIMPLIFY_TOLERANCE = 0.04;		//!< m a simplified line may stray from the traced one
	protected static const float MIN_CONTOUR_AREA = 3;			//!< cells; smaller pieces (mirrors, antennas) are left out
	protected static const float COLLIDER_BOUNDS_MARGIN = 0.5;	//!< m colliders may reach past the meshes' bounds
	protected static const float DOORS_CLOSED_TIME = 1;			//!< seconds every door must be closed before tracing (done swinging shut)
	protected static const float GUN_SAMPLE_SHARE = 0.85;		//!< of the main gun's length, from its back, where IsGunTraced traces
	protected static const float GUN_SAMPLE_HEIGHT = 2;			//!< m above and below the gun IsGunTraced traces

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected IEntity m_Root;
	protected TurretComponent m_Turret;				//!< the frame follows its aim; null for a hull or weapon rack
	protected ref array<IEntity> m_aParts = {};		//!< the root and its parts with physics, without pieces of their own below it
	protected ref TraceParam m_TraceParam;
	protected ref CSI_VehicleOutline m_Outline;		//!< set once built
	protected bool m_bCacheable;					//!< the outline may be kept for the prefab (not a wreck)
	protected ref array<int> m_aPieceGunLayers = {};	//!< collider layers of gun pieces below the root (AddPieceGunLayers)
	protected string m_sShapeKey;					//!< GetShapeKey, empty if it can't be shared

	// Doors
	protected ref array<BaseCompartmentManagerComponent> m_aCompartmentManagers = {};	//!< whose doors must be closed
	protected bool m_bDoorOpen;						//!< a door was open at the last UpdateDoors
	protected float m_fDoorsClosedTime;				//!< seconds every door has been closed

	// Turn-out hatches, open while their seat's occupant is turned out: the seat, the hatch's manager and anim door index
	protected ref array<BaseCompartmentSlot> m_aTurnOutSlots = {};
	protected ref array<BaseCompartmentManagerComponent> m_aTurnOutManagers = {};
	protected ref array<int> m_aTurnOutDoors = {};

	// Trace grid in the outline's frame
	protected vector m_vMins;
	protected vector m_vMaxs;
	protected float m_fCellSize;
	protected int m_iColumns;						//!< traces along x, with the empty ring
	protected int m_iRows;							//!< traces along z, with the empty ring
	protected ref array<bool> m_aHits = {};			//!< by row, then column
	protected ref array<bool> m_aStamped = {};		//!< cells filled by a wheel or track, not traced
	protected ref array<bool> m_aVisited = {};		//!< cells the flood from the ring has reached
	protected ref array<int> m_aFloodStack = {};	//!< cells reached and not traced yet
	protected bool m_bSeeded;						//!< the flood has started
	protected bool m_bGridDone;

	// Root space: rotors to skip (position, axis, (radius, is main)), wheels and tracks to stamp (min x, max x, min z, max z)
	protected ref array<vector> m_aRotors = {};
	protected bool m_bSkids;	//!< a helicopter on skids, left out of the traces
	protected ref array<float> m_aWheelRectangles = {};

	// The frame in the world, set each step
	protected BaseWorld m_World;
	protected vector m_vFrameX;
	protected vector m_vFrameY;
	protected vector m_vFrameZ;
	protected vector m_vFrameOrigin;
	protected vector m_vFramePivot;		//!< CSI_VehicleOutline.GetFramePivot's, in the root's space
	protected float m_fFrameCos = 1;	//!< of the frame's turn from the root
	protected float m_fFrameSin;

	// Grid stages finished one a frame, then marching squares: each edge point's two neighbors and crossing fraction
	protected bool m_bGapsClosed;
	protected bool m_bHolesFilled;
	protected bool m_bLinked;
	protected ref map<int, int> m_mFirstNeighbors = new map<int, int>();
	protected ref map<int, int> m_mSecondNeighbors = new map<int, int>();
	protected ref array<int> m_aEdgeKeys = {};
	protected int m_iNextEdge;
	protected ref map<int, float> m_mEdgeFractions = new map<int, float>();

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] root a vehicle, turret or weapon rack
	//! \param[in] turret null for a hull or weapon rack
	//! \param[in] framePivot from CSI_VehicleOutline.GetFramePivot, in the root's space
	void CSI_VehicleOutlineBuilder(notnull IEntity root, TurretComponent turret, vector framePivot)
	{
		m_Root = root;
		m_Turret = turret;
		m_vFramePivot = framePivot;
		m_aParts.Insert(root);
		AddParts(root);
		AddRotors();
		AddWheels();
		AddTurnOutSlots(GetVehicle(root));

		// A hull's grid covers only its own model and wheels, cutting off cages around it; turrets and racks cover all parts
		bool hasBounds;
		if (!turret && !CSI_VehicleOutline.IsWeaponRack(root))
			hasBounds = AddBounds(root, true);

		foreach (IEntity part : m_aParts)
		{
			BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(part.FindComponent(BaseCompartmentManagerComponent));
			if (compartmentManager)
				m_aCompartmentManagers.Insert(compartmentManager);
		}

		// Without a model of its own, every part's bounds
		if (!hasBounds)
		{
			foreach (IEntity boundsPart : m_aParts)
			{
				if (AddBounds(boundsPart, !hasBounds))
					hasBounds = true;
			}
		}

		// A turret's main gun too: merged into it (MergePhysics), it isn't a part but may reach past the turret
		IEntity turretGun;
		if (turret && !root.FindComponent(WeaponComponent))
			turretGun = GetMainGun(root);

		if (turretGun && !m_aParts.Contains(turretGun) && AddBounds(turretGun, !hasBounds))
			hasBounds = true;

		AddPieceGunLayers(root);

		if (!hasBounds)
			return;

		// Wheels and tracks sit at the edge, maybe past the meshes
		for (int wheel = 0, wheelCount = m_aWheelRectangles.Count() / 4; wheel < wheelCount; wheel++)
		{
			m_vMins[0] = Math.Min(m_vMins[0], m_aWheelRectangles[wheel * 4]);
			m_vMaxs[0] = Math.Max(m_vMaxs[0], m_aWheelRectangles[wheel * 4 + 1]);
			m_vMins[2] = Math.Min(m_vMins[2], m_aWheelRectangles[wheel * 4 + 2]);
			m_vMaxs[2] = Math.Max(m_vMaxs[2], m_aWheelRectangles[wheel * 4 + 3]);
		}

		// Finer for turrets, finest for guns, whose barrels are thin
		float minCellSize = HULL_CELL_SIZE;
		if (root.FindComponent(WeaponComponent))
			minCellSize = GUN_CELL_SIZE;
		else if (turret)
			minCellSize = TURRET_CELL_SIZE;

		// A turret's frame turns with its aim: cover its bounds both as they are and turned by the aim
		if (turret)
		{
			float frameYaw = CSI_VehicleOutline.GetFrameYaw(root, turret);
			float cosYaw = Math.Cos(frameYaw);
			float sinYaw = Math.Sin(frameYaw);
			vector mins = m_vMins - m_vFramePivot;
			vector maxs = m_vMaxs - m_vFramePivot;
			m_vMins = mins;
			m_vMaxs = maxs;

			for (int corner = 0; corner < 4; corner++)
			{
				// Corners 0, 1, 4 and 5: the bottom four
				vector point = GetBoxCorner(mins, maxs, (corner & 1) + (corner & 2) * 2);
				float x = point[0] * cosYaw - point[2] * sinYaw;
				float z = point[0] * sinYaw + point[2] * cosYaw;
				m_vMins[0] = Math.Min(m_vMins[0], x);
				m_vMins[2] = Math.Min(m_vMins[2], z);
				m_vMaxs[0] = Math.Max(m_vMaxs[0], x);
				m_vMaxs[2] = Math.Max(m_vMaxs[2], z);
			}
		}

		vector size = m_vMaxs - m_vMins;
		m_fCellSize = Math.Max(minCellSize, Math.Max(size[0], size[2]) / MAX_CELLS);

		// An untraced ring of empty cells around the bounds, so gaps can be closed up to the edge
		m_iColumns = Math.Ceil(size[0] / m_fCellSize) + 2;
		m_iRows = Math.Ceil(size[2] / m_fCellSize) + 2;
		int cellCount = m_iColumns * m_iRows;
		m_aHits.Resize(cellCount);
		m_aStamped.Resize(cellCount);
		m_aVisited.Resize(cellCount);

		// Sphere traces for a turret, whose barrel may be thinner than a cell
		if (turret)
		{
			TraceSphere sweep = new TraceSphere();
			sweep.Radius = m_fCellSize * SWEEP_RADIUS;
			m_TraceParam = sweep;
		}
		else
		{
			m_TraceParam = new TraceParam();
		}

		m_TraceParam.Flags = TraceFlags.ENTS | TraceFlags.ANY_CONTACT;
		m_TraceParam.LayerMask = GetTraceLayers();
		if (m_aParts.Count() == 1)
			m_TraceParam.Include = root;
		else
			m_TraceParam.IncludeArray = m_aParts;

		// Add the part layers only if the traces miss the turret's gun: a merged gun may collide only there
		if (turretGun && !IsGunTraced(turretGun))
			m_TraceParam.LayerMask = m_TraceParam.LayerMask | GetPartLayers();

		// A part of the same shape was built already: nothing to trace
		m_sShapeKey = GetShapeKey(minCellSize);
		CSI_VehicleOutline shared = CSI_VehicleOutline.GetShared(m_sShapeKey);
		if (!shared)
			return;

		m_Outline = shared;
		m_bCacheable = !IsWrecked();

	}

	//------------------------------------------------------------------------------------------------
	//! Adds the parent's descendants with physics, leaving out crew, stored items and pieces of their own.
	//! \param[in] parent
	protected void AddParts(notnull IEntity parent)
	{
		IEntity child = parent.GetChildren();
		while (child)
		{
			if (CSI_VehicleOutline.IsOutlinedPart(child) && !CSI_VehicleOutline.IsOwnPiece(child))
			{
				if (child.GetPhysics())
					m_aParts.Insert(child);

				AddParts(child);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Notes the root's rotors, whose blades are left out of the traces.
	protected void AddRotors()
	{
		VehicleHelicopterSimulation helicopter = VehicleHelicopterSimulation.Cast(m_Root.FindComponent(VehicleHelicopterSimulation));
		if (!helicopter)
			return;

		m_bSkids = helicopter.LandingGearGetType() == LandingGearType.SKID;

		int rotorCount = helicopter.RotorCount();
		float largestRadius;
		for (int rotor = 0; rotor < rotorCount; rotor++)
		{
			largestRadius = Math.Max(largestRadius, helicopter.RotorGetRadius(rotor));
		}

		// Axes may be misreported: tell main rotors by size, keep an axis only if mostly up (main) or sideways (tail)
		for (int rotor = 0; rotor < rotorCount; rotor++)
		{
			vector position, axis;
			helicopter.RotorGetPosition(rotor, position);
			helicopter.RotorGetAxis(rotor, axis);
			axis.Normalize();

			float radius = helicopter.RotorGetRadius(rotor);
			bool isMain = radius >= largestRadius * MAIN_ROTOR_SHARE;
			if (isMain && Math.AbsFloat(axis[1]) < ROTOR_AXIS_FIT)
				axis = vector.Up;
			else if (!isMain && Math.AbsFloat(axis[0]) < ROTOR_AXIS_FIT)
				axis = vector.Right;

			m_aRotors.Insert(position);
			m_aRotors.Insert(axis);
			float mainFlag = 0;
			if (isMain)
				mainFlag = 1;

			m_aRotors.Insert(Vector(radius, mainFlag, 0));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Notes the root's wheels and tracks, stamped in as they have no colliders the traces can hit.
	protected void AddWheels()
	{
		VehicleWheeledSimulation wheeled = VehicleWheeledSimulation.Cast(m_Root.FindComponent(VehicleWheeledSimulation));
		if (wheeled)
		{
			for (int wheel = 0, wheelCount = wheeled.WheelCount(); wheel < wheelCount; wheel++)
			{
				AddWheelRectangle(wheeled.WheelGetPosition(wheel), wheeled.WheelGetRadius(wheel));
			}

			return;
		}

		VehicleWheeledSimulation_SA_B wheeledSA = VehicleWheeledSimulation_SA_B.Cast(m_Root.FindComponent(VehicleWheeledSimulation_SA_B));
		if (wheeledSA)
		{
			for (int wheel = 0, wheelCount = wheeledSA.WheelCount(); wheel < wheelCount; wheel++)
			{
				AddWheelRectangle(wheeledSA.WheelGetPosition(wheel), wheeledSA.WheelGetRadius(wheel));
			}

			return;
		}

		VehicleTrackedSimulation tracked = VehicleTrackedSimulation.Cast(m_Root.FindComponent(VehicleTrackedSimulation));
		if (!tracked || tracked.TrackCount() <= 0)
			return;

		// A track runs from its front wheel to its back one; WheelCount counts every track's wheels
		int trackCount = tracked.TrackCount();
		int wheelsPerTrack = tracked.WheelCount() / trackCount;
		for (int track = 0; track < trackCount; track++)
		{
			if (wheelsPerTrack <= 0)
				break;

			float sumX = 0;
			float minZ = float.MAX;
			float maxZ = -float.MAX;
			for (int wheel = 0; wheel < wheelsPerTrack; wheel++)
			{
				vector position = tracked.WheelGetPosition(track, wheel);
				float radius = tracked.WheelGetRadius(track, wheel);
				sumX += position[0];
				minZ = Math.Min(minZ, position[2] - radius);
				maxZ = Math.Max(maxZ, position[2] + radius);
			}

			float centerX = sumX / wheelsPerTrack;
			m_aWheelRectangles.Insert(centerX - TRACK_WIDTH * 0.5);
			m_aWheelRectangles.Insert(centerX + TRACK_WIDTH * 0.5);
			m_aWheelRectangles.Insert(minZ);
			m_aWheelRectangles.Insert(maxZ);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] position wheel center in the root's space
	//! \param[in] radius m
	protected void AddWheelRectangle(vector position, float radius)
	{
		float halfWidth = Math.Clamp(radius * WHEEL_WIDTH_SHARE, MIN_WHEEL_WIDTH, MAX_WHEEL_WIDTH) * 0.5;
		m_aWheelRectangles.Insert(position[0] - halfWidth);
		m_aWheelRectangles.Insert(position[0] + halfWidth);
		m_aWheelRectangles.Insert(position[2] - radius);
		m_aWheelRectangles.Insert(position[2] + radius);
	}

	//------------------------------------------------------------------------------------------------
	//! Notes the vehicle's seats that can turn out, and the hatch each one opens.
	//! \param[in] vehicle
	protected void AddTurnOutSlots(notnull IEntity vehicle)
	{
		array<BaseCompartmentSlot> slots = {};
		CSI_VehicleOutline.GetSeats(vehicle, slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (!slot.CanTurnOut())
				continue;

			BaseCompartmentManagerComponent slotManager = slot.GetManager();
			if (!slotManager)
				continue;

			BaseCompartmentDoor hatch = slotManager.GetDoor(slot.GetTurnOutDoorIndex());
			if (!hatch)
				continue;

			BaseCompartmentManagerComponent hatchManager = GetDoorManager(slotManager, hatch);
			if (!hatchManager)
				continue;

			m_aTurnOutSlots.Insert(slot);
			m_aTurnOutManagers.Insert(hatchManager);
			m_aTurnOutDoors.Insert(hatch.GetAnimDoorIndex());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Grows the trace grid's bounds by the part's mesh bounds, in the root's space.
	//! \param[in] part
	//! \param[in] first start the bounds from this part
	//! \return false if the part has no bounds
	protected bool AddBounds(notnull IEntity part, bool first)
	{
		vector mins, maxs;
		part.GetBounds(mins, maxs);
		if (mins == maxs)
			return false;

		vector partTransform[4];
		if (part != m_Root)
			part.GetWorldTransform(partTransform);

		for (int corner = 0; corner < 8; corner++)
		{
			vector point = GetBoxCorner(mins, maxs, corner);
			if (part != m_Root)
				point = m_Root.CoordToLocal(point.Multiply4(partTransform));

			if (first && corner == 0)
			{
				m_vMins = point;
				m_vMaxs = point;
				continue;
			}

			for (int axis = 0; axis < 3; axis++)
			{
				m_vMins[axis] = Math.Min(m_vMins[axis], point[axis]);
				m_vMaxs[axis] = Math.Max(m_vMaxs[axis], point[axis]);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return projectile layers, plus every part layer for a gun (weapons collide on their own layers)
	protected int GetTraceLayers()
	{
		int layers = EPhysicsLayerPresets.Projectile;
		if (!m_Root.FindComponent(WeaponComponent))
			return layers;

		return layers | GetPartLayers();
	}

	//------------------------------------------------------------------------------------------------
	//! \return every collider layer of the root and its parts
	protected int GetPartLayers()
	{
		int layers;
		foreach (IEntity part : m_aParts)
		{
			Physics physics = part.GetPhysics();
			if (!physics)
				continue;

			for (int geom = 0, geomCount = physics.GetNumGeoms(); geom < geomCount; geom++)
			{
				layers |= physics.GetGeomInteractionLayer(geom);
			}
		}

		return layers;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity a turret
	//! \return its main gun: the weapon with a model in its lowest weapon slot, null without one
	protected static IEntity GetMainGun(notnull IEntity entity)
	{
		IEntity gun;
		int gunSlotIndex;
		array<Managed> slots = {};
		entity.FindComponents(WeaponSlotComponent, slots);
		foreach (Managed slotComponent : slots)
		{
			WeaponSlotComponent slot = WeaponSlotComponent.Cast(slotComponent);
			if (!slot || !slot.GetWeaponEntity() || !slot.GetWeaponEntity().GetVObject())
				continue;

			if (gun && slot.GetWeaponSlotIndex() >= gunSlotIndex)
				continue;

			gun = slot.GetWeaponEntity();
			gunSlotIndex = slot.GetWeaponSlotIndex();
		}

		return gun;
	}

	//------------------------------------------------------------------------------------------------
	//! Traces straight down through the main gun near its muzzle, as the grid would.
	//! \param[in] gun null if none
	//! \return true if the trace hits the gun, or there's no gun
	protected bool IsGunTraced(IEntity gun)
	{
		if (!gun)
			return true;

		vector mins, maxs;
		gun.GetBounds(mins, maxs);
		if (mins == maxs)
			return true;

		vector sample = (mins + maxs) * 0.5;
		sample[2] = mins[2] + (maxs[2] - mins[2]) * GUN_SAMPLE_SHARE;

		vector gunTransform[4];
		gun.GetWorldTransform(gunTransform);
		vector point = sample.Multiply4(gunTransform);

		m_TraceParam.Start = point + vector.Up * GUN_SAMPLE_HEIGHT;
		m_TraceParam.End = point - vector.Up * GUN_SAMPLE_HEIGHT;
		return m_Root.GetWorld().TraceMove(m_TraceParam, null) < 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Notes the collider layers of gun pieces below the parent, so AddColliderHull skips their merged copies.
	//! \param[in] parent
	protected void AddPieceGunLayers(notnull IEntity parent)
	{
		IEntity child = parent.GetChildren();
		while (child)
		{
			if (child.FindComponent(WeaponComponent) && CSI_VehicleOutline.IsOwnPiece(child))
				AddSubtreeLayers(child);
			else
				AddPieceGunLayers(child);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity whose colliders' layers, and its descendants', go into m_aPieceGunLayers
	protected void AddSubtreeLayers(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (physics)
		{
			for (int geom = 0, geomCount = physics.GetNumGeoms(); geom < geomCount; geom++)
			{
				int layer = physics.GetGeomInteractionLayer(geom);
				if (!m_aPieceGunLayers.Contains(layer))
					m_aPieceGunLayers.Insert(layer);
			}
		}

		IEntity child = entity.GetChildren();
		while (child)
		{
			AddSubtreeLayers(child);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] minCellSize
	//! \return key of everything the outline depends on, empty if a part has neither mesh nor prefab
	protected string GetShapeKey(float minCellSize)
	{
		array<string> entries = {};
		foreach (IEntity part : m_aParts)
		{
			string shape = string.Empty;
			VObject visualObject = part.GetVObject();
			if (visualObject)
				shape = visualObject.GetResourceName();

			if (shape.IsEmpty())
				shape = CSI_VehicleOutline.GetPrefabName(part);

			if (shape.IsEmpty())
				return string.Empty;

			vector position = vector.Zero;
			vector forward = vector.Forward;
			if (part != m_Root)
			{
				vector transform[4];
				part.GetWorldTransform(transform);
				position = m_Root.CoordToLocal(transform[3]);
				forward = m_Root.VectorToLocal(transform[2]);
			}

			int layer = -1;
			int geomCount = 0;
			Physics physics = part.GetPhysics();
			if (physics)
			{
				layer = physics.GetInteractionLayer();
				geomCount = physics.GetNumGeoms();
			}

			entries.Insert(string.Format("%1 %2 %3 %4 %5", shape, FormatCentimeters(position), FormatCentimeters(forward), layer, geomCount));
		}

		// Slot order doesn't change the shape
		entries.Sort();

		string key = string.Format("%1 %2", FormatCentimeters(Vector(minCellSize, 0, 0)), m_Turret != null);
		foreach (string entry : entries)
		{
			key += "|" + entry;
		}

		for (int wheel = 0, valueCount = m_aWheelRectangles.Count() - 3; wheel < valueCount; wheel += 4)
		{
			key += "|" + FormatCentimeters(Vector(m_aWheelRectangles[wheel], m_aWheelRectangles[wheel + 1], 0)) + "," + FormatCentimeters(Vector(m_aWheelRectangles[wheel + 2], m_aWheelRectangles[wheel + 3], 0));
		}

		foreach (vector rotorValue : m_aRotors)
		{
			key += "|" + FormatCentimeters(rotorValue);
		}

		return key;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 BUILDING
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Call every frame while building; an open door restarts the traces.
	//! \param[in] timeSlice seconds since the last call
	//! \return true once Step may trace, also once built or if the entity is gone
	bool UpdateDoors(float timeSlice)
	{
		if (m_Outline || !m_Root)
			return true;

		if (!AreDoorsClosed())
		{
			m_bDoorOpen = true;
			RestartTraces();
			return false;
		}

		m_bDoorOpen = false;
		m_fDoorsClosedTime += timeSlice;
		return m_fDoorsClosedTime >= DOORS_CLOSED_TIME;
	}

	//------------------------------------------------------------------------------------------------
	bool HasOpenDoor()
	{
		return m_bDoorOpen;
	}

	//------------------------------------------------------------------------------------------------
	//! Runs the next traces once UpdateDoors allows it, and builds the outline after the last one.
	//! \param[in] maxTraces traces this call may run
	//! \return true once built (GetOutline), also if the entity is gone
	bool Step(int maxTraces)
	{
		if (m_Outline)
			return true;

		if (!m_Root)
		{
			m_Outline = new CSI_VehicleOutline();
			return true;
		}

		if (m_fDoorsClosedTime < DOORS_CLOSED_TIME)
			return false;

		UpdateFrame();
		int traces;

		// Trace the grid, then finish it a stage a frame, as each goes over every cell
		if (!m_bGridDone)
		{
			FloodGrid(maxTraces);
			return false;
		}

		if (!m_bGapsClosed)
		{
			CloseGaps();
			m_bGapsClosed = true;
			return false;
		}

		if (!m_bHolesFilled)
		{
			FillHoles();
			m_bHolesFilled = true;
			return false;
		}

		if (!m_bLinked)
		{
			LinkCells();
			m_bLinked = true;
		}

		// Each edge point, more closely
		while (m_iNextEdge < m_aEdgeKeys.Count() && traces < maxTraces)
		{
			RefineEdge(m_aEdgeKeys[m_iNextEdge]);
			m_iNextEdge++;
			traces += REFINE_STEPS;
		}

		if (m_iNextEdge < m_aEdgeKeys.Count())
			return false;

		m_Outline = new CSI_VehicleOutline();
		AddContours();

		// With only copies of another piece's gun as colliders, the piece stays empty
		bool onlyPieceGunColliders;
		if (m_Outline.IsEmpty())
			onlyPieceGunColliders = AddColliderHull();

		if (m_Outline.IsEmpty() && m_iColumns > 0 && !onlyPieceGunColliders)
			AddBoundsRectangle();

		// A wreck's outline is only its own
		m_bCacheable = !IsWrecked();
		if (m_bCacheable)
			CSI_VehicleOutline.Share(m_sShapeKey, m_Outline);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return null until Step returns true
	CSI_VehicleOutline GetOutline()
	{
		return m_Outline;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the outline may be kept for the prefab (not a wreck)
	bool IsCacheable()
	{
		return m_bCacheable;
	}

	//------------------------------------------------------------------------------------------------
	//! \return false if a real door is open (fake ones always report open), except turned-out crew's hatches
	protected bool AreDoorsClosed()
	{
		foreach (BaseCompartmentManagerComponent compartmentManager : m_aCompartmentManagers)
		{
			if (!compartmentManager)
				continue;

			for (int door = 0; compartmentManager.GetDoor(door); door++)
			{
				if (!compartmentManager.IsDoorFake(door) && compartmentManager.IsDoorOpen(door) && !IsTurnedOutHatch(compartmentManager, door))
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartmentManager
	//! \param[in] door index in the manager
	//! \return true if it's the hatch of a seat whose occupant is turned out
	protected bool IsTurnedOutHatch(notnull BaseCompartmentManagerComponent compartmentManager, int door)
	{
		if (m_aTurnOutSlots.IsEmpty())
			return false;

		BaseCompartmentDoor compartmentDoor = compartmentManager.GetDoor(door);
		if (!compartmentDoor)
			return false;

		BaseCompartmentManagerComponent hatchManager = GetDoorManager(compartmentManager, compartmentDoor);
		int animDoor = compartmentDoor.GetAnimDoorIndex();
		foreach (int index, BaseCompartmentSlot slot : m_aTurnOutSlots)
		{
			if (slot && m_aTurnOutManagers[index] == hatchManager && m_aTurnOutDoors[index] == animDoor && slot.IsOccupantTurnedOut())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void RestartTraces()
	{
		m_fDoorsClosedTime = 0;
		if (!m_bSeeded)
			return;

		m_bSeeded = false;
		m_bGridDone = false;
		m_bGapsClosed = false;
		m_bHolesFilled = false;
		m_bLinked = false;
		m_iNextEdge = 0;
		m_aFloodStack.Clear();
		ClearCells(m_aHits);
		ClearCells(m_aStamped);
		ClearCells(m_aVisited);
		m_mFirstNeighbors.Clear();
		m_mSecondNeighbors.Clear();
		m_aEdgeKeys.Clear();
		m_mEdgeFractions.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Traces from the empty ring inward, spreading from misses; cells enclosed by hits count as hits untraced.
	//! \param[in] maxTraces traces this call may run
	protected void FloodGrid(int maxTraces)
	{
		if (!m_bSeeded)
			SeedFlood();

		int traces;
		while (!m_aFloodStack.IsEmpty() && traces < maxTraces)
		{
			int last = m_aFloodStack.Count() - 1;
			int cell = m_aFloodStack[last];
			m_aFloodStack.Remove(last);

			int column = cell % m_iColumns;
			int row = cell / m_iColumns;
			float x = GetSampleX(column);
			float z = GetSampleZ(row);
			if (IsOnWheel(x, z))
			{
				m_aHits[cell] = true;
				m_aStamped[cell] = true;
				continue;
			}

			traces++;
			if (TraceColumn(x, z))
			{
				m_aHits[cell] = true;
				continue;
			}

			// Diagonal neighbors too, as marching squares joins misses diagonally
			for (int neighborRow = row - 1; neighborRow <= row + 1; neighborRow++)
			{
				for (int neighborColumn = column - 1; neighborColumn <= column + 1; neighborColumn++)
				{
					PushFloodCell(neighborColumn, neighborRow);
				}
			}
		}

		if (!m_aFloodStack.IsEmpty())
			return;

		for (int innerRow = 1; innerRow < m_iRows - 1; innerRow++)
		{
			for (int innerColumn = 1; innerColumn < m_iColumns - 1; innerColumn++)
			{
				int innerCell = innerRow * m_iColumns + innerColumn;
				if (!m_aVisited[innerCell])
					m_aHits[innerCell] = true;
			}
		}

		m_bGridDone = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Starts the flood from every cell next to the empty ring.
	protected void SeedFlood()
	{
		m_bSeeded = true;

		for (int column = 1; column < m_iColumns - 1; column++)
		{
			PushFloodCell(column, 1);
			PushFloodCell(column, m_iRows - 2);
		}

		for (int row = 1; row < m_iRows - 1; row++)
		{
			PushFloodCell(1, row);
			PushFloodCell(m_iColumns - 2, row);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Queues an inner cell for tracing if the flood hasn't reached it yet.
	//! \param[in] column
	//! \param[in] row
	protected void PushFloodCell(int column, int row)
	{
		if (column < 1 || row < 1 || column > m_iColumns - 2 || row > m_iRows - 2)
			return;

		int cell = row * m_iColumns + column;
		if (m_aVisited[cell])
			return;

		m_aVisited[cell] = true;
		m_aFloodStack.Insert(cell);
	}

	//------------------------------------------------------------------------------------------------
	//! Places the outline's frame in the world for this step's traces.
	protected void UpdateFrame()
	{
		m_World = m_Root.GetWorld();

		vector frame[4];
		CSI_VehicleOutline.GetWorldFrame(m_Root, m_Turret, m_vFramePivot, frame);
		m_vFrameX = frame[0];
		m_vFrameY = frame[1];
		m_vFrameZ = frame[2];
		m_vFrameOrigin = frame[3];

		if (!m_Turret)
			return;

		float frameYaw = CSI_VehicleOutline.GetFrameYaw(m_Root, m_Turret);
		m_fFrameCos = Math.Cos(frameYaw);
		m_fFrameSin = Math.Sin(frameYaw);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] x m in the frame
	//! \param[in] z m in the frame
	//! \return the point in the root's space, at y 0
	protected vector FrameToRoot(float x, float z)
	{
		return Vector(m_vFramePivot[0] + x * m_fFrameCos + z * m_fFrameSin, 0, m_vFramePivot[2] - x * m_fFrameSin + z * m_fFrameCos);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] x m in the frame
	//! \param[in] z m in the frame
	//! \return true if a trace straight down there hits the root or its parts, skipping rotor blades
	protected bool TraceColumn(float x, float z)
	{
		float top = m_vMaxs[1] + TRACE_MARGIN;
		float bottom = m_vMins[1] - TRACE_MARGIN;

		// Leave out a helicopter's skids
		if (m_bSkids)
			bottom = m_vMins[1] + SKID_HEIGHT;

		// Under a main rotor only below its blades; past a tail rotor's disc above and below it
		float bandTop, bandBottom;
		bool traceAbove;
		if (GetRotorBand(x, z, bandTop, bandBottom, traceAbove))
		{
			if (traceAbove && bandTop < top && TraceSegment(x, z, top, bandTop))
				return true;

			return bandBottom > bottom && TraceSegment(x, z, bandBottom, bottom);
		}

		return TraceSegment(x, z, top, bottom);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] x m in the frame
	//! \param[in] z m in the frame
	//! \param[in] top height in the frame
	//! \param[in] bottom height in the frame
	//! \return true if a trace down between the heights hits the root or its parts
	protected bool TraceSegment(float x, float z, float top, float bottom)
	{
		vector column = m_vFrameOrigin + m_vFrameX * x + m_vFrameZ * z;
		m_TraceParam.Start = column + m_vFrameY * top;
		m_TraceParam.End = column + m_vFrameY * bottom;

		return m_World.TraceMove(m_TraceParam, null) < 1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] x m in the frame
	//! \param[in] z m in the frame
	//! \param[out] bandTop
	//! \param[out] bandBottom
	//! \param[out] traceAbove false under a main rotor
	//! \return true if the column passes through a rotor's blades, with the heights to skip
	protected bool GetRotorBand(float x, float z, out float bandTop, out float bandBottom, out bool traceAbove)
	{
		if (m_aRotors.IsEmpty())
			return false;

		vector point = FrameToRoot(x, z);
		bool inBand;
		for (int i = 0, count = m_aRotors.Count(); i < count; i += 3)
		{
			vector position = m_aRotors[i];
			vector axis = m_aRotors[i + 1];
			float radius = m_aRotors[i + 2][0];
			bool isMain = m_aRotors[i + 2][1] != 0;
			float offsetX = point[0] - position[0];
			float offsetZ = point[2] - position[2];

			float top, bottom;
			bool above;
			if (isMain)
			{
				// Main rotor: the blades' plane, tilted with the mast
				float reach = radius + ROTOR_HALF_THICKNESS;
				if (offsetX * offsetX + offsetZ * offsetZ > reach * reach)
					continue;

				float planeY = position[1] - (offsetX * axis[0] + offsetZ * axis[2]) / axis[1];
				top = planeY + ROTOR_HALF_THICKNESS;
				bottom = planeY - ROTOR_HALF_THICKNESS;
				above = false;
			}
			else
			{
				// Tail rotor: a disc standing up, seen edge on from above
				float axisLength = Math.Sqrt(axis[0] * axis[0] + axis[2] * axis[2]);
				float along = (offsetX * axis[0] + offsetZ * axis[2]) / axisLength;
				if (Math.AbsFloat(along) > ROTOR_HALF_THICKNESS)
					continue;

				float acrossSq = offsetX * offsetX + offsetZ * offsetZ - along * along;
				if (acrossSq > radius * radius)
					continue;

				float height = Math.Sqrt(radius * radius - acrossSq);
				top = position[1] + height + ROTOR_HALF_THICKNESS;
				bottom = position[1] - height - ROTOR_HALF_THICKNESS;
				above = true;
			}

			if (!inBand)
			{
				bandTop = top;
				bandBottom = bottom;
				traceAbove = above;
				inBand = true;
				continue;
			}

			bandTop = Math.Max(bandTop, top);
			bandBottom = Math.Min(bandBottom, bottom);
			traceAbove = traceAbove && above;
		}

		return inBand;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] x m in the frame
	//! \param[in] z m in the frame
	//! \return true if the point is on a wheel or track seen from above
	protected bool IsOnWheel(float x, float z)
	{
		if (m_aWheelRectangles.IsEmpty())
			return false;

		vector point = FrameToRoot(x, z);
		for (int i = 0, count = m_aWheelRectangles.Count(); i < count; i += 4)
		{
			if (point[0] >= m_aWheelRectangles[i] && point[0] <= m_aWheelRectangles[i + 1] && point[2] >= m_aWheelRectangles[i + 2] && point[2] <= m_aWheelRectangles[i + 3])
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills gaps of up to two cells between hits (grow, then shrink), so racks and railings come out as one shape.
	protected void CloseGaps()
	{
		array<bool> grown = {};
		SpreadHits(m_aHits, grown, true);
		SpreadHits(grown, m_aHits, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills the misses CloseGaps shut in, so no shape keeps a hole.
	protected void FillHoles()
	{
		array<bool> reached = {};
		reached.Resize(m_aHits.Count());
		array<int> spreading = {};

		// The ring is all misses: spread from it through misses, diagonally too
		for (int column = 0; column < m_iColumns; column++)
		{
			PushOpenCell(column, 0, reached, spreading);
			PushOpenCell(column, m_iRows - 1, reached, spreading);
		}

		for (int row = 0; row < m_iRows; row++)
		{
			PushOpenCell(0, row, reached, spreading);
			PushOpenCell(m_iColumns - 1, row, reached, spreading);
		}

		while (!spreading.IsEmpty())
		{
			int last = spreading.Count() - 1;
			int cell = spreading[last];
			spreading.Remove(last);

			int cellColumn = cell % m_iColumns;
			int cellRow = cell / m_iColumns;
			for (int neighborRow = cellRow - 1; neighborRow <= cellRow + 1; neighborRow++)
			{
				for (int neighborColumn = cellColumn - 1; neighborColumn <= cellColumn + 1; neighborColumn++)
				{
					PushOpenCell(neighborColumn, neighborRow, reached, spreading);
				}
			}
		}

		foreach (int index, bool isReached : reached)
		{
			if (!isReached)
				m_aHits[index] = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Queues a miss for FillHoles if it isn't reached yet.
	//! \param[in] column
	//! \param[in] row
	//! \param[in] reached by cell
	//! \param[in] spreading cells reached and not spread from yet
	protected void PushOpenCell(int column, int row, notnull array<bool> reached, notnull array<int> spreading)
	{
		if (column < 0 || row < 0 || column >= m_iColumns || row >= m_iRows)
			return;

		int cell = row * m_iColumns + column;
		if (reached[cell] || m_aHits[cell])
			return;

		reached[cell] = true;
		spreading.Insert(cell);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] from
	//! \param[out] to from grown or shrunk by a cell; outside the grid counts as misses
	//! \param[in] grow
	protected void SpreadHits(notnull array<bool> from, notnull array<bool> to, bool grow)
	{
		// Along rows, then along columns
		array<bool> rows = {};
		rows.Resize(from.Count());
		to.Resize(from.Count());

		for (int row = 0; row < m_iRows; row++)
		{
			for (int column = 0; column < m_iColumns; column++)
			{
				int cell = row * m_iColumns + column;
				bool left = column > 0 && from[cell - 1];
				bool right = column < m_iColumns - 1 && from[cell + 1];
				if (grow)
					rows[cell] = from[cell] || left || right;
				else
					rows[cell] = from[cell] && left && right;
			}
		}

		for (int row = 0; row < m_iRows; row++)
		{
			for (int column = 0; column < m_iColumns; column++)
			{
				int cell = row * m_iColumns + column;
				bool below = row > 0 && rows[cell - m_iColumns];
				bool above = row < m_iRows - 1 && rows[cell + m_iColumns];
				if (grow)
					to[cell] = rows[cell] || below || above;
				else
					to[cell] = rows[cell] && below && above;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Links the edge points around the hit cells (marching squares).
	protected void LinkCells()
	{
		// Cells run one past the grid on every side, so every line closes
		for (int row = -1; row < m_iRows; row++)
		{
			for (int column = -1; column < m_iColumns; column++)
			{
				// One bit per corner: bottom left, bottom right, top right, top left
				int cellCase = IsHit(column, row) + IsHit(column + 1, row) * 2 + IsHit(column + 1, row + 1) * 4 + IsHit(column, row + 1) * 8;
				if (cellCase == 0 || cellCase == 15)
					continue;

				int bottom = GetEdgeKey(column, row, false);
				int right = GetEdgeKey(column + 1, row, true);
				int top = GetEdgeKey(column, row + 1, false);
				int left = GetEdgeKey(column, row, true);

				switch (cellCase)
				{
					case 1:		LinkEdges(left, bottom); break;
					case 2:		LinkEdges(bottom, right); break;
					case 3:		LinkEdges(left, right); break;
					case 4:		LinkEdges(right, top); break;
					case 5:		LinkEdges(left, bottom); LinkEdges(right, top); break;
					case 6:		LinkEdges(bottom, top); break;
					case 7:		LinkEdges(left, top); break;
					case 8:		LinkEdges(left, top); break;
					case 9:		LinkEdges(bottom, top); break;
					case 10:	LinkEdges(bottom, right); LinkEdges(left, top); break;
					case 11:	LinkEdges(right, top); break;
					case 12:	LinkEdges(left, right); break;
					case 13:	LinkEdges(bottom, right); break;
					case 14:	LinkEdges(left, bottom); break;
				}
			}
		}

		foreach (int key, int neighbor : m_mFirstNeighbors)
		{
			m_aEdgeKeys.Insert(key);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Finds where the outline crosses the edge point's cell edge; wheel and track edges stay in the middle.
	//! \param[in] key GetEdgeKey
	protected void RefineEdge(int key)
	{
		int column, row;
		bool alongZ;
		DecodeEdgeKey(key, column, row, alongZ);

		int endColumn = column;
		int endRow = row;
		if (alongZ)
			endRow++;
		else
			endColumn++;

		bool startHit = IsHit(column, row) == 1;
		if ((startHit && IsStamped(column, row)) || (!startHit && IsStamped(endColumn, endRow)))
			return;

		float startX = GetSampleX(column);
		float startZ = GetSampleZ(row);
		float endX = GetSampleX(endColumn);
		float endZ = GetSampleZ(endRow);

		float low = 0;
		float high = 1;
		for (int step = 0; step < REFINE_STEPS; step++)
		{
			float middle = (low + high) * 0.5;
			if (TraceColumn(Math.Lerp(startX, endX, middle), Math.Lerp(startZ, endZ, middle)) == startHit)
				low = middle;
			else
				high = middle;
		}

		m_mEdgeFractions.Set(key, (low + high) * 0.5);
	}

	//------------------------------------------------------------------------------------------------
	//! Walks the linked edge points into closed lines, leaves out small pieces and simplifies the rest.
	protected void AddContours()
	{
		float minArea = MIN_CONTOUR_AREA * m_fCellSize * m_fCellSize;
		int pointCount = m_mFirstNeighbors.Count();
		set<int> visited = new set<int>();

		foreach (int start : m_aEdgeKeys)
		{
			if (visited.Contains(start))
				continue;

			array<float> contour = {};
			int previous = -1;
			int current = start;
			for (int step = 0; step < pointCount; step++)
			{
				visited.Insert(current);
				AddEdgePoint(contour, current);

				int next;
				m_mFirstNeighbors.Find(current, next);
				if (next == previous)
					m_mSecondNeighbors.Find(current, next);

				previous = current;
				current = next;
				if (current == start || visited.Contains(current))
					break;
			}

			if (Math.AbsFloat(CSI_VehicleOutline.GetArea(contour)) < minArea)
				continue;

			m_Outline.AddContour(Simplify(contour, SIMPLIFY_TOLERANCE));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Fallback: the convex hull of the colliders' boxes, leaving out merged copies of gun pieces.
	//! \return true if every collider is such a copy, so the piece stays empty
	protected bool AddColliderHull()
	{
		array<float> points = {};

		float frameYaw = CSI_VehicleOutline.GetFrameYaw(m_Root, m_Turret);
		float cosYaw = Math.Cos(frameYaw);
		float sinYaw = Math.Sin(frameYaw);

		int geomsSeen;
		int geomsUsed;
		foreach (IEntity part : m_aParts)
		{
			if (!part)
				continue;

			Physics physics = part.GetPhysics();
			if (!physics)
				continue;

			vector partTransform[4];
			if (part != m_Root)
				part.GetWorldTransform(partTransform);

			for (int geom = 0, geomCount = physics.GetNumGeoms(); geom < geomCount; geom++)
			{
				geomsSeen++;
				if (m_aPieceGunLayers.Contains(physics.GetGeomInteractionLayer(geom)))
					continue;

				geomsUsed++;

				vector geomTransform[4];
				physics.GetGeomTransform(geom, geomTransform);

				vector mins, maxs;
				physics.GetGeomDirectBounds(geom, mins, maxs);
				if (mins == maxs)
					continue;

				for (int corner = 0; corner < 8; corner++)
				{
					vector point = GetBoxCorner(mins, maxs, corner).Multiply4(geomTransform);
					if (part != m_Root)
						point = m_Root.CoordToLocal(point.Multiply4(partTransform));

					// Into the frame; kept near the meshes if there are any, in case a collider's bounds are in another space
					point = point - m_vFramePivot;
					float x = point[0] * cosYaw - point[2] * sinYaw;
					float z = point[0] * sinYaw + point[2] * cosYaw;
					if (m_iColumns > 0)
					{
						x = Math.Clamp(x, m_vMins[0] - COLLIDER_BOUNDS_MARGIN, m_vMaxs[0] + COLLIDER_BOUNDS_MARGIN);
						z = Math.Clamp(z, m_vMins[2] - COLLIDER_BOUNDS_MARGIN, m_vMaxs[2] + COLLIDER_BOUNDS_MARGIN);
					}

					points.Insert(x);
					points.Insert(z);
				}
			}
		}

		array<float> hull = GetConvexHull(points);
		if (hull.Count() >= 6)
			m_Outline.AddContour(hull);

		return geomsSeen > 0 && geomsUsed == 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Last fallback: the grid's bounds.
	protected void AddBoundsRectangle()
	{
		array<float> rectangle = {
			m_vMins[0], m_vMins[2],
			m_vMaxs[0], m_vMins[2],
			m_vMaxs[0], m_vMaxs[2],
			m_vMins[0], m_vMaxs[2]};

		m_Outline.AddContour(rectangle);
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the root or its vehicle is destroyed (a wreck's model may be in place)
	protected bool IsWrecked()
	{
		if (!m_Root)
			return true;

		IEntity vehicle = GetVehicle(m_Root);
		return IsDestroyed(m_Root) || (vehicle != m_Root && IsDestroyed(vehicle));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 HELPERS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected float GetSampleX(int column)
	{
		return m_vMins[0] + (column - 0.5) * m_fCellSize;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetSampleZ(int row)
	{
		return m_vMins[2] + (row - 0.5) * m_fCellSize;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] column
	//! \param[in] row
	//! \return 1 if the trace there hit, 0 if it missed or is outside the grid
	protected int IsHit(int column, int row)
	{
		if (column < 0 || row < 0 || column >= m_iColumns || row >= m_iRows)
			return 0;

		if (m_aHits[row * m_iColumns + column])
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] column
	//! \param[in] row
	//! \return true if a wheel or track filled the cell rather than a trace
	protected bool IsStamped(int column, int row)
	{
		if (column < 0 || row < 0 || column >= m_iColumns || row >= m_iRows)
			return false;

		return m_aStamped[row * m_iColumns + column];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] column of the edge's first trace (-1 to m_iColumns)
	//! \param[in] row of the edge's first trace (-1 to m_iRows)
	//! \param[in] alongZ the edge runs to the next row, else to the next column
	//! \return a key for the edge's point
	protected int GetEdgeKey(int column, int row, bool alongZ)
	{
		int key = ((row + 1) * (m_iColumns + 2) + column + 1) * 2;
		if (alongZ)
			key++;

		return key;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] key GetEdgeKey
	//! \param[out] column
	//! \param[out] row
	//! \param[out] alongZ
	protected void DecodeEdgeKey(int key, out int column, out int row, out bool alongZ)
	{
		alongZ = key % 2 == 1;
		int cell = key / 2;
		int stride = m_iColumns + 2;
		column = cell % stride - 1;
		row = cell / stride - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] contour x, z pairs the edge point is added to
	//! \param[in] key GetEdgeKey
	protected void AddEdgePoint(notnull array<float> contour, int key)
	{
		int column, row;
		bool alongZ;
		DecodeEdgeKey(key, column, row, alongZ);

		float fraction = 0.5;
		m_mEdgeFractions.Find(key, fraction);

		float x = GetSampleX(column);
		float z = GetSampleZ(row);
		if (alongZ)
			z += m_fCellSize * fraction;
		else
			x += m_fCellSize * fraction;

		contour.Insert(x);
		contour.Insert(z);
	}

	//------------------------------------------------------------------------------------------------
	protected void LinkEdges(int keyA, int keyB)
	{
		LinkEdge(keyA, keyB);
		LinkEdge(keyB, keyA);
	}

	//------------------------------------------------------------------------------------------------
	protected void LinkEdge(int key, int neighbor)
	{
		if (!m_mFirstNeighbors.Contains(key))
			m_mFirstNeighbors.Set(key, neighbor);
		else
			m_mSecondNeighbors.Set(key, neighbor);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartmentManager the manager listing the door
	//! \param[in] door
	//! \return the manager whose door it is: for a door reference, the manager it points to
	protected static BaseCompartmentManagerComponent GetDoorManager(notnull BaseCompartmentManagerComponent compartmentManager, notnull BaseCompartmentDoor door)
	{
		CompartmentDoorReference doorReference = CompartmentDoorReference.Cast(door);
		if (doorReference)
			return doorReference.GetReferencedDoorCompartmentManager();

		return compartmentManager;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return its topmost ancestor with seats, or the entity itself without one
	protected static IEntity GetVehicle(notnull IEntity entity)
	{
		IEntity vehicle = entity;
		IEntity ancestor = entity.GetParent();
		while (ancestor)
		{
			if (ancestor.FindComponent(BaseCompartmentManagerComponent))
				vehicle = ancestor;

			ancestor = ancestor.GetParent();
		}

		return vehicle;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsDestroyed(notnull IEntity entity)
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return false;

		return damageManager.GetState() == EDamageState.DESTROYED;
	}

	//------------------------------------------------------------------------------------------------
	protected static void ClearCells(notnull array<bool> cells)
	{
		int count = cells.Count();
		cells.Clear();
		cells.Resize(count);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	//! \return the value's three components in whole centimeters
	protected static string FormatCentimeters(vector value)
	{
		int x = Math.Round(value[0] * 100);
		int y = Math.Round(value[1] * 100);
		int z = Math.Round(value[2] * 100);
		return string.Format("%1,%2,%3", x, y, z);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] mins
	//! \param[in] maxs
	//! \param[in] corner 0 to 7, one bit per axis
	//! \return the box's corner
	protected static vector GetBoxCorner(vector mins, vector maxs, int corner)
	{
		vector point = mins;
		if (corner & 1)
			point[0] = maxs[0];

		if (corner & 2)
			point[1] = maxs[1];

		if (corner & 4)
			point[2] = maxs[2];

		return point;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops points that stray less than the tolerance from the line between their neighbors (Douglas-Peucker).
	//! \param[in] contour closed x, z pairs
	//! \param[in] tolerance m
	//! \return the simplified contour
	protected static array<float> Simplify(notnull array<float> contour, float tolerance)
	{
		int count = contour.Count() / 2;
		if (count < 4)
			return contour;

		// Split the closed line at the point farthest from the first
		int farthest;
		float farthestDistanceSq;
		for (int i = 1; i < count; i++)
		{
			float dx = contour[i * 2] - contour[0];
			float dz = contour[i * 2 + 1] - contour[1];
			float distanceSq = dx * dx + dz * dz;
			if (distanceSq > farthestDistanceSq)
			{
				farthestDistanceSq = distanceSq;
				farthest = i;
			}
		}

		array<bool> keep = {};
		keep.Resize(count);
		keep[0] = true;
		keep[farthest] = true;

		// Index count is the first point again, closing the line
		array<int> ranges = {0, farthest, farthest, count};
		while (!ranges.IsEmpty())
		{
			int last = ranges.Count() - 1;
			int rangeEnd = ranges[last];
			int rangeStart = ranges[last - 1];
			ranges.Remove(last);
			ranges.Remove(last - 1);

			float startX = contour[rangeStart * 2];
			float startZ = contour[rangeStart * 2 + 1];
			float endX = contour[(rangeEnd % count) * 2];
			float endZ = contour[(rangeEnd % count) * 2 + 1];

			int worst = -1;
			float worstDistance = tolerance;
			for (int point = rangeStart + 1; point < rangeEnd; point++)
			{
				float distance = GetDistanceToSegment(contour[point * 2], contour[point * 2 + 1], startX, startZ, endX, endZ);
				if (distance > worstDistance)
				{
					worstDistance = distance;
					worst = point;
				}
			}

			if (worst < 0)
				continue;

			keep[worst] = true;
			ranges.Insert(rangeStart);
			ranges.Insert(worst);
			ranges.Insert(worst);
			ranges.Insert(rangeEnd);
		}

		array<float> simplified = {};
		foreach (int i, bool kept : keep)
		{
			if (!kept)
				continue;

			simplified.Insert(contour[i * 2]);
			simplified.Insert(contour[i * 2 + 1]);
		}

		return simplified;
	}

	//------------------------------------------------------------------------------------------------
	protected static float GetDistanceToSegment(float x, float z, float startX, float startZ, float endX, float endZ)
	{
		float segmentX = endX - startX;
		float segmentZ = endZ - startZ;
		float lengthSq = segmentX * segmentX + segmentZ * segmentZ;

		float t;
		if (lengthSq > 0)
			t = Math.Clamp(((x - startX) * segmentX + (z - startZ) * segmentZ) / lengthSq, 0, 1);

		float dx = startX + t * segmentX - x;
		float dz = startZ + t * segmentZ - z;
		return Math.Sqrt(dx * dx + dz * dz);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] allPoints x, z pairs
	//! \return the convex hull as closed x, z pairs (gift wrapping), empty for fewer than 3 points
	protected static array<float> GetConvexHull(notnull array<float> allPoints)
	{
		// Each point once, as a box's top and bottom corners coincide from above
		array<float> points = {};
		for (int point = 0, valueCount = allPoints.Count() - 1; point < valueCount; point += 2)
		{
			bool known = false;
			for (int existing = 0, existingCount = points.Count() - 1; existing < existingCount; existing += 2)
			{
				if (points[existing] == allPoints[point] && points[existing + 1] == allPoints[point + 1])
				{
					known = true;
					break;
				}
			}

			if (!known)
			{
				points.Insert(allPoints[point]);
				points.Insert(allPoints[point + 1]);
			}
		}

		array<float> hull = {};
		int count = points.Count() / 2;
		if (count < 3)
			return hull;

		// Start from the leftmost point, which is on the hull
		int start;
		for (int i = 1; i < count; i++)
		{
			if (points[i * 2] < points[start * 2] || (points[i * 2] == points[start * 2] && points[i * 2 + 1] < points[start * 2 + 1]))
				start = i;
		}

		int current = start;
		for (int step = 0; step < count; step++)
		{
			hull.Insert(points[current * 2]);
			hull.Insert(points[current * 2 + 1]);

			// The next hull point has every other point on one side
			int next = (current + 1) % count;
			for (int candidate = 0; candidate < count; candidate++)
			{
				float cross = (points[next * 2] - points[current * 2]) * (points[candidate * 2 + 1] - points[current * 2 + 1])
					- (points[next * 2 + 1] - points[current * 2 + 1]) * (points[candidate * 2] - points[current * 2]);
				if (cross > 0)
					next = candidate;
			}

			current = next;
			if (current == start)
				break;
		}

		return hull;
	}
}
