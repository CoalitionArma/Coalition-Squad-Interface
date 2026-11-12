//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Client world system that feeds the HUD and nametags: local character, yaw, vehicle/mortar state and the
//! 				sorted group member list (every 60 frames).
//! 
//------------------------------------------------------------------------------------------------
class CSI_HUDSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected SCR_ChimeraCharacter m_LocalPlayerCharacter;
	protected ref array<int> m_aLocalGroupPlayerIds = {};	//!< replaced (not modified) when the group changes, so readers can compare references
	protected bool m_bIsLocalPlayerInVehicle;
	protected bool m_bIsLocalPlayerInMortar;
	protected int m_iLocalGroupCount;
	protected float m_fLocalYaw;

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

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \return null while the local player has no character
	SCR_ChimeraCharacter GetLocalPlayerCharacter()
	{
		return m_LocalPlayerCharacter;
	}

	//------------------------------------------------------------------------------------------------
	//! \return local group's player IDs in display order (see GetSortedGroupArray); a new array when the group changes
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

	protected int m_iUpdate;	//!< frames since the last group/vehicle refresh
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_GroupsManagerComponent || !m_PlayerDataManager)
		{
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
			return;
		}

		// Every frame: yaw
		m_iUpdate++;
		UpdateLocalAimingYaw();

		// Every 60 frames: character, vehicle state and group list
		if (!(m_iUpdate >= 60))
			return;
		else
			m_iUpdate = 0;

		if (!m_GroupsManagerComponent)
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		m_LocalPlayerCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());

		if (!m_LocalPlayerCharacter)
			return;

		BaseCompartmentSlot vehicleCompartment = CSI_EntityHelper.GetCharacterVehicleCompartment(m_LocalPlayerCharacter);

		if (vehicleCompartment)
		{
			IEntity vehicle = vehicleCompartment.GetVehicle();
			if (vehicle)
			{
				m_bIsLocalPlayerInVehicle = true;

				// Assigned (not just set true) so moving straight from a mortar into another vehicle clears it
				m_bIsLocalPlayerInMortar = SCR_MortarMuzzleComponent.Cast(vehicle.FindComponent(SCR_MortarMuzzleComponent)) != null;
			};
		} else {
			m_bIsLocalPlayerInVehicle = false;
			m_bIsLocalPlayerInMortar = false;
		};

		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		if (playersGroup)
		{
			m_aLocalGroupPlayerIds = GetSortedGroupArray(playersGroup.GetPlayerIDs());
			m_iLocalGroupCount = m_aLocalGroupPlayerIds.Count();
		} else
			m_iLocalGroupCount = 0;
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

		array<int> playersGroupArray = {};
		array<string> tempLocalGroupArray = {};
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

			int playerValue = 0;

			// Sort player by their color so we can group color teams together (a lil bit racist).
			switch (playerData.GetColorTeam())
			{
				case playersCTArray[0]	: playerValue = -3; break;
				case playersCTArray[1]	: playerValue = -5; break;
				case playersCTArray[2]	: playerValue = -7; break;
				case playersCTArray[3]	: playerValue = -9; break;
				case playersCTArray[4]	: playerValue = 2; break;
				default : {playerValue = 2;  break;};
			};

			switch (true)
			{
				case (playerData.GetIsSquadLeader()) : playerValue = -1; break;
				case (playerData.GetIsTeamLeader() && playerData.GetColorTeam() == CSI_EColorTeam.NONE) : playerValue--; break;
				case (playerData.GetIsTeamLeader() && playerData.GetColorTeam() != CSI_EColorTeam.NONE) : playerValue++; break;
			};

			// Format a string with what we need for displaying/sorting a player.
			string playerStr = string.Format("%1;%2", playerValue, playerID);

			tempLocalGroupArray.Insert(playerStr);
		};

		tempLocalGroupArray.Sort(false);

		array<string> outPlayerStrArray = {};
		foreach (string playerStr : tempLocalGroupArray)
		{
			outPlayerStrArray.Clear();
			playerStr.Split(";", outPlayerStrArray, false);

			playersGroupArray.Insert(outPlayerStrArray[1].ToInt());
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
