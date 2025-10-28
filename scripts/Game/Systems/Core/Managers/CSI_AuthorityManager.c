[ComponentEditorProps(category: "GameScripted/Authority", description: "")]
class CSI_AuthorityManagerClass : SCR_BaseGameModeComponentClass {};

class CSI_AuthorityManager : SCR_BaseGameModeComponent
{	
	protected ref map<int, CSI_PlayerData> m_mPlayerDataMap = new map<int, CSI_PlayerData>;
	
	[RplProp()]
	protected ref array<int> m_aPlayerIDs = {}; 
	
	[RplProp()]
	protected ref array<ref CSI_PlayerData> m_aPlayerData = {}; 
	
	[RplProp(onRplName: "PlayerDataUpdate")]
	protected int m_PlayerDataUpdate;

	//------------------------------------------------------------------------------------------------
	protected static CSI_AuthorityManager m_sInstance;
	void CSI_AuthorityManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	// INITIALIZATION
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static CSI_AuthorityManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		//--- Server only
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		UpdateAuthoritySettingArray();
	}
	
	//------------------------------------------------------------------------------------------------
	// DATA AND REPLICATION HANDLING
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	CSI_PlayerData GetPlayerData(int playerID)
	{
		return m_mPlayerDataMap.Get(playerID);
	}

	//------------------------------------------------------------------------------------------------
	void RequestDataUpdate()
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
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
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
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

	// Functions to change/get Server Override Settings

	//------------------------------------------------------------------------------------------------
}
