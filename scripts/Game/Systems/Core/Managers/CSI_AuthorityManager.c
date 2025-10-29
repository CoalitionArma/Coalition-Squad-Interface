[ComponentEditorProps(category: "GameScripted/Authority", description: "")]
class CSI_AuthorityManagerClass : SCR_BaseGameModeComponentClass {};

class CSI_AuthorityManager : SCR_BaseGameModeComponent
{	
	protected bool m_bDataUpdateInProgress;
	protected static CSI_AuthorityManager m_sInstance;
	protected ref map<int, CSI_PlayerData> m_mPlayerDataMap = new map<int, CSI_PlayerData>;
	
	[RplProp()]
	protected ref array<int> m_aPlayerIDs = {}; 
	
	[RplProp()]
	protected ref array<ref CSI_PlayerData> m_aPlayerData = {}; 
	
	[RplProp(onRplName: "PlayerDataUpdate")]
	protected int m_PlayerDataUpdate;
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the AuthorityManager
	static CSI_AuthorityManager GetInstance()
	{
		return m_sInstance;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerData(int playerID, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (!playerData)
		{
			playerData = new CSI_PlayerData;
			m_mPlayerDataMap.Set(playerID, playerData);
		}
		
		playerData.SetDisplayIcon(icon);
		playerData.SetRank(rank);
		
		RequestDataUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		playerData.SetColorTeam(colorTeam);
		
		RequestDataUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		playerData.SetOverrideIcon(overrideIcon);
		
		RequestDataUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		playerData.SetIsTeamLeader(isTL);
		
		RequestDataUpdate();
	}

	//------------------------------------------------------------------------------------------------
	CSI_PlayerData GetPlayerData(int playerID)
	{
		return m_mPlayerDataMap.Get(playerID);
	}

	//------------------------------------------------------------------------------------------------
	protected void RequestDataUpdate()
	{
		if (!m_bDataUpdateInProgress)
		{
			GetGame().GetCallqueue().CallLater(DataUpdate, 250, false);
			m_bDataUpdateInProgress = true;
		};
	}

	//------------------------------------------------------------------------------------------------
	protected void DataUpdate()
	{
		m_bDataUpdateInProgress = false;
		
		m_aPlayerIDs.Clear();
		m_aPlayerData.Clear();

		// Fill arrays with all map data
		foreach (int playerID, ref CSI_PlayerData playerData : m_mPlayerDataMap)
		{
			m_aPlayerIDs.Insert(playerID);
			m_aPlayerData.Insert(playerData);
		}

		// Update replication properties
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
	void CSI_AuthorityManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}
