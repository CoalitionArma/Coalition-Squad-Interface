class CSI_PlayerDataManagerClass : ScriptComponentClass {};

class CSI_PlayerDataManager : ScriptComponent
{	
	protected bool m_bDataUpdateInProgress;

	protected ref map<int, CSI_PlayerData> m_mPlayerDataMap = new map<int, CSI_PlayerData>;
	
	[RplProp()]
	protected ref array<int> m_aPlayerIDs = {}; 
	
	[RplProp()]
	protected ref array<ref CSI_PlayerData> m_aPlayerData = {}; 
	
	[RplProp(onRplName: "PlayerDataUpdate")]
	protected int m_PlayerDataUpdate;
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerData(int playerID, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (!playerData)
			playerData = CreatePlayerData(playerID);
		
		playerData.SetDisplayIcon(icon);
		playerData.SetRank(rank);
		
		DataUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetColorTeam(colorTeam);
		
		DataUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetOverrideIcon(overrideIcon);
		
		DataUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetIsTeamLeader(isTL);
		
		DataUpdate();
	}

	//------------------------------------------------------------------------------------------------
	CSI_PlayerData GetPlayerData(int playerID)
	{
		return m_mPlayerDataMap.Get(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	CSI_PlayerData CreatePlayerData(int playerID)
	{
		CSI_PlayerData playerData = new CSI_PlayerData;
		m_mPlayerDataMap.Set(playerID, playerData);
		return playerData;
	}

	//------------------------------------------------------------------------------------------------
	protected void DataUpdate()
	{
		array<int> tempPlayerIDs = {};
		array<ref CSI_PlayerData> tempPlayerData = {};

		// Fill arrays with all map data
		foreach (int playerID, ref CSI_PlayerData playerData : m_mPlayerDataMap)
		{
			tempPlayerIDs.Insert(playerID);
			tempPlayerData.Insert(playerData);
		}

		// Update replication properties
		m_aPlayerIDs = tempPlayerIDs;
		m_aPlayerData = tempPlayerData;
		m_PlayerDataUpdate++;
		Replication.BumpMe();
		
		#ifdef WORKBENCH
			PlayerDataUpdate();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlayerDataUpdate()
	{
		// Update local map from replicated arrays
		for (int i = 0; i < m_aPlayerIDs.Count(); i++)
		{
			int playerID = m_aPlayerIDs.Get(i);
			
			CSI_PlayerData newPlayerData = m_aPlayerData.Get(i);
			CSI_PlayerData oldPlayerData = m_mPlayerDataMap.Get(playerID);
			
			if(!oldPlayerData)
				m_mPlayerDataMap.Set(playerID, newPlayerData);
			else
				oldPlayerData.DataUpdate(playerID, newPlayerData);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the PlayerDataManager
	protected static CSI_PlayerDataManager m_sInstance;
	static CSI_PlayerDataManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	void CSI_PlayerDataManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}
