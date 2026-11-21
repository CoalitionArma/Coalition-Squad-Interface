//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-20
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Client world system feeding the HUD and nametags: the locally controlled character, yaw,
//! 				vehicle/mortar state and the sorted group member list.
//! 
//------------------------------------------------------------------------------------------------
class CSI_HUDSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected SCR_PlayerController m_PlayerController;		//!< local, listened to for controlled entity changes
	protected SCR_ChimeraCharacter m_LocalPlayerCharacter;	//!< locally controlled character
	protected SCR_CompartmentAccessComponent m_LocalCompartmentAccess;	//!< local character's, listened to for vehicle and mortar state
	protected ref array<int> m_aLocalGroupPlayerIds = {};	//!< replaced when the group or its order changes; readers compare references
	protected bool m_bIsLocalPlayerInVehicle;
	protected bool m_bIsLocalPlayerInMortar;
	protected IEntity m_LocalVehicle;	//!< topmost entity with seats, null on foot
	protected int m_iLocalGroupCount;
	protected float m_fLocalYaw;

	protected static const float REFRESH_INTERVAL = 0.5;	//!< seconds between group refreshes
	protected static const int SORT_KEY_STEP = 1000;		//!< sort key multiplier, above any group's member count (GetSortedGroupArray)

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INIT METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.Frame);
	}

	//------------------------------------------------------------------------------------------------
	//! Should be paused when simulation is paused
	//! \return
	override bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnStopped()
	{
		if (m_PlayerController)
			m_PlayerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);

		if (m_LocalCompartmentAccess)
		{
			m_LocalCompartmentAccess.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
			m_LocalCompartmentAccess.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
		}

		m_PlayerController = null;
		m_LocalCompartmentAccess = null;
		super.OnStopped();
	}

	//------------------------------------------------------------------------------------------------
	void ~CSI_HUDSystem()
	{
		// Clear cached radar vehicle outlines here, not in OnStopped, which also runs on pause
		CSI_VehicleOutline.ClearCache();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	SCR_ChimeraCharacter GetLocalPlayerCharacter()
	{
		return m_LocalPlayerCharacter;
	}

	//------------------------------------------------------------------------------------------------
	//! \return IDs in display order; replaced by a new array, never changed, when the group or its order changes
	array<int> GetLocalGroupPlayerIds()
	{
		return m_aLocalGroupPlayerIds;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsLocalPlayerInVehicle()
	{
		return m_bIsLocalPlayerInVehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return topmost entity of the local character's vehicle, null on foot
	IEntity GetLocalVehicle()
	{
		return m_LocalVehicle;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsLocalPlayerInMortar()
	{
		return m_bIsLocalPlayerInMortar;
	}

	//------------------------------------------------------------------------------------------------
	int GetLocalGroupCount()
	{
		return m_iLocalGroupCount;
	}

	//------------------------------------------------------------------------------------------------
	//! \return negated yaw for the compass and radar
	float GetLocalYaw()
	{
		return m_fLocalYaw;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected float m_fTimeSinceRefresh;	//!< seconds since the last refresh
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_GroupsManagerComponent || !m_PlayerDataManager)
		{
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
			return;
		}

		// Follow controlled entity changes and catch up with the current one
		if (!m_PlayerController)
		{
			m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (!m_PlayerController)
				return;

			m_PlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
			OnControlledEntityChanged(null, m_PlayerController.GetControlledEntity());
		}

		// Every frame: yaw
		UpdateLocalAimingYaw();

		// Every REFRESH_INTERVAL: group list
		m_fTimeSinceRefresh += args.GetTimeSliceSeconds();
		if (m_fTimeSinceRefresh < REFRESH_INTERVAL)
			return;

		m_fTimeSinceRefresh = 0;
		UpdateLocalGroup();
	}

	//------------------------------------------------------------------------------------------------
	//! Switches character, moves the compartment listeners to it and forces a group refresh on the next update.
	//! \param[in] from
	//! \param[in] to
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (m_LocalCompartmentAccess)
		{
			m_LocalCompartmentAccess.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
			m_LocalCompartmentAccess.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
		}

		m_LocalPlayerCharacter = SCR_ChimeraCharacter.Cast(to);

		m_LocalCompartmentAccess = null;
		if (m_LocalPlayerCharacter)
			m_LocalCompartmentAccess = SCR_CompartmentAccessComponent.Cast(m_LocalPlayerCharacter.GetCompartmentAccessComponent());

		// Catch up with the seat the character is already in
		BaseCompartmentSlot compartment;
		if (m_LocalCompartmentAccess)
		{
			m_LocalCompartmentAccess.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
			m_LocalCompartmentAccess.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			compartment = m_LocalCompartmentAccess.GetCompartment();
		}

		UpdateLocalVehicleState(compartment);
		m_fTimeSinceRefresh = REFRESH_INTERVAL;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (manager)
			UpdateLocalVehicleState(manager.FindCompartment(slotID, mgrID));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		// Seat switches also enter the new seat
		if (move)
			return;

		UpdateLocalVehicleState(null);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartment the local character's seat, null if it isn't in one
	protected void UpdateLocalVehicleState(BaseCompartmentSlot compartment)
	{
		m_bIsLocalPlayerInVehicle = false;
		m_bIsLocalPlayerInMortar = false;
		m_LocalVehicle = null;

		if (!compartment)
			return;

		IEntity vehicle = compartment.GetVehicle();
		if (!vehicle)
			return;

		m_bIsLocalPlayerInVehicle = true;
		m_LocalVehicle = vehicle;
		m_bIsLocalPlayerInMortar = SCR_MortarMuzzleComponent.Cast(vehicle.FindComponent(SCR_MortarMuzzleComponent)) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Also runs without a character, as the group belongs to the player.
	protected void UpdateLocalGroup()
	{
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playersGroup)
		{
			// Replace rather than clear, so readers comparing references notice
			if (!m_aLocalGroupPlayerIds.IsEmpty())
				m_aLocalGroupPlayerIds = {};

			m_iLocalGroupCount = 0;
			return;
		}

		// Replaced only on a change, so readers comparing references have nothing to do meanwhile
		array<int> sortedPlayerIds = GetSortedGroupArray(playersGroup.GetPlayerIDs());
		if (!SCR_ArrayHelperT<int>.AreEqual(sortedPlayerIds, m_aLocalGroupPlayerIds))
			m_aLocalGroupPlayerIds = sortedPlayerIds;

		m_iLocalGroupCount = m_aLocalGroupPlayerIds.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Stores the current camera's yaw, negated.
	void UpdateLocalAimingYaw()
	{
		vector transform[4];
		GetGame().GetWorld().GetCurrentCamera(transform);

		m_fLocalYaw = -Math3D.MatrixToAngles(transform)[0];
	}

	//------------------------------------------------------------------------------------------------
	//! Display order: SL, the SL's team, other teams, unassigned; each team's TL first.
	//! Players without CSI data are left out.
	//! \param[in] playerIDs
	//! \return sorted player IDs
	array<int> GetSortedGroupArray(array<int> playerIDs)
	{
		if (!m_PlayerDataManager)
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();

		CSI_EColorTeam currentSLColorTeam = CSI_EColorTeam.NONE;

		foreach (PlayerId playerID : playerIDs)
		{
			CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

			if (!playerData)
				continue;

			if (playerData.GetIsSquadLeader())
				currentSLColorTeam = playerData.GetColorTeam();
		};

		// Team order, with the SL's own team moved to the front
		array<CSI_EColorTeam> playersCTArray = {CSI_EColorTeam.RED, CSI_EColorTeam.BLUE, CSI_EColorTeam.YELLOW, CSI_EColorTeam.GREEN, CSI_EColorTeam.NONE};
		playersCTArray.RemoveItemOrdered(currentSLColorTeam);
		playersCTArray.InsertAt(currentSLColorTeam, 0);

		// One number per member, sorted by the engine: the sort key, then their place in the group, so members with
		// the same key keep the group's order
		array<int> sortedKeys = {};
		foreach (int groupIndex, PlayerId playerID : playerIDs)
		{
			CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

			if (!playerData)
				continue;

			// Sort key: SL 1, then two per team in playersCTArray order, TL first
			int sortKey = 1;
			if (!playerData.GetIsSquadLeader())
			{
				int teamIndex = playersCTArray.Find(playerData.GetColorTeam());
				if (teamIndex < 0)
					teamIndex = playersCTArray.Count() - 1;

				sortKey = teamIndex * 2 + 3;
				if (playerData.GetIsTeamLeader())
					sortKey--;
			}

			sortedKeys.Insert(sortKey * SORT_KEY_STEP + groupIndex);
		};

		sortedKeys.Sort();

		array<int> playersGroupArray = {};
		foreach (int sortedKey : sortedKeys)
		{
			// The place in the group is what's left of the key
			playersGroupArray.Insert(playerIDs[sortedKey - (sortedKey / SORT_KEY_STEP) * SORT_KEY_STEP]);
		}

		return playersGroupArray;
	};

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	static CSI_HUDSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;
		return CSI_HUDSystem.Cast(world.FindSystem(CSI_HUDSystem));
	}
}
