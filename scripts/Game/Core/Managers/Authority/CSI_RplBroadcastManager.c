class CSI_RplBroadcastManagerClass : ScriptComponentClass {}

class CSI_RplBroadcastManager : ScriptComponent
{
	protected CSI_PlayerDataManager m_PlayerDataManager;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
	}
	

	
	
	
	
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdateColorTeam(int playerId, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerId);
		if (playerData)
		{
			playerData.SetColorTeam(colorTeam);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the RplBroadcastManager
	protected static CSI_RplBroadcastManager m_sInstance;
	static CSI_RplBroadcastManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	void CSI_RplBroadcastManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}