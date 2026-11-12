//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Client world system that feeds the HUD and nametags: the locally controlled character (switched as soon as
//! 				it changes, like vanilla SCR_InfoDisplayExtended), yaw, vehicle/mortar state and the sorted group member
//! 				list (every REFRESH_INTERVAL).
//! 
//------------------------------------------------------------------------------------------------
class CSI_HUDSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected SCR_PlayerController m_PlayerController;		//!< local player controller, listened to for controlled entity changes
	protected SCR_ChimeraCharacter m_LocalPlayerCharacter;	//!< locally controlled character
	protected ref array<int> m_aLocalGroupPlayerIds = {};	//!< replaced (not modified) on every refresh, so readers can compare references
	protected bool m_bIsLocalPlayerInVehicle;
	protected bool m_bIsLocalPlayerInMortar;
	protected int m_iLocalGroupCount;
	protected float m_fLocalYaw;

	protected static const float REFRESH_INTERVAL = 0.5;	//!< seconds between vehicle state and group list refreshes

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
	//! The controlled entity listener is added from OnUpdatePoint, which only runs between OnStarted and OnStopped.
	override protected void OnStopped()
	{
		if (m_PlayerController)
			m_PlayerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);

		m_PlayerController = null;
		super.OnStopped();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \return the locally controlled character, null while the local player has none
	SCR_ChimeraCharacter GetLocalPlayerCharacter()
	{
		return m_LocalPlayerCharacter;
	}

	//------------------------------------------------------------------------------------------------
	//! \return local group's player IDs in display order (see GetSortedGroupArray); a new array on every refresh, empty without a group
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
	//! \return negated yaw the compass and radar face, updated every frame
	float GetLocalYaw()
	{
		return m_fLocalYaw;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected float m_fTimeSinceRefresh;	//!< seconds since the last vehicle state and group list refresh
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_GroupsManagerComponent || !m_PlayerDataManager)
		{
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
			return;
		}

		// The local player controller may not exist yet. Like vanilla SCR_InfoDisplayExtended, listen for controlled entity
		// changes (spawn, respawn, possession) and catch up with the entity controlled right now.
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

		// Every REFRESH_INTERVAL: vehicle state and group list. Time based like vanilla systems (SCR_AIEnemyMarkingSystem),
		// so the rate doesn't depend on the frame rate.
		m_fTimeSinceRefresh += args.GetTimeSliceSeconds();
		if (m_fTimeSinceRefresh < REFRESH_INTERVAL)
			return;

		m_fTimeSinceRefresh = 0;
		UpdateLocalVehicleState();
		UpdateLocalGroup();
	}

	//------------------------------------------------------------------------------------------------
	//! Switches to the newly controlled character and has the next update refresh the vehicle state and group list,
	//! rather than keeping the old character's for up to REFRESH_INTERVAL.
	//! \param[in] from previously controlled entity
	//! \param[in] to newly controlled entity, null while the player has none
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		m_LocalPlayerCharacter = SCR_ChimeraCharacter.Cast(to);
		m_fTimeSinceRefresh = REFRESH_INTERVAL;
	}

	//------------------------------------------------------------------------------------------------
	//! Works out whether the local character is in a vehicle, and whether that vehicle is a mortar.
	protected void UpdateLocalVehicleState()
	{
		// Cleared first, so neither sticks from a character that died in a vehicle
		m_bIsLocalPlayerInVehicle = false;
		m_bIsLocalPlayerInMortar = false;

		if (!m_LocalPlayerCharacter)
			return;

		BaseCompartmentSlot vehicleCompartment = CSI_EntityHelper.GetCharacterVehicleCompartment(m_LocalPlayerCharacter);
		if (!vehicleCompartment)
			return;

		IEntity vehicle = vehicleCompartment.GetVehicle();
		if (!vehicle)
			return;

		m_bIsLocalPlayerInVehicle = true;
		m_bIsLocalPlayerInMortar = SCR_MortarMuzzleComponent.Cast(vehicle.FindComponent(SCR_MortarMuzzleComponent)) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Refreshes the sorted group member list. The group belongs to the player, not the character, so this also runs
	//! while the player has no character.
	protected void UpdateLocalGroup()
	{
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playersGroup)
		{
			// A new empty array, so readers comparing references notice the group is gone
			if (!m_aLocalGroupPlayerIds.IsEmpty())
				m_aLocalGroupPlayerIds = {};

			m_iLocalGroupCount = 0;
			return;
		}

		m_aLocalGroupPlayerIds = GetSortedGroupArray(playersGroup.GetPlayerIDs());
		m_iLocalGroupCount = m_aLocalGroupPlayerIds.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Caches the local yaw for the compass and radar: camera yaw in vehicles (or when aim yaw is positive), else head aim.
	void UpdateLocalAimingYaw()
	{
		if (!m_LocalPlayerCharacter)
			return;
		float yaw;

		AimingComponent playerAimingComponent = m_LocalPlayerCharacter.GetHeadAimingComponent();
		if (!playerAimingComponent)
			return;

		if (m_bIsLocalPlayerInVehicle || playerAimingComponent.GetAimingDirection().ToYaw() > 0)
		{
			vector transform[4];
			GetGame().GetWorld().GetCurrentCamera(transform);

			yaw = -Math3D.MatrixToAngles(transform)[0];
		} else
			yaw = -playerAimingComponent.GetAimingDirectionWorld().ToYaw();

		m_fLocalYaw = yaw;
	}

	//------------------------------------------------------------------------------------------------
	//! Sorts group members for display: squad leader, then the SL's color team, the other teams, and unassigned
	//! players last; each team's TL first. Players without CSI data are left out.
	//! \param[in] playerIDs
	//! \return sorted player IDs
	array<int> GetSortedGroupArray(array<int> playerIDs)
	{
		if (!m_PlayerDataManager)
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();

		SCR_SortedArray<int> sortedPlayerIDs = new SCR_SortedArray<int>();	// lowest key first; equal keys keep group order
		CSI_EColorTeam currentSLColorTeam = CSI_EColorTeam.NONE;

		foreach (int playerID : playerIDs)
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

		foreach (int playerID : playerIDs)
		{
			CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

			if (!playerData)
				continue;

			// Sort key: SL 1, then two keys per team in playersCTArray order, TL before members (2-3, 4-5, ...)
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

			sortedPlayerIDs.Insert(sortKey, playerID);
		};

		array<int> playersGroupArray = {};
		sortedPlayerIDs.ToArray(playersGroupArray);
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
