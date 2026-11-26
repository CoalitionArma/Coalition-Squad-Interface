//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-26
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Builds the top-down outline of a vehicle's hull, turret or weapon rack over a few frames by
//! 				tracing down through a grid over it, past doors, leaving out a hull's small spurs such as mirrors;
//! 				falls back to its colliders' hull, then its bounds.
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
	protected static const float MAX_SPUR_LENGTH = 0.8;			//!< m; a hull's bits up to two cells thin sticking out no longer than this (mirrors) are left out
	protected static const float MAX_SPUR_AREA = 0.25;			//!< square m; larger such bits are kept
	protected static const int MAX_DOOR_PASSES = 8;				//!< door colliders a trace may look past
	protected static const float DOOR_STEP = 0.02;				//!< m a trace moves on past a door collider
	protected static const float BONE_MATCH_SQ = 0.000025;		//!< squared m (5 mm) within which a bone's transform fits a parent bone's
	protected static const float GUN_SAMPLE_SHARE = 0.85;		//!< of the main gun's length, from its back, where IsGunTraced traces
	protected static const float GUN_SAMPLE_HEIGHT = 2;			//!< m above and below the gun IsGunTraced traces
	protected static const int STAGE_CELLS_PER_STEP = 1024;	//!< grid cells a step goes over in the stages after the traces

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected IEntity m_Root;
	protected BaseWorld m_World;					//!< the root's
	protected TurretComponent m_Turret;				//!< the frame follows its aim; null for a hull or weapon rack
	protected bool m_bHull;							//!< the vehicle's own outline, not a turret's, gun's or weapon rack's
	protected ref array<IEntity> m_aParts = {};		//!< the root and its parts with physics, without pieces of their own below it
	protected ref TraceParam m_TraceParam;
	protected ref CSI_VehicleOutline m_Outline;		//!< set once built
	protected bool m_bCacheable;					//!< the outline may be kept for the prefab (not a wreck)
	protected string m_sShapeKey;					//!< GetShapeKey, empty if it can't be shared

	// Doors, left out of the traces open or closed
	protected ref array<IEntity> m_aDoorBoneParts = {};		//!< parts with door bones
	protected ref array<ref set<int>> m_aDoorBones = {};	//!< per door bone part, its door bones' indices in GetBoneNames order, as traces report them
	protected ref array<ref set<int>> m_aDoorPivots = {};	//!< per door bone part, the same bones' ids (GetBoneIndex), as children's GetPivot report them
	protected ref array<IEntity> m_aDoorParts = {};			//!< parts hanging on a door (its glass)
	protected bool m_bHasDoors;								//!< a door bone, a door part or a collider named like a door

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

	// Root space: rotors to skip, wheels and tracks to stamp
	protected ref array<vector> m_aRotors = {};			//!< 3 per rotor: position, axis, then (radius, 1 if main else 0, 0)
	protected bool m_bSkids;							//!< a helicopter on skids, left out of the traces
	protected ref array<float> m_aWheelRectangles = {};	//!< 4 per wheel or track: min x, max x, min z, max z

	// The frame in the world, set each step
	protected vector m_vFrameX;
	protected vector m_vFrameY;
	protected vector m_vFrameZ;
	protected vector m_vFrameOrigin;
	protected vector m_vFramePivot;		//!< CSI_VehicleOutline.GetFramePivot's, in the root's space
	protected float m_fFrameCos;		//!< of the frame's turn from the root
	protected float m_fFrameSin;		//!< of the frame's turn from the root

	// Grid stages after the traces, a slice of STAGE_CELLS_PER_STEP cells a step
	protected int m_iStagePass;						//!< pass of the stage in progress; each stage leaves this and m_iStageCell at 0
	protected int m_iStageCell;						//!< next cell of that pass
	protected ref array<bool> m_aPassCells = {};	//!< CloseGaps: the array its passes alternate with m_aHits
	protected ref array<bool> m_aReached = {};		//!< FillHoles: misses reached from the ring
	protected ref array<int> m_aSpreading = {};		//!< FillHoles: reached misses not spread from yet
	protected bool m_bFillSeeded;					//!< FillHoles: the spread has started
	protected ref set<int> m_aEdgeHits = new set<int>();	//!< FillHoles: a hull's hits next to misses reached from the ring
	protected ref array<bool> m_aSpurs = {};		//!< RemoveSpurs: hits left out as small spurs; their edges aren't refined
	protected int m_iLinkRow = -1;					//!< LinkCells: next row, from -1 as cells run one past the grid

	// Stages done, then marching squares
	protected bool m_bGapsClosed;
	protected bool m_bHolesFilled;
	protected bool m_bLinked;
	protected ref map<int, int> m_mFirstNeighbors = new map<int, int>();	//!< edge point key (GetEdgeKey) to the first edge point linked to it
	protected ref map<int, int> m_mSecondNeighbors = new map<int, int>();	//!< edge point key to the second edge point linked to it
	protected ref array<int> m_aEdgeKeys = {};								//!< every edge point's key, in the order they are refined
	protected int m_iNextEdge;												//!< index in m_aEdgeKeys of the next edge point to refine
	protected ref map<int, float> m_mEdgeFractions = new map<int, float>();	//!< edge point key to where the outline crosses its cell edge (0 to 1); missing if unrefined

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
		m_World = root.GetWorld();
		m_Turret = turret;
		m_vFramePivot = framePivot;
		m_bHull = !turret && !CSI_VehicleOutline.IsWeaponRack(root);
		m_aParts.Insert(root);
		AddParts(root);
		AddRotors();
		AddWheels();
		AddDoors();
		UpdateFrame();

		// A turret's main gun, merged into it (MergePhysics), isn't a part
		bool isGun = root.FindComponent(WeaponComponent) != null;
		IEntity turretGun;
		if (turret && !isGun)
			turretGun = GetMainGun(root);

		if (!InitBounds(turretGun))
			return;

		// Finer for turrets, finest for guns, whose barrels are thin
		float minCellSize = HULL_CELL_SIZE;
		if (isGun)
			minCellSize = GUN_CELL_SIZE;
		else if (turret)
			minCellSize = TURRET_CELL_SIZE;

		InitGrid(minCellSize);
		InitTrace(isGun, turretGun);

		// A part of the same shape was built already: nothing to trace
		m_sShapeKey = GetShapeKey(minCellSize);
		CSI_VehicleOutline shared = CSI_VehicleOutline.GetShared(m_sShapeKey);
		if (!shared)
			return;

		m_Outline = shared;
		m_bCacheable = !IsWrecked();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the trace grid's bounds in the outline's frame.
	//! \param[in] turretGun the turret's main gun, null if none
	//! \return false if nothing has bounds
	protected bool InitBounds(IEntity turretGun)
	{
		// A hull's grid covers only its own model and wheels, cutting off cages around it; turrets and racks cover all parts
		bool hasBounds;
		if (m_bHull)
			hasBounds = AddBounds(m_Root, true);

		// Without a model of its own, every part's bounds
		if (!hasBounds)
		{
			foreach (IEntity part : m_aParts)
			{
				if (AddBounds(part, !hasBounds))
					hasBounds = true;
			}
		}

		// The turret's main gun may reach past the turret
		if (turretGun && !m_aParts.Contains(turretGun) && AddBounds(turretGun, !hasBounds))
			hasBounds = true;

		if (!hasBounds)
			return false;

		// Wheels and tracks sit at the edge, maybe past the meshes
		for (int wheel = 0, wheelCount = m_aWheelRectangles.Count() / 4; wheel < wheelCount; wheel++)
		{
			m_vMins[0] = Math.Min(m_vMins[0], m_aWheelRectangles[wheel * 4]);
			m_vMaxs[0] = Math.Max(m_vMaxs[0], m_aWheelRectangles[wheel * 4 + 1]);
			m_vMins[2] = Math.Min(m_vMins[2], m_aWheelRectangles[wheel * 4 + 2]);
			m_vMaxs[2] = Math.Max(m_vMaxs[2], m_aWheelRectangles[wheel * 4 + 3]);
		}

		if (!m_Turret)
			return true;

		// A turret's frame turns with its aim: cover its bounds both as they are and turned by the aim
		vector rootMins = m_vMins;
		vector rootMaxs = m_vMaxs;
		m_vMins = rootMins - m_vFramePivot;
		m_vMaxs = rootMaxs - m_vFramePivot;
		for (int corner = 0; corner < 8; corner++)
		{
			vector point = RootToFrame(GetBoxCorner(rootMins, rootMaxs, corner));
			m_vMins[0] = Math.Min(m_vMins[0], point[0]);
			m_vMins[2] = Math.Min(m_vMins[2], point[2]);
			m_vMaxs[0] = Math.Max(m_vMaxs[0], point[0]);
			m_vMaxs[2] = Math.Max(m_vMaxs[2], point[2]);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sizes the trace grid to its bounds.
	//! \param[in] minCellSize m
	protected void InitGrid(float minCellSize)
	{
		vector size = m_vMaxs - m_vMins;
		m_fCellSize = Math.Max(minCellSize, Math.Max(size[0], size[2]) / MAX_CELLS);

		// An untraced ring of empty cells around the bounds, so gaps can be closed up to the edge
		m_iColumns = Math.Ceil(size[0] / m_fCellSize) + 2;
		m_iRows = Math.Ceil(size[2] / m_fCellSize) + 2;
		int cellCount = m_iColumns * m_iRows;
		m_aHits.Resize(cellCount);
		m_aStamped.Resize(cellCount);
		m_aVisited.Resize(cellCount);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets up the grid's traces.
	//! \param[in] isGun the root is a weapon
	//! \param[in] turretGun the turret's main gun, null if none
	protected void InitTrace(bool isGun, IEntity turretGun)
	{
		// Sphere traces for a turret, whose barrel may be thinner than a cell
		if (m_Turret)
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
		m_TraceParam.LayerMask = EPhysicsLayerPresets.Projectile;
		if (m_aParts.Count() == 1)
			m_TraceParam.Include = m_Root;
		else
			m_TraceParam.IncludeArray = m_aParts;

		// Every part layer too for a gun, as weapons collide on their own layers, and for a turret whose traces miss its
		// main gun, as a merged gun may collide only there
		if (isGun || (turretGun && !IsGunTraced(turretGun)))
			m_TraceParam.LayerMask = m_TraceParam.LayerMask | GetPartLayers();
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
	//! Notes the parts' door bones, the parts hanging on a door, and whether any collider is named like a door.
	protected void AddDoors()
	{
		foreach (IEntity part : m_aParts)
		{
			if (!part)
				continue;

			// It swings with the door
			if (IsOnDoor(part))
			{
				m_aDoorParts.Insert(part);
				m_bHasDoors = true;
				continue;
			}

			Animation animation = part.GetAnimation();
			if (animation)
			{
				array<string> boneNames = {};
				animation.GetBoneNames(boneNames);
				set<int> doorBones = new set<int>();
				set<int> doorPivots = new set<int>();
				foreach (int index, string boneName : boneNames)
				{
					// Slot bones hold other entities, which IsOnDoor checks
					string lowerBone = boneName;
					lowerBone.ToLower();
					if (!IsDoorName(lowerBone) || lowerBone.StartsWith("slot_"))
						continue;

					doorBones.Insert(index);
					doorPivots.Insert(animation.GetBoneIndex(boneName));
				}

				if (!doorBones.IsEmpty())
				{
					m_aDoorBoneParts.Insert(part);
					m_aDoorBones.Insert(doorBones);
					m_aDoorPivots.Insert(doorPivots);
					m_bHasDoors = true;
				}
			}

			Physics physics = part.GetPhysics();
			if (!physics || m_bHasDoors)
				continue;

			for (int geom = 0, geomCount = physics.GetNumGeoms(); geom < geomCount; geom++)
			{
				if (IsDoorName(physics.GetGeomName(geom)))
				{
					m_bHasDoors = true;
					break;
				}
			}
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
		return m_World.TraceMove(m_TraceParam, null) < 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the collider layers of gun pieces below the parent, so AddColliderHull skips their merged copies.
	//! \param[in] parent
	//! \param[in,out] layers each once
	protected static void AddPieceGunLayers(notnull IEntity parent, notnull array<int> layers)
	{
		IEntity child = parent.GetChildren();
		while (child)
		{
			// Gun pieces sit only below outlined parts: crew, their gear and stored items are skipped
			if (CSI_VehicleOutline.IsOutlinedPart(child))
			{
				if (child.FindComponent(WeaponComponent) && CSI_VehicleOutline.IsOwnPiece(child))
					AddSubtreeLayers(child, layers);
				else
					AddPieceGunLayers(child, layers);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the collider layers of the entity and its descendants.
	//! \param[in] entity
	//! \param[in,out] layers each once
	protected static void AddSubtreeLayers(notnull IEntity entity, notnull array<int> layers)
	{
		Physics physics = entity.GetPhysics();
		if (physics)
		{
			for (int geom = 0, geomCount = physics.GetNumGeoms(); geom < geomCount; geom++)
			{
				int layer = physics.GetGeomInteractionLayer(geom);
				if (!layers.Contains(layer))
					layers.Insert(layer);
			}
		}

		IEntity child = entity.GetChildren();
		while (child)
		{
			AddSubtreeLayers(child, layers);
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
	//! Runs the next traces, and builds the outline after the last one.
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

		UpdateFrame();
		int traces;

		// Trace the grid, then finish it a slice a step, as each stage goes over every cell
		if (!m_bGridDone)
		{
			m_bGridDone = FloodGrid(maxTraces);
			return false;
		}

		if (!m_bGapsClosed)
		{
			m_bGapsClosed = CloseGaps();
			return false;
		}

		// Spurs go in the step that fills the last holes, taking no step of their own
		if (!m_bHolesFilled)
		{
			m_bHolesFilled = FillHoles();
			if (m_bHolesFilled && m_bHull)
				RemoveSpurs();

			return false;
		}

		if (!m_bLinked)
		{
			m_bLinked = LinkCells();
			return false;
		}

		// Each edge point, more closely
		while (m_iNextEdge < m_aEdgeKeys.Count() && traces < maxTraces)
		{
			traces += RefineEdge(m_aEdgeKeys[m_iNextEdge]);
			m_iNextEdge++;
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
	//! Traces from the empty ring inward, spreading from misses (FillHoles fills the cells it never reaches).
	//! \param[in] maxTraces traces this call may run
	//! \return true once every reachable cell is traced
	protected bool FloodGrid(int maxTraces)
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

		return m_aFloodStack.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] count cells in the pass
	//! \return the cell this step's slice stops at
	protected int GetSliceEnd(int count)
	{
		int end = m_iStageCell + STAGE_CELLS_PER_STEP;
		if (end > count)
			return count;

		return end;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] end the cell the slice stopped at
	//! \param[in] count cells in the pass
	//! \return true once the pass is done, so the next one starts at cell 0
	protected bool FinishSlice(int end, int count)
	{
		if (end < count)
		{
			m_iStageCell = end;
			return false;
		}

		m_iStageCell = 0;
		return true;
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
	//! Places the outline's frame in the world and reads its turn from the root, for this step's traces.
	protected void UpdateFrame()
	{
		vector frame[4];
		CSI_VehicleOutline.GetWorldFrame(m_Root, m_Turret, m_vFramePivot, frame);
		m_vFrameX = frame[0];
		m_vFrameY = frame[1];
		m_vFrameZ = frame[2];
		m_vFrameOrigin = frame[3];

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
	//! \param[in] point in the root's space
	//! \return the point in the frame
	protected vector RootToFrame(vector point)
	{
		vector offset = point - m_vFramePivot;
		return Vector(offset[0] * m_fFrameCos - offset[2] * m_fFrameSin, offset[1], offset[0] * m_fFrameSin + offset[2] * m_fFrameCos);
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
	//! \return true if a trace down between the heights hits the root or its parts, doors aside
	protected bool TraceSegment(float x, float z, float top, float bottom)
	{
		vector column = m_vFrameOrigin + m_vFrameX * x + m_vFrameZ * z;
		m_TraceParam.Start = column + m_vFrameY * top;
		m_TraceParam.End = column + m_vFrameY * bottom;

		if (m_World.TraceMove(m_TraceParam, null) >= 1)
			return false;

		return !m_bHasDoors || !IsDoorHit() || TracePastDoors();
	}

	//------------------------------------------------------------------------------------------------
	//! Traces m_TraceParam's segment again nearest contact first, going on past each door collider.
	//! \return true if something other than a door lies on it
	protected bool TracePastDoors()
	{
		vector end = m_TraceParam.End;
		TraceFlags flags = m_TraceParam.Flags;
		m_TraceParam.Flags = TraceFlags.ENTS;

		bool hit;
		for (int pass = 0; pass < MAX_DOOR_PASSES; pass++)
		{
			float fraction = m_World.TraceMove(m_TraceParam, null);
			if (fraction >= 1)
				break;

			if (!IsDoorHit())
			{
				hit = true;
				break;
			}

			vector doorPoint = m_TraceParam.Start + (end - m_TraceParam.Start) * fraction;
			if (vector.Distance(doorPoint, end) <= DOOR_STEP)
				break;

			m_TraceParam.Start = doorPoint + (end - doorPoint).Normalized() * DOOR_STEP;
		}

		m_TraceParam.Flags = flags;
		return hit;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity a part
	//! \return true if it hangs on a door bone of a part above it (through any entities between), or below a door part
	protected bool IsOnDoor(notnull IEntity entity)
	{
		IEntity child = entity;
		IEntity parent = entity.GetParent();
		while (parent)
		{
			if (m_aDoorParts.Contains(parent))
				return true;

			int parentIndex = m_aDoorBoneParts.Find(parent);
			if (parentIndex >= 0 && IsUnderDoorBone(parent.GetAnimation(), child.GetPivot(), m_aDoorPivots[parentIndex]))
				return true;

			if (parent == m_Root)
				return false;

			child = parent;
			parent = parent.GetParent();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] animation
	//! \param[in] bone id (GetBoneIndex)
	//! \param[in] doorBones ids
	//! \return true if the bone is a door bone, is named like one, or hangs on one (a door's glass slot)
	protected static bool IsUnderDoorBone(Animation animation, int bone, notnull set<int> doorBones)
	{
		if (!animation || bone == -1)
			return false;

		if (doorBones.Contains(bone))
			return true;

		array<string> boneNames = {};
		animation.GetBoneNames(boneNames);
		foreach (string boneName : boneNames)
		{
			if (animation.GetBoneIndex(boneName) != bone)
				continue;

			if (IsDoorName(boneName))
				return true;

			break;
		}

		foreach (int doorBone : doorBones)
		{
			if (IsParentBone(animation, bone, doorBone))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The animation can't name a bone's parent, but a bone's model transform is its local one on top of its parent's.
	//! \param[in] animation
	//! \param[in] bone id
	//! \param[in] parent id of the candidate parent
	//! \return true if the candidate is the bone's parent
	protected static bool IsParentBone(notnull Animation animation, int bone, int parent)
	{
		vector local[4];
		vector model[4];
		vector parentModel[4];
		if (!animation.GetBoneLocalMatrix(bone, local) || !animation.GetBoneMatrix(bone, model) || !animation.GetBoneMatrix(parent, parentModel))
			return false;

		return vector.DistanceSq(local[3].Multiply4(parentModel), model[3]) < BONE_MATCH_SQ
			&& vector.DistanceSq(local[2].Multiply3(parentModel), model[2]) < BONE_MATCH_SQ
			&& vector.DistanceSq(local[0].Multiply3(parentModel), model[0]) < BONE_MATCH_SQ;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the last trace hit a door: a part hanging on one, a collider named like one, or one on a door bone
	protected bool IsDoorHit()
	{
		if (m_aDoorParts.Contains(m_TraceParam.TraceEnt))
			return true;

		if (IsDoorName(m_TraceParam.ColliderName))
			return true;

		int partIndex = m_aDoorBoneParts.Find(m_TraceParam.TraceEnt);
		return partIndex >= 0 && m_aDoorBones[partIndex].Contains(m_TraceParam.NodeIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] name a bone or collider name, any case
	//! \return true if it names a door
	protected static bool IsDoorName(string name)
	{
		string lowerName = name;
		lowerName.ToLower();
		return lowerName.Contains("door") && !lowerName.Contains("indoor");
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
	//! \return true once done
	protected bool CloseGaps()
	{
		if (m_iStagePass == 0 && m_iStageCell == 0)
			m_aPassCells.Resize(m_aHits.Count());

		// Grow along rows, then columns; shrink likewise; each pass from one array into the other
		bool passDone;
		switch (m_iStagePass)
		{
			case 0: passDone = SpreadHits(m_aHits, m_aPassCells, true, true); break;
			case 1: passDone = SpreadHits(m_aPassCells, m_aHits, false, true); break;
			case 2: passDone = SpreadHits(m_aHits, m_aPassCells, true, false); break;
			case 3: passDone = SpreadHits(m_aPassCells, m_aHits, false, false); break;
		}

		if (!passDone)
			return false;

		m_iStagePass++;
		if (m_iStagePass < 4)
			return false;

		m_iStagePass = 0;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills the misses the ring doesn't reach through misses (holes CloseGaps shut in, cells FloodGrid never reached),
	//! so no shape keeps a hole: spreads through misses from the ring, then fills the rest.
	//! \return true once done
	protected bool FillHoles()
	{
		int count = m_aHits.Count();
		if (!m_bFillSeeded)
		{
			m_bFillSeeded = true;
			m_aReached.Clear();
			m_aReached.Resize(count);
			m_aSpreading.Clear();

			// The ring is all misses: spread from it through misses, diagonally too
			for (int column = 0; column < m_iColumns; column++)
			{
				PushOpenCell(column, 0);
				PushOpenCell(column, m_iRows - 1);
			}

			for (int row = 0; row < m_iRows; row++)
			{
				PushOpenCell(0, row);
				PushOpenCell(m_iColumns - 1, row);
			}
		}

		if (m_iStagePass == 0)
		{
			// Each cell looks at its 9 neighbors
			int budget = STAGE_CELLS_PER_STEP / 9;
			while (!m_aSpreading.IsEmpty() && budget > 0)
			{
				budget--;
				int last = m_aSpreading.Count() - 1;
				int cell = m_aSpreading[last];
				m_aSpreading.Remove(last);

				int cellColumn = cell % m_iColumns;
				int cellRow = cell / m_iColumns;
				for (int neighborRow = cellRow - 1; neighborRow <= cellRow + 1; neighborRow++)
				{
					for (int neighborColumn = cellColumn - 1; neighborColumn <= cellColumn + 1; neighborColumn++)
					{
						PushOpenCell(neighborColumn, neighborRow);
					}
				}
			}

			if (m_aSpreading.IsEmpty())
				m_iStagePass = 1;

			return false;
		}

		int end = GetSliceEnd(count);
		for (int index = m_iStageCell; index < end; index++)
		{
			if (!m_aReached[index])
				m_aHits[index] = true;
		}

		if (!FinishSlice(end, count))
			return false;

		m_iStagePass = 0;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Queues a miss for FillHoles if it isn't reached yet, and notes a hull's hits next to it (its outer edge).
	//! \param[in] column
	//! \param[in] row
	protected void PushOpenCell(int column, int row)
	{
		if (!IsInGrid(column, row))
			return;

		int cell = row * m_iColumns + column;
		if (m_aHits[cell])
		{
			if (m_bHull)
				m_aEdgeHits.Insert(cell);

			return;
		}

		if (m_aReached[cell])
			return;

		m_aReached[cell] = true;
		m_aSpreading.Insert(cell);
	}

	//------------------------------------------------------------------------------------------------
	//! Leaves out a hull's small spurs such as mirrors, grouped from its thin outer edge hits (GroupSpur).
	protected void RemoveSpurs()
	{
		m_aSpurs.Resize(m_aHits.Count());

		// Thin: no full 3 by 3 block of hits covers it, so an opening (shrink, then grow a cell) would take it off; only
		// edge hits can be thin, as any other hit centers a full block
		set<int> thin = new set<int>();
		foreach (int cell : m_aEdgeHits)
		{
			if (!IsInFullBlock(cell % m_iColumns, cell / m_iColumns))
				thin.Insert(cell);
		}

		set<int> grouped = new set<int>();
		foreach (int thinCell : thin)
		{
			if (!grouped.Contains(thinCell))
				GroupSpur(thinCell, thin, grouped);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] column
	//! \param[in] row
	//! \return true if the cell lies in a 3 by 3 block of hits
	protected bool IsInFullBlock(int column, int row)
	{
		for (int centerRow = row - 1; centerRow <= row + 1; centerRow++)
		{
			for (int centerColumn = column - 1; centerColumn <= column + 1; centerColumn++)
			{
				if (IsFullBlockCenter(centerColumn, centerRow))
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] column
	//! \param[in] row
	//! \return true if the cell and its 8 neighbors are all hits
	protected bool IsFullBlockCenter(int column, int row)
	{
		if (column < 1 || row < 1 || column > m_iColumns - 2 || row > m_iRows - 2)
			return false;

		for (int blockRow = row - 1; blockRow <= row + 1; blockRow++)
		{
			int rowStart = blockRow * m_iColumns;
			if (!m_aHits[rowStart + column - 1] || !m_aHits[rowStart + column] || !m_aHits[rowStart + column + 1])
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Gathers a thin cell's spur and leaves it out if it's small, holds no wheel and meets the body in one place.
	//! \param[in] first a thin cell not grouped yet
	//! \param[in] thin every thin cell
	//! \param[in,out] grouped the thin cells grouped so far
	protected void GroupSpur(int first, notnull set<int> thin, notnull set<int> grouped)
	{
		array<int> spur = GetTouchingGroup(first, thin, grouped);
		int minColumn = first % m_iColumns;
		int maxColumn = minColumn;
		int minRow = first / m_iColumns;
		int maxRow = minRow;
		bool hasWheel = false;
		foreach (int cell : spur)
		{
			int column = cell % m_iColumns;
			int row = cell / m_iColumns;
			if (column < minColumn)
				minColumn = column;

			if (column > maxColumn)
				maxColumn = column;

			if (row < minRow)
				minRow = row;

			if (row > maxRow)
				maxRow = row;

			if (m_aStamped[cell])
				hasWheel = true;
		}

		// A thin stretch between two thicker ones, like a tail boom's, meets the body twice and is kept
		float length = (Math.Max(maxColumn - minColumn, maxRow - minRow) + 1) * m_fCellSize;
		float area = spur.Count() * m_fCellSize * m_fCellSize;
		if (hasWheel || length > MAX_SPUR_LENGTH || area > MAX_SPUR_AREA || CountBodyContacts(spur, thin) > 1)
			return;

		foreach (int spurCell : spur)
		{
			m_aHits[spurCell] = false;
			m_aSpurs[spurCell] = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] spur its cells
	//! \param[in] thin every thin cell; the body is the hits that aren't
	//! \return separate places where the spur meets the body
	protected int CountBodyContacts(notnull array<int> spur, notnull set<int> thin)
	{
		// The spur's cells next to the body
		set<int> contacts = new set<int>();
		foreach (int cell : spur)
		{
			int column = cell % m_iColumns;
			int row = cell / m_iColumns;
			for (int neighborRow = row - 1; neighborRow <= row + 1; neighborRow++)
			{
				for (int neighborColumn = column - 1; neighborColumn <= column + 1; neighborColumn++)
				{
					if (!IsInGrid(neighborColumn, neighborRow))
						continue;

					int neighbor = neighborRow * m_iColumns + neighborColumn;
					if (m_aHits[neighbor] && !thin.Contains(neighbor))
						contacts.Insert(cell);
				}
			}
		}

		// Groups of touching contact cells
		int groups = 0;
		set<int> counted = new set<int>();
		foreach (int contact : contacts)
		{
			if (counted.Contains(contact))
				continue;

			GetTouchingGroup(contact, contacts, counted);
			groups++;
		}

		return groups;
	}

	//------------------------------------------------------------------------------------------------
	//! Gathers the first cell and the cells of the set joined to it through touching cells, diagonal neighbors too.
	//! \param[in] first a cell of the set not taken yet
	//! \param[in] cells the set
	//! \param[in,out] taken cells already in a group; the new group's are added
	//! \return the new group's cells
	protected array<int> GetTouchingGroup(int first, notnull set<int> cells, notnull set<int> taken)
	{
		taken.Insert(first);
		array<int> group = {first};
		for (int index = 0; index < group.Count(); index++)
		{
			int column = group[index] % m_iColumns;
			int row = group[index] / m_iColumns;
			for (int neighborRow = row - 1; neighborRow <= row + 1; neighborRow++)
			{
				for (int neighborColumn = column - 1; neighborColumn <= column + 1; neighborColumn++)
				{
					if (!IsInGrid(neighborColumn, neighborRow))
						continue;

					int neighbor = neighborRow * m_iColumns + neighborColumn;
					if (!cells.Contains(neighbor) || taken.Contains(neighbor))
						continue;

					taken.Insert(neighbor);
					group.Insert(neighbor);
				}
			}
		}

		return group;
	}

	//------------------------------------------------------------------------------------------------
	//! One slice of a pass growing or shrinking the hits by a cell along rows or columns; outside the grid counts as misses.
	//! \param[in] from
	//! \param[out] to
	//! \param[in] alongRows else along columns
	//! \param[in] grow else shrink
	//! \return true once the pass is done
	protected bool SpreadHits(notnull array<bool> from, notnull array<bool> to, bool alongRows, bool grow)
	{
		int count = from.Count();
		int end = GetSliceEnd(count);
		for (int cell = m_iStageCell; cell < end; cell++)
		{
			bool before;
			bool after;
			if (alongRows)
			{
				int column = cell % m_iColumns;
				before = column > 0 && from[cell - 1];
				after = column < m_iColumns - 1 && from[cell + 1];
			}
			else
			{
				before = cell >= m_iColumns && from[cell - m_iColumns];
				after = cell + m_iColumns < count && from[cell + m_iColumns];
			}

			if (grow)
				to[cell] = from[cell] || before || after;
			else
				to[cell] = from[cell] && before && after;
		}

		return FinishSlice(end, count);
	}

	//------------------------------------------------------------------------------------------------
	//! Links the edge points around the hit cells (marching squares), a slice of rows a step.
	//! \return true once done
	protected bool LinkCells()
	{
		int rowsPerStep = STAGE_CELLS_PER_STEP / (m_iColumns + 1);
		if (rowsPerStep < 1)
			rowsPerStep = 1;

		int endRow = m_iLinkRow + rowsPerStep;
		if (endRow > m_iRows)
			endRow = m_iRows;

		// Cells run one past the grid on every side, so every line closes
		for (int row = m_iLinkRow; row < endRow; row++)
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

		m_iLinkRow = endRow;
		if (m_iLinkRow < m_iRows)
			return false;

		foreach (int key, int neighbor : m_mFirstNeighbors)
		{
			m_aEdgeKeys.Insert(key);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds where the outline crosses the edge point's cell edge, unless it's next to a wheel, track or spur left out.
	//! \param[in] key GetEdgeKey
	//! \return traces run
	protected int RefineEdge(int key)
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

		// Wheels and tracks aren't traced, and traces would find a spur left out: those edges stay in the middle
		bool startHit = IsHit(column, row) == 1;
		if ((startHit && IsStamped(column, row)) || (!startHit && IsStamped(endColumn, endRow)))
			return 0;

		if (IsSpur(column, row) || IsSpur(endColumn, endRow))
			return 0;

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
		return REFINE_STEPS;
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
		array<int> pieceGunLayers = {};
		AddPieceGunLayers(m_Root, pieceGunLayers);

		int geomsSeen;
		int geomsUsed;
		foreach (IEntity part : m_aParts)
		{
			if (!part || m_aDoorParts.Contains(part))
				continue;

			Physics physics = part.GetPhysics();
			if (!physics)
				continue;

			// Direct bounds are in the body's physics space
			vector bodyTransform[4];
			physics.GetDirectWorldTransform(bodyTransform);

			for (int geom = 0, geomCount = physics.GetNumGeoms(); geom < geomCount; geom++)
			{
				geomsSeen++;
				if (pieceGunLayers.Contains(physics.GetGeomInteractionLayer(geom)) || IsDoorName(physics.GetGeomName(geom)))
					continue;

				geomsUsed++;

				vector geomTransform[4];
				physics.GetGeomDirectTransform(geom, geomTransform);

				vector mins, maxs;
				physics.GetGeomDirectBounds(geom, mins, maxs);
				if (mins == maxs)
					continue;

				for (int corner = 0; corner < 8; corner++)
				{
					vector point = m_Root.CoordToLocal(GetBoxCorner(mins, maxs, corner).Multiply4(geomTransform).Multiply4(bodyTransform));
					vector framePoint = RootToFrame(point);
					points.Insert(framePoint[0]);
					points.Insert(framePoint[2]);
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
		if (!IsInGrid(column, row))
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
		if (!IsInGrid(column, row))
			return false;

		return m_aStamped[row * m_iColumns + column];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] column
	//! \param[in] row
	//! \return true if RemoveSpurs left the cell out
	protected bool IsSpur(int column, int row)
	{
		if (m_aSpurs.IsEmpty() || !IsInGrid(column, row))
			return false;

		return m_aSpurs[row * m_iColumns + column];
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsInGrid(int column, int row)
	{
		return column >= 0 && row >= 0 && column < m_iColumns && row < m_iRows;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] column of the edge's first trace (-1 to m_iColumns)
	//! \param[in] row of the edge's first trace (-1 to m_iRows)
	//! \param[in] alongZ the edge runs to the next row, else to the next column
	//! \return a key for the edge's point
	protected int GetEdgeKey(int column, int row, bool alongZ)
	{
		// Twice the cell's index in a grid one cell wider on every side, plus 1 along z
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

		// Unrefined edges stay in the middle; Find clears the value when the key is missing
		float fraction;
		if (!m_mEdgeFractions.Find(key, fraction))
			fraction = 0.5;

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
