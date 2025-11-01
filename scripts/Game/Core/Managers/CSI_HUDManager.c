class CSI_HUDManagerClass : ScriptComponentClass {};

class CSI_HUDManager : ScriptComponent
{	
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected SCR_ChimeraCharacter m_LocalPlayerCharacter;
	protected ref array<int> m_aLocalGroupPlayerIds;
	protected bool m_bIsLocalPlayerInVehicle;
	protected int m_iLocalGroupCount;
	protected float m_iLocalYaw;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateLocalHUDValues()
	{
		m_LocalPlayerCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());
		
		if (!m_LocalPlayerCharacter) 
			return;
		
		if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_LocalPlayerCharacter))
			m_bIsLocalPlayerInVehicle = true;
		else
			m_bIsLocalPlayerInVehicle = false;
		
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (playersGroup)
		{
			m_aLocalGroupPlayerIds = GetSortedGroupArray(playersGroup.GetPlayerIDs());
			m_iLocalGroupCount = m_aLocalGroupPlayerIds.Count();
		} else
			m_iLocalGroupCount = 0;
	}
	
	//------------------------------------------------------------------------------------------------
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
	protected array<int> GetSortedGroupArray(array<int> playerIds)
	{
		array<int> playersGroupArray = {};
		array<string> tempLocalGroupArray = {};

		// Parse through current group array.
		foreach (int playerID : playerIds)
		{
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(playerID);
			
			if (!playerData)
				continue;
			
			int playerValue = playerData.GetPlayerValue();
			
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
