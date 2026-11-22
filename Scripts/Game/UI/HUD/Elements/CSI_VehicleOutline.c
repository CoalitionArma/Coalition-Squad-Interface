//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-22
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Top-down outline of a vehicle part as closed lines of x, z pairs (m), cached per prefab and shape
//! 				for the session; plus static helpers for finding parts and their frames.
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutline
{
	protected static const string TURRET_YAW_BONE_PREFIX = "v_turret";	//!< lowercase prefix of turret yaw bones
	protected static const float YAW_BONE_TIE = 0.05;					//!< radians (about 3°); closer yaw bone scores count as tied

	protected static ref map<ResourceName, ref CSI_VehicleOutline> s_mOutlines = new map<ResourceName, ref CSI_VehicleOutline>();	//!< by part prefab
	protected static ref map<string, ref CSI_VehicleOutline> s_mShapes = new map<string, ref CSI_VehicleOutline>();	//!< by CSI_VehicleOutlineBuilder shape key

	protected ref array<ref array<float>> m_aContours = {};
	protected ref array<ref array<int>> m_aTriangles;	//!< per contour, index triples of its points; made on first use
	protected float m_fRadius;	//!< m, origin to farthest point
	protected float m_fRearZ;	//!< m, rearmost z; 0 if nothing is behind the origin

	//------------------------------------------------------------------------------------------------
	//! \param[in] contour closed x, z pairs in meters
	void AddContour(notnull array<float> contour)
	{
		m_aContours.Insert(contour);
		m_aTriangles = null;

		for (int i = 0, count = contour.Count() - 1; i < count; i += 2)
		{
			m_fRadius = Math.Max(m_fRadius, Math.Sqrt(contour[i] * contour[i] + contour[i + 1] * contour[i + 1]));
			m_fRearZ = Math.Min(m_fRearZ, contour[i + 1]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return m, rearmost z in the frame; 0 if nothing is behind the origin
	float GetRearZ()
	{
		return m_fRearZ;
	}

	//------------------------------------------------------------------------------------------------
	array<ref array<float>> GetContours()
	{
		return m_aContours;
	}

	//------------------------------------------------------------------------------------------------
	//! \return per contour, index triples of its points
	array<ref array<int>> GetTriangles()
	{
		if (m_aTriangles)
			return m_aTriangles;

		m_aTriangles = {};
		foreach (array<float> contour : m_aContours)
		{
			array<int> triangles = {};
			Triangulate(contour, triangles);
			m_aTriangles.Insert(triangles);
		}

		return m_aTriangles;
	}

	//------------------------------------------------------------------------------------------------
	float GetRadius()
	{
		return m_fRadius;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return m_aContours.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] prefab
	//! \return null if none is built or received yet
	static CSI_VehicleOutline GetCachedPrefab(ResourceName prefab)
	{
		return s_mOutlines.Get(prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Caches the outline for the prefab, empty ones too; does nothing without a prefab.
	//! \param[in] prefab
	//! \param[in] outline
	static void Cache(ResourceName prefab, CSI_VehicleOutline outline)
	{
		if (outline && !prefab.IsEmpty())
			s_mOutlines.Set(prefab, outline);
	}

	//------------------------------------------------------------------------------------------------
	//! Caches an outline the server sent, unless one is cached already (its triangles are kept).
	//! \param[in] prefab
	//! \param[in] data in ToData's format
	static void CacheData(ResourceName prefab, notnull array<float> data)
	{
		if (prefab.IsEmpty() || s_mOutlines.Contains(prefab))
			return;

		CSI_VehicleOutline outline = new CSI_VehicleOutline();
		int index = 1;
		for (int contour = 0, contourCount = data[0]; contour < contourCount; contour++)
		{
			if (!data.IsIndexValid(index))
				break;

			int valueCount = data[index] * 2;
			index++;
			if (index + valueCount > data.Count())
				break;

			array<float> points = {};
			for (int value = 0; value < valueCount; value++)
			{
				points.Insert(data[index + value]);
			}

			index += valueCount;
			outline.AddContour(points);
		}

		s_mOutlines.Set(prefab, outline);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeKey CSI_VehicleOutlineBuilder's
	//! \return null if none is built yet
	static CSI_VehicleOutline GetShared(string shapeKey)
	{
		return s_mShapes.Get(shapeKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Shares the outline with parts of the same shape (camo and faction variants); does nothing without a key.
	//! \param[in] shapeKey CSI_VehicleOutlineBuilder's
	//! \param[in] outline
	static void Share(string shapeKey, notnull CSI_VehicleOutline outline)
	{
		if (!shapeKey.IsEmpty())
			s_mShapes.Set(shapeKey, outline);
	}

	//------------------------------------------------------------------------------------------------
	//! Forgets every outline, so each session builds or receives its own.
	static void ClearCache()
	{
		s_mOutlines.Clear();
		s_mShapes.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! \return for an RPC: contour count, then per contour its point count and x, z pairs
	array<float> ToData()
	{
		array<float> data = {m_aContours.Count()};
		foreach (array<float> contour : m_aContours)
		{
			data.Insert(contour.Count() / 2);
			data.InsertAll(contour);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the seats of the entity and its non-character descendants, each once.
	//! \param[in] entity
	//! \param[out] seats
	static void GetSeats(notnull IEntity entity, notnull array<BaseCompartmentSlot> seats)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(entity.FindComponent(BaseCompartmentManagerComponent));
		if (compartmentManager)
		{
			array<BaseCompartmentSlot> managerSeats = {};
			compartmentManager.GetCompartments(managerSeats);
			foreach (BaseCompartmentSlot seat : managerSeats)
			{
				if (seat && !seats.Contains(seat))
					seats.Insert(seat);
			}
		}

		IEntity child = entity.GetChildren();
		while (child)
		{
			if (!ChimeraCharacter.Cast(child))
				GetSeats(child, seats);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return empty without a prefab
	static ResourceName GetPrefabName(notnull IEntity entity)
	{
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return ResourceName.Empty;

		return prefabData.GetPrefabName();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	//! \param[out] parts the hull, then each turret, gun mount, mounted gun and weapon rack below it
	static void GetOutlineParts(notnull IEntity vehicle, notnull array<ref CSI_VehicleOutlinePart> parts)
	{
		parts.Clear();
		parts.Insert(new CSI_VehicleOutlinePart(vehicle, GetPartTurret(vehicle, true)));
		AddPieceParts(vehicle, parts);
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddPieceParts(notnull IEntity parent, notnull array<ref CSI_VehicleOutlinePart> parts)
	{
		IEntity child = parent.GetChildren();
		while (child)
		{
			if (IsOutlinedPart(child))
			{
				// Weapon racks don't turn
				if (IsWeaponRack(child))
				{
					parts.Insert(new CSI_VehicleOutlinePart(child, null));
				}
				else
				{
					// Skip a mount aimed by its gun's turret; only the gun is drawn
					TurretComponent turret = GetPartTurret(child, false);
					if (turret && !IsGunMount(child, turret))
						parts.Insert(new CSI_VehicleOutlinePart(child, turret));
				}

				AddPieceParts(child, parts);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity a vehicle's descendant
	//! \return false for crew, rotors and hidden entities (stored items)
	static bool IsOutlinedPart(notnull IEntity entity)
	{
		return !ChimeraCharacter.Cast(entity) && !VehiclePartRotor.Cast(entity) && (entity.GetFlags() & EntityFlags.VISIBLE) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity a vehicle's descendant
	//! \param[in] turret GetPartTurret's
	//! \return true for a mount aimed by its gun's turret (the M1025's ring)
	static bool IsGunMount(notnull IEntity entity, notnull TurretComponent turret)
	{
		IEntity gun = turret.GetOwner();
		if (!gun || gun == entity)
			return false;

		return gun.FindComponent(WeaponComponent) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity a vehicle's descendant
	//! \return true if outlined apart from its parent (turret, gun mount, mounted gun or weapon rack)
	static bool IsOwnPiece(notnull IEntity entity)
	{
		return GetPartTurret(entity, false) != null || IsWeaponRack(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return true if its parent holds it in a weapon rack slot (aircraft hardpoints)
	static bool IsWeaponRack(notnull IEntity entity)
	{
		IEntity parent = entity.GetParent();
		if (!parent)
			return false;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(parent.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return false;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);
		foreach (EntitySlotInfo slot : slots)
		{
			if (SCR_WeaponRackSlotInfo.Cast(slot) && slot.GetAttachedEntity() == entity)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[in] isRoot true for the vehicle itself, which ignores turret controllers
	//! \return the turret its outline turns with: its own, else its controller's; null if none
	static TurretComponent GetPartTurret(notnull IEntity entity, bool isRoot)
	{
		TurretComponent turret = TurretComponent.Cast(entity.FindComponent(TurretComponent));
		if (turret || isRoot)
			return turret;

		TurretControllerComponent turretController = TurretControllerComponent.Cast(entity.FindComponent(TurretControllerComponent));
		if (turretController)
			return turretController.GetTurretComponent();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[in] turret null for a hull or weapon rack
	//! \return radians from the entity's forward to the turret's aim; 0 without a turret
	static float GetFrameYaw(notnull IEntity entity, TurretComponent turret)
	{
		if (!turret)
			return 0;

		vector aim = entity.VectorToLocal(turret.GetAimingDirectionWorld());
		return Math.Atan2(aim[0], aim[2]);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the outline's world frame: the entity's axes turned by GetFrameYaw, placed at the pivot.
	//! \param[in] entity
	//! \param[in] turret null for a hull or weapon rack
	//! \param[in] pivot GetFramePivot's
	//! \param[out] frame x, y and z axes, then the origin
	static void GetWorldFrame(notnull IEntity entity, TurretComponent turret, vector pivot, out vector frame[4])
	{
		vector transform[4];
		entity.GetWorldTransform(transform);

		float frameYaw = GetFrameYaw(entity, turret);
		float cosYaw = Math.Cos(frameYaw);
		float sinYaw = Math.Sin(frameYaw);

		frame[0] = transform[0] * cosYaw - transform[2] * sinYaw;
		frame[1] = transform[1];
		frame[2] = transform[0] * sinYaw + transform[2] * cosYaw;
		frame[3] = pivot.Multiply4(transform);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[in] turret null for a hull or weapon rack
	//! \return pivot in entity space at y 0: the turret's yaw bone, else its gun's parent bone, else the origin
	static vector GetFramePivot(notnull IEntity entity, TurretComponent turret)
	{
		if (!turret || turret.HasRootRotation() || entity.FindComponent(WeaponComponent))
			return vector.Zero;

		Animation animation = entity.GetAnimation();
		if (!animation)
			return vector.Zero;

		array<string> boneNames = {};
		animation.GetBoneNames(boneNames);

		array<string> gunBones = {};
		GetGunBoneNames(entity, turret, gunBones);

		array<string> gunNumbers = {};
		foreach (string gunBone : gunBones)
		{
			string number = GetBoneNumber(gunBone);
			if (!number.IsEmpty() && !gunNumbers.Contains(number))
				gunNumbers.Insert(number);
		}

		float frameYaw = GetFrameYaw(entity, turret);
		vector pivot;
		float bestScore = -1;
		foreach (string boneName : boneNames)
		{
			string lowerName = boneName;
			lowerName.ToLower();
			if (lowerName.IndexOf(TURRET_YAW_BONE_PREFIX) != 0)
				continue;

			TNodeId bone = animation.GetBoneIndex(boneName);
			vector boneMatrix[4];
			if (bone == -1 || !animation.GetBoneMatrix(bone, boneMatrix))
				continue;

			// Score: turn from the aim, minus a full turn if numbered like the gun bones; lowest wins, ties keep the first
			float score = Math.Atan2(boneMatrix[2][0], boneMatrix[2][2]) - frameYaw;
			score = Math.AbsFloat(Math.Atan2(Math.Sin(score), Math.Cos(score)));
			if (gunNumbers.Contains(GetBoneNumber(lowerName)))
				score -= Math.PI2;

			if (bestScore != -1 && score > bestScore - YAW_BONE_TIE)
				continue;

			bestScore = score;
			pivot = boneMatrix[3];
		}

		vector gunParent;
		if (bestScore == -1 && GetGunParentOrigin(entity, gunBones, gunParent))
			pivot = gunParent;

		pivot[1] = 0;
		return pivot;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity a turret
	//! \param[in] turret its TurretComponent
	//! \param[out] boneNames its weapon slot bones, then its camera's
	protected static void GetGunBoneNames(notnull IEntity entity, notnull TurretComponent turret, notnull array<string> boneNames)
	{
		array<Managed> slots = {};
		entity.FindComponents(WeaponSlotComponent, slots);
		foreach (Managed slotComponent : slots)
		{
			WeaponSlotComponent slot = WeaponSlotComponent.Cast(slotComponent);
			if (slot && slot.GetSlotInfo() && !slot.GetSlotInfo().GetBoneName().IsEmpty())
				boneNames.Insert(slot.GetSlotInfo().GetBoneName());
		}

		PointInfo camera = turret.GetCameraAttachmentSlot();
		if (camera && !camera.GetBoneName().IsEmpty())
			boneNames.Insert(camera.GetBoneName());
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the origin of the first gun bone's parent bone, which the gun turns with.
	//! \param[in] entity a turret
	//! \param[in] gunBones GetGunBoneNames'
	//! \param[out] origin in the entity's space
	//! \return false if no gun bone is in its skeleton
	protected static bool GetGunParentOrigin(notnull IEntity entity, notnull array<string> gunBones, out vector origin)
	{
		Animation animation = entity.GetAnimation();
		if (!animation)
			return false;

		foreach (string boneName : gunBones)
		{
			TNodeId bone = animation.GetBoneIndex(boneName);
			vector modelMatrix[4];
			vector localMatrix[4];
			if (bone == -1 || !animation.GetBoneMatrix(bone, modelMatrix) || !animation.GetBoneLocalMatrix(bone, localMatrix))
				continue;

			vector local = localMatrix[3];
			vector parentInBone = Vector(-vector.Dot(local, localMatrix[0]), -vector.Dot(local, localMatrix[1]), -vector.Dot(local, localMatrix[2]));
			origin = parentInBone.Multiply4(modelMatrix);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] boneName
	//! \return what follows its last underscore if that's a number (v_gun_02: 02), else empty
	protected static string GetBoneNumber(string boneName)
	{
		int underscore = boneName.LastIndexOf("_");
		if (underscore < 0 || underscore == boneName.Length() - 1)
			return string.Empty;

		string number = boneName.Substring(underscore + 1, boneName.Length() - underscore - 1);
		if (number.ToInt(-1) < 0)
			return string.Empty;

		return number;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] contour closed x, z pairs
	//! \return signed area (shoelace), positive for counterclockwise
	static float GetArea(notnull array<float> contour)
	{
		float area;
		int count = contour.Count() / 2;
		for (int i = 0; i < count; i++)
		{
			int next = (i + 1) % count;
			area += contour[i * 2] * contour[next * 2 + 1] - contour[next * 2] * contour[i * 2 + 1];
		}

		return area * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! Splits a closed line into triangles (ear clipping).
	//! \param[in] contour closed x, z pairs
	//! \param[out] triangles index triples of its points
	protected static void Triangulate(notnull array<float> contour, notnull array<int> triangles)
	{
		int count = contour.Count() / 2;
		if (count < 3)
			return;

		array<int> remaining = {};
		for (int i = 0; i < count; i++)
		{
			remaining.Insert(i);
		}

		float winding = 1;
		if (GetArea(contour) < 0)
			winding = -1;

		while (remaining.Count() > 3)
		{
			int remainingCount = remaining.Count();
			int ear = -1;
			for (int corner = 0; corner < remainingCount; corner++)
			{
				if (IsEar(contour, remaining, corner, winding))
				{
					ear = corner;
					break;
				}
			}

			// No ear (self-crossing line): cut a corner off anyway
			if (ear < 0)
				ear = 0;

			triangles.Insert(remaining[(ear + remainingCount - 1) % remainingCount]);
			triangles.Insert(remaining[ear]);
			triangles.Insert(remaining[(ear + 1) % remainingCount]);
			remaining.RemoveOrdered(ear);
		}

		triangles.InsertAll(remaining);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] contour closed x, z pairs
	//! \param[in] remaining indices of the points not cut off yet, in order
	//! \param[in] corner index into remaining
	//! \param[in] winding 1 for a counterclockwise line, -1 for a clockwise one
	//! \return true if the corner is convex and holds no other point
	protected static bool IsEar(notnull array<float> contour, notnull array<int> remaining, int corner, float winding)
	{
		int count = remaining.Count();
		int previous = remaining[(corner + count - 1) % count];
		int current = remaining[corner];
		int next = remaining[(corner + 1) % count];

		float ax = contour[previous * 2];
		float az = contour[previous * 2 + 1];
		float bx = contour[current * 2];
		float bz = contour[current * 2 + 1];
		float cx = contour[next * 2];
		float cz = contour[next * 2 + 1];

		if (((bx - ax) * (cz - az) - (bz - az) * (cx - ax)) * winding <= 0)
			return false;

		foreach (int point : remaining)
		{
			if (point == previous || point == current || point == next)
				continue;

			float px = contour[point * 2];
			float pz = contour[point * 2 + 1];
			if (((bx - ax) * (pz - az) - (bz - az) * (px - ax)) * winding >= 0
				&& ((cx - bx) * (pz - bz) - (cz - bz) * (px - bx)) * winding >= 0
				&& ((ax - cx) * (pz - cz) - (az - cz) * (px - cx)) * winding >= 0)
				return false;
		}

		return true;
	}
}
