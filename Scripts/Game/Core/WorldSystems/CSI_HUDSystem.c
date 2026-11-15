//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
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
	protected ref array<int> m_aLocalGroupPlayerIds = {};	//!< new array each refresh; readers compare references
	protected bool m_bIsLocalPlayerInVehicle;
	protected bool m_bIsLocalPlayerInMortar;
	protected int m_iLocalGroupCount;
	protected float m_fLocalYaw;

	protected static const float REFRESH_INTERVAL = 0.5;	//!< seconds between vehicle and group refreshes

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

		m_PlayerController = null;
		super.OnStopped();
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
	//! \return IDs in display order, a new array each refresh
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

		// Every REFRESH_INTERVAL: vehicle state and group list
		m_fTimeSinceRefresh += args.GetTimeSliceSeconds();
		if (m_fTimeSinceRefresh < REFRESH_INTERVAL)
			return;

		m_fTimeSinceRefresh = 0;
		UpdateLocalVehicleState();
		UpdateLocalGroup();
	}

	//------------------------------------------------------------------------------------------------
	//! Switches character and forces a refresh on the next update.
	//! \param[in] from
	//! \param[in] to
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		m_LocalPlayerCharacter = SCR_ChimeraCharacter.Cast(to);
		m_fTimeSinceRefresh = REFRESH_INTERVAL;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLocalVehicleState()
	{
		// Cleared first so they don't stick after dying in a vehicle
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

		m_aLocalGroupPlayerIds = GetSortedGroupArray(playersGroup.GetPlayerIDs());
		m_iLocalGroupCount = m_aLocalGroupPlayerIds.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Camera yaw in vehicles or when aim yaw is positive, else head aim.
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
	//! Display order: SL, the SL's team, other teams, unassigned; each team's TL first.
	//! Players without CSI data are left out.
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
