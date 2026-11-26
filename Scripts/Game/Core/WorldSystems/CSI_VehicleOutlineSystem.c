//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Server system that builds the radar's vehicle outlines once per prefab, a vehicle at a time with
//! 				few traces per second, and sends them to players who get in (CSI_RplToAuthorityManager).
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutlineSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected static const int TRACES_PER_SECOND = 720;		//!< build traces a second, run in steps of TRACES_PER_STEP
	protected static const int TRACES_PER_STEP = 20;		//!< traces in one build step, at most one step a frame
	protected static const float SPAWN_SETTLE_TIME = 3;		//!< seconds after registering before outlining, for its parts to spawn
	protected static const float BUILD_TIMEOUT = 90;		//!< seconds over all tries before a vehicle is dropped

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected SCR_EditableEntityCore m_EntityCore;	//!< listened to for new and removed vehicles
	protected bool m_bStarted;						//!< the vehicles already known were queued
	protected float m_fTime;						//!< seconds since the system started
	protected float m_fTraceAllowance;				//!< traces saved up, at most two steps' worth

	protected ref array<ref CSI_QueuedVehicle> m_aQueue = {};	//!< vehicles waiting to be outlined, oldest first
	protected ref set<ResourceName> m_aQueuedPrefabs = new set<ResourceName>();	//!< vehicle prefabs queued or outlined, so each is done once

	// The vehicle being outlined
	protected IEntity m_CurrentVehicle;
	protected ResourceName m_sCurrentPrefab;
	protected float m_fCurrentStartTime;			//!< when its first try started
	protected ref array<ref CSI_VehicleOutlinePart> m_aCurrentParts = {};

	protected ref map<ResourceName, ref array<CSI_RplToAuthorityManager>> m_mWaitingRequesters = new map<ResourceName, ref array<CSI_RplToAuthorityManager>>();	//!< players waiting, by part prefab

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INIT METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Server)
			.AddPoint(WorldSystemPoint.Frame);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnStopped()
	{
		if (m_EntityCore)
		{
			m_EntityCore.Event_OnEntityRegistered.Remove(OnEntityRegistered);
			m_EntityCore.Event_OnEntityUnregistered.Remove(OnEntityUnregistered);
		}

		m_EntityCore = null;
		super.OnStopped();
	}

	//------------------------------------------------------------------------------------------------
	void ~CSI_VehicleOutlineSystem()
	{
		// Outlines aren't kept from one session to the next
		CSI_VehicleOutline.ClearCache();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		m_fTime += timeSlice;

		if (!m_bStarted)
			QueueKnownVehicles();

		if (m_aCurrentParts.IsEmpty() && !StartNextVehicle())
			return;

		StepCurrentVehicle(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Queues the vehicles already known and listens for new and removed ones.
	protected void QueueKnownVehicles()
	{
		m_bStarted = true;

		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!m_EntityCore)
		{
			Print("CSI_VehicleOutlineSystem: editable entity core not found, vehicles are only outlined when a player gets in", LogLevel.WARNING);
			return;
		}

		m_EntityCore.Event_OnEntityRegistered.Insert(OnEntityRegistered);
		m_EntityCore.Event_OnEntityUnregistered.Insert(OnEntityUnregistered);

		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		m_EntityCore.GetAllEntities(entities);
		foreach (SCR_EditableEntityComponent entity : entities)
		{
			OnEntityRegistered(entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntityRegistered(SCR_EditableEntityComponent entity)
	{
		// A wreck's outline isn't its prefab's
		if (!entity || entity.GetEntityType() != EEditableEntityType.VEHICLE || entity.IsDestroyed())
			return;

		IEntity vehicle = entity.GetOwner();
		if (!vehicle)
			return;

		ResourceName prefab = CSI_VehicleOutline.GetPrefabName(vehicle);
		if (prefab.IsEmpty() || m_aQueuedPrefabs.Contains(prefab))
			return;

		m_aQueuedPrefabs.Insert(prefab);
		QueueVehicle(vehicle, prefab, m_fTime + SPAWN_SETTLE_TIME, false, -1);
	}

	//------------------------------------------------------------------------------------------------
	//! Drops a removed vehicle from the queue so another of its prefab can be outlined.
	//! \param[in] entity
	protected void OnEntityUnregistered(SCR_EditableEntityComponent entity)
	{
		if (!entity)
			return;

		int index = FindQueued(entity.GetOwner());
		if (index < 0)
			return;

		ReleasePrefab(m_aQueue[index].m_sPrefab);
		m_aQueue.RemoveOrdered(index);
	}

	//------------------------------------------------------------------------------------------------
	//! Takes the first queued vehicle that may start, dropping those deleted meanwhile.
	//! \return false if none may start yet
	protected bool StartNextVehicle()
	{
		for (int index = 0; index < m_aQueue.Count(); index++)
		{
			CSI_QueuedVehicle queued = m_aQueue[index];
			if (queued.m_fStartTime > m_fTime)
				continue;

			IEntity vehicle = queued.m_Vehicle;
			ResourceName prefab = queued.m_sPrefab;
			float firstTryTime = queued.m_fFirstTryTime;
			m_aQueue.RemoveOrdered(index);
			index--;

			if (!vehicle)
			{
				ReleasePrefab(prefab);
				continue;
			}

			m_CurrentVehicle = vehicle;
			m_sCurrentPrefab = prefab;
			m_fCurrentStartTime = firstTryTime;
			if (firstTryTime < 0)
				m_fCurrentStartTime = m_fTime;

			// Parts cached since a player asked answer them now; the rest get builders in StepCurrentVehicle
			CSI_VehicleOutline.GetOutlineParts(vehicle, m_aCurrentParts);
			foreach (CSI_VehicleOutlinePart part : m_aCurrentParts)
			{
				if (part.m_Outline)
					SendToWaiting(part.m_sPrefab, part.m_Outline);
			}

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Steps the current vehicle's builds and sends finished outlines to waiting players.
	//! \param[in] timeSlice
	protected void StepCurrentVehicle(float timeSlice)
	{
		// Deleted or timed out: free its prefab and its waiting players, who ask again later
		if (!m_CurrentVehicle || m_fTime - m_fCurrentStartTime > BUILD_TIMEOUT)
		{
			foreach (CSI_VehicleOutlinePart droppedPart : m_aCurrentParts)
			{
				if (!droppedPart.m_Outline)
					m_mWaitingRequesters.Remove(droppedPart.m_sPrefab);
			}

			ReleasePrefab(m_sCurrentPrefab);
			EndCurrentVehicle();
			return;
		}

		// Step only once a step's traces have built up (later build stages too); the surplus carries over
		m_fTraceAllowance = Math.Min(m_fTraceAllowance + TRACES_PER_SECOND * timeSlice, 2 * TRACES_PER_STEP);
		int maxTraces;
		if (m_fTraceAllowance >= TRACES_PER_STEP)
			maxTraces = TRACES_PER_STEP;

		int building;
		bool stepped;
		bool builderStarted;
		foreach (CSI_VehicleOutlinePart part : m_aCurrentParts)
		{
			if (part.m_Outline)
				continue;

			// Cached meanwhile by another part of the same prefab
			if (part.TakeCached())
			{
				SendToWaiting(part.m_sPrefab, part.m_Outline);
				continue;
			}

			// One builder made a frame, as each walks the whole vehicle
			if (!part.m_Builder)
			{
				if (!part.m_Entity)
					continue;

				building++;
				if (!builderStarted)
				{
					part.StartBuilding();
					builderStarted = true;
				}

				continue;
			}

			if (part.StepBuild(stepped, maxTraces))
			{
				FinishPart(part);
				continue;
			}

			building++;
		}

		if (stepped)
			m_fTraceAllowance -= TRACES_PER_STEP;

		if (building == 0)
			EndCurrentVehicle();
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps a finished outline and sends it to waiting players; a wreck's frees its prefab and players instead.
	//! \param[in] part
	protected void FinishPart(notnull CSI_VehicleOutlinePart part)
	{
		if (part.FinishBuild())
		{
			SendToWaiting(part.m_sPrefab, part.m_Outline);
			return;
		}

		m_mWaitingRequesters.Remove(part.m_sPrefab);
		ReleasePrefab(m_sCurrentPrefab);
	}

	//------------------------------------------------------------------------------------------------
	protected void EndCurrentVehicle()
	{
		m_CurrentVehicle = null;
		m_aCurrentParts.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	//! \param[in] prefab the vehicle's
	//! \param[in] startTime when it may start, in m_fTime
	//! \param[in] first ahead of the queue, else at its end
	//! \param[in] firstTryTime when its first try started, -1 if it hasn't been tried
	protected void QueueVehicle(notnull IEntity vehicle, ResourceName prefab, float startTime, bool first, float firstTryTime)
	{
		CSI_QueuedVehicle queued = new CSI_QueuedVehicle(vehicle, prefab, startTime, firstTryTime);
		if (first)
			m_aQueue.InsertAt(queued, 0);
		else
			m_aQueue.Insert(queued);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	//! \return its index in m_aQueue, -1 if it isn't queued
	protected int FindQueued(IEntity vehicle)
	{
		foreach (int index, CSI_QueuedVehicle queued : m_aQueue)
		{
			if (queued.m_Vehicle == vehicle)
				return index;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Lets another vehicle of the prefab be queued.
	//! \param[in] prefab
	protected void ReleasePrefab(ResourceName prefab)
	{
		int prefabIndex = m_aQueuedPrefabs.Find(prefab);
		if (prefabIndex >= 0)
			m_aQueuedPrefabs.Remove(prefabIndex);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 REQUESTS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Sends the vehicle's built outlines and queues the vehicle next for the rest.
	//! \param[in] vehicle the player's vehicle
	//! \param[in] requester the player's, receives the outlines
	void RequestOutlines(notnull IEntity vehicle, notnull CSI_RplToAuthorityManager requester)
	{
		array<ref CSI_VehicleOutlinePart> parts = {};
		CSI_VehicleOutline.GetOutlineParts(vehicle, parts);

		bool missing;
		foreach (CSI_VehicleOutlinePart part : parts)
		{
			// Only parts with a prefab are cached and sent
			if (part.m_sPrefab.IsEmpty())
				continue;

			if (part.m_Outline)
			{
				requester.SendVehicleOutline(part.m_sPrefab, part.m_Outline);
				continue;
			}

			array<CSI_RplToAuthorityManager> waiting = m_mWaitingRequesters.Get(part.m_sPrefab);
			if (!waiting)
			{
				waiting = {};
				m_mWaitingRequesters.Set(part.m_sPrefab, waiting);
			}

			// Players who left are null
			for (int index = waiting.Count() - 1; index >= 0; index--)
			{
				if (!waiting[index])
					waiting.Remove(index);
			}

			if (!waiting.Contains(requester))
				waiting.Insert(requester);

			missing = true;
		}

		if (!missing || vehicle == m_CurrentVehicle)
			return;

		// Next, ahead of the queue; an earlier try keeps its timeout
		float firstTryTime = -1;
		int queuedIndex = FindQueued(vehicle);
		if (queuedIndex >= 0)
		{
			firstTryTime = m_aQueue[queuedIndex].m_fFirstTryTime;
			m_aQueue.RemoveOrdered(queuedIndex);
		}

		ResourceName prefab = CSI_VehicleOutline.GetPrefabName(vehicle);
		QueueVehicle(vehicle, prefab, m_fTime, true, firstTryTime);
		if (!prefab.IsEmpty())
			m_aQueuedPrefabs.Insert(prefab);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] prefab a part's
	//! \param[in] outline
	protected void SendToWaiting(ResourceName prefab, notnull CSI_VehicleOutline outline)
	{
		array<CSI_RplToAuthorityManager> waiting = m_mWaitingRequesters.Get(prefab);
		if (!waiting)
			return;

		foreach (CSI_RplToAuthorityManager requester : waiting)
		{
			if (requester)
				requester.SendVehicleOutline(prefab, outline);
		}

		m_mWaitingRequesters.Remove(prefab);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \return the server's system, null on clients
	static CSI_VehicleOutlineSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return CSI_VehicleOutlineSystem.Cast(world.FindSystem(CSI_VehicleOutlineSystem));
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Coalition Squad Interface
//!	\Description: 	A vehicle waiting in CSI_VehicleOutlineSystem's build queue.
//!
//------------------------------------------------------------------------------------------------
class CSI_QueuedVehicle
{
	IEntity m_Vehicle;
	ResourceName m_sPrefab;			//!< the vehicle's
	float m_fStartTime;				//!< earliest start, in the system's time
	float m_fFirstTryTime;			//!< when its first try started, -1 if untried

	//------------------------------------------------------------------------------------------------
	void CSI_QueuedVehicle(IEntity vehicle, ResourceName prefab, float startTime, float firstTryTime)
	{
		m_Vehicle = vehicle;
		m_sPrefab = prefab;
		m_fStartTime = startTime;
		m_fFirstTryTime = firstTryTime;
	}
}
