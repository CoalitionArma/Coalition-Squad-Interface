class CSI_HUDManagerClass : ScriptComponentClass {};

class CSI_HUDManager : ScriptComponent
{	
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected SCR_ChimeraCharacter m_LocalPlayerCharacter;
	protected ref array<int> m_aLocalGroupPlayerIds = {};
	protected bool m_bIsLocalPlayerInVehicle;
	protected bool m_bIsLocalPlayerInMortar;
	protected int m_iLocalGroupCount;
	protected float m_iLocalYaw;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates the heads-up display values for the local client
	 */
	void UpdateLocalHUDValues()
	{
		m_LocalPlayerCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());
		
		if (!m_LocalPlayerCharacter) 
			return;
		
		BaseCompartmentSlot vehicleCompartment = CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_LocalPlayerCharacter);
		
		if (vehicleCompartment)
		{ 
			IEntity vehicle = vehicleCompartment.GetVehicle();
			if (vehicle)
			{
				m_bIsLocalPlayerInVehicle = true;
				
				if (SCR_MortarMuzzleComponent.Cast(vehicle.FindComponent(SCR_MortarMuzzleComponent)))
					m_bIsLocalPlayerInMortar = true;
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
	/**
	 * Updates the local player's aiming yaw angle.
	 */
	void UpdateLocalAimingYaw()
	{
		if (!m_LocalPlayerCharacter)
			return;
        float yaw;

		AimingComponent playerControllerComponent = m_LocalPlayerCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) 
			return;

		if (m_bIsLocalPlayerInVehicle || playerControllerComponent.GetAimingDirection().ToYaw() > 0)
		{
			vector transform[4];
			GetGame().GetWorld().GetCurrentCamera(transform);

			yaw = -Math3D.MatrixToAngles(transform)[0];
		} else
			yaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();

		m_iLocalYaw = yaw;
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Returns a sorted array of player IDs based on their group assignments
	 * @param playerIDs: Array of player IDs to sort
	 * @return Sorted array of player IDs
	 */
	array<int> GetSortedGroupArray(array<int> playerIDs)
	{
		array<int> playersGroupArray = {};
		array<string> tempLocalGroupArray = {};

		// Parse through current group array.
		foreach (int playerID : playerIDs)
		{
			CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
			
			if (!playerData)
				continue;
			
			int playerValue = 0;
	
			// Sort player by their color so we can group color teams together (a lil bit racist).
			switch (playerData.GetColorTeam()) 
			{
				case CSI_EColorTeam.RED    : playerValue = -3; break;
				case CSI_EColorTeam.BLUE   : playerValue = -5; break;
				case CSI_EColorTeam.YELLOW : playerValue = -7; break;
				case CSI_EColorTeam.GREEN  : playerValue = -9; break;
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

		foreach (string playerStr : tempLocalGroupArray) 
		{
			array<string> outPlayerStrArray = {};
			playerStr.Split(";", outPlayerStrArray, false);
			
			playersGroupArray.Insert(outPlayerStrArray[1].ToInt());
		}
		
		return playersGroupArray;
	};
	
	//------------------------------------------------------------------------------------------------
	SCR_ChimeraCharacter GetLocalPlayerCharacter()
	{
		return m_LocalPlayerCharacter;
	}
	
	//------------------------------------------------------------------------------------------------
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
	float GetLocalYaw()
	{
		return m_iLocalYaw;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the HUDManager
	protected static CSI_HUDManager m_sInstance;
	static CSI_HUDManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	void CSI_HUDManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}
