class CSI_RplBroadcastManagerClass : ScriptComponentClass {}

class CSI_RplBroadcastManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================
	
	protected CSI_PlayerDataManager m_PlayerDataManager;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SYSTEM INITILIZATION
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SERVER-SIDE RPC HANDLERS - Executed on the authority (server)
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	void RegisterPlayerData(int playerID)
	{
		RpcDo_RegisterPlayerData(playerID);
		Rpc(RpcDo_RegisterPlayerData, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemovePlayerData(int playerID)
	{
		RpcDo_RemovePlayerData(playerID);
		Rpc(RpcDo_RemovePlayerData, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerDisplayIcon(int playerID, CSI_EIcon icon)
	{
		RpcDo_UpdatePlayerDisplayIcon(playerID, icon);
		Rpc(RpcDo_UpdatePlayerDisplayIcon, playerID, icon);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		RpcDo_UpdatePlayerColorTeam(playerID, colorTeam);
		Rpc(RpcDo_UpdatePlayerColorTeam, playerID, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		RpcDo_UpdatePlayerOverrideIcon(playerID, overrideIcon);
		Rpc(RpcDo_UpdatePlayerOverrideIcon, playerID, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		RpcDo_UpdatePlayerRank(playerID, rank);
		Rpc(RpcDo_UpdatePlayerRank, playerID, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		RpcDo_UpdatePlayerTeamLeader(playerID, isTL);
		Rpc(RpcDo_UpdatePlayerTeamLeader, playerID, isTL);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		RpcDo_UpdatePlayerSquadLeader(playerID, isSL);
		Rpc(RpcDo_UpdatePlayerSquadLeader, playerID, isSL);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 CLIENT-SIDE METHODS - These send RPCs from authority (server) to all clients
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RegisterPlayerData(int playerID)
	{
		m_PlayerDataManager.RegisterPlayerData(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RemovePlayerData(int playerID)
	{
		m_PlayerDataManager.RemovePlayerData(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerDisplayIcon(int playerID, CSI_EIcon icon)
	{
		m_PlayerDataManager.UpdatePlayerDisplayIcon(playerID, icon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		m_PlayerDataManager.UpdatePlayerColorTeam(playerID, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		m_PlayerDataManager.UpdatePlayerOverrideIcon(playerID, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		m_PlayerDataManager.UpdatePlayerRank(playerID, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		m_PlayerDataManager.UpdatePlayerTeamLeader(playerID, isTL);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		m_PlayerDataManager.UpdatePlayerSquadLeader(playerID, isSL);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
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