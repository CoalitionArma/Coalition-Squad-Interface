//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Coalition Squad Interface
//!	\Description: 	One hull, turret, gun or weapon rack outline drawn by CSI_VehicleOutlineComponent, with its
//! 				placement and cached layout.
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutlinePiece
{
	ref CSI_VehicleOutline m_Outline;
	CSI_EVehicleOutlineKind m_eKind;

	// SetPiece's placement, m: a point (x, z) goes to m_vAxisX * x + m_vAxisZ * z + m_vOffset (right in [0], forward in [2])
	vector m_vAxisX = vector.Right;
	vector m_vAxisZ = vector.Forward;
	vector m_vOffset;
	vector m_vLaidOutAxisX;								//!< m_vAxisX when last laid out
	vector m_vLaidOutAxisZ;								//!< m_vAxisZ when last laid out
	vector m_vLaidOutOffset;							//!< m_vOffset when last laid out
	bool m_bLaidOut;									//!< the layout below is current

	// Layout in px from the canvas center, in SetRotation's frame (y down): line runs, one band each, and the fill
	ref array<ref array<float>> m_aRuns = {};
	ref array<int> m_aRunBands = {};
	ref array<bool> m_aRunsClosed = {};
	int m_iRunCount;
	ref array<float> m_aFillPoints = {};
	ref array<int> m_aFillIndices = {};

	// Turned to the view, absolute px: what the canvas draws
	ref array<ref array<float>> m_aTurnedRuns = {};
	ref TriMeshDrawCommand m_Fill;
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Coalition Squad Interface
//!	\Description: 	One outlined part of a vehicle (hull, turret, gun or weapon rack): its entity, turret and
//! 				outline, or on the server the builder still making it.
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutlinePart
{
	IEntity m_Entity;
	ResourceName m_sPrefab;								//!< the entity's, empty without one
	TurretComponent m_Turret;							//!< the frame follows its aim; null for the hull and weapon racks
	vector m_vFramePivot;								//!< CSI_VehicleOutline.GetFramePivot's
	CSI_EVehicleOutlineKind m_eKind;
	ref CSI_VehicleOutline m_Outline;					//!< null until built or received
	ref CSI_VehicleOutlineBuilder m_Builder;

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[in] turret null for the hull and weapon racks
	void CSI_VehicleOutlinePart(notnull IEntity entity, TurretComponent turret)
	{
		m_Entity = entity;
		m_sPrefab = CSI_VehicleOutline.GetPrefabName(entity);
		m_Turret = turret;
		m_vFramePivot = CSI_VehicleOutline.GetFramePivot(entity, turret);

		if (!turret)
			m_eKind = CSI_EVehicleOutlineKind.HULL;
		else if (entity.FindComponent(WeaponComponent))
			m_eKind = CSI_EVehicleOutlineKind.GUN;
		else
			m_eKind = CSI_EVehicleOutlineKind.TURRET;

		m_Outline = CSI_VehicleOutline.GetCachedPrefab(m_sPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Server only.
	void StartBuilding()
	{
		if (!m_Builder && m_Entity)
			m_Builder = new CSI_VehicleOutlineBuilder(m_Entity, m_Turret, m_vFramePivot);
	}

	//------------------------------------------------------------------------------------------------
	//! Runs one build step, unless a part already stepped this frame.
	//! \param[in,out] stepped a part stepped this frame; set if this one does
	//! \param[in] maxTraces traces the step may run, 0 for none this frame
	//! \return true once built; then call FinishBuild
	bool StepBuild(inout bool stepped, int maxTraces)
	{
		if (m_Builder.GetOutline())
			return true;

		if (stepped || maxTraces <= 0)
			return false;

		stepped = true;
		return m_Builder.Step(maxTraces);
	}

	//------------------------------------------------------------------------------------------------
	//! Takes the finished outline and caches it for the prefab, unless it's a wreck's.
	//! \return true if it was cached
	bool FinishBuild()
	{
		m_Outline = m_Builder.GetOutline();
		bool cacheable = m_Builder.IsCacheable();
		m_Builder = null;

		if (!m_Entity || !cacheable)
			return false;

		CSI_VehicleOutline.Cache(m_sPrefab, m_Outline);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Takes the cached outline once there is one, dropping any build.
	//! \return true if the part has an outline
	bool TakeCached()
	{
		if (m_Outline)
			return true;

		m_Outline = CSI_VehicleOutline.GetCachedPrefab(m_sPrefab);
		if (!m_Outline)
			return false;

		m_Builder = null;
		return true;
	}
}
