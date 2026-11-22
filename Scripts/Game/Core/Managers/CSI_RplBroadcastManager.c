class CSI_RplBroadcastManagerClass : ScriptComponentClass {}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-22
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Server to all clients: applies player data changes on the server, then broadcasts them with RpcDo_*.
//! 				A value the server already has isn't broadcast again, as a client asks until its change comes back.
//! 
//------------------------------------------------------------------------------------------------
class CSI_RplBroadcastManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_PlayerDataManager m_PlayerDataManager;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SYSTEM INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SERVER-SIDE METHODS - Apply on the server, then broadcast to all clients
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	void RegisterPlayerData(PlayerId playerID)
	{
		// Broadcast RPCs don't run on the sender, so apply on the server first
		RpcDo_RegisterPlayerData(playerID);
		Rpc(RpcDo_RegisterPlayerData, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void RemovePlayerData(PlayerId playerID)
	{
		RpcDo_RemovePlayerData(playerID);
		Rpc(RpcDo_RemovePlayerData, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerDisplayIcon(PlayerId playerID, CSI_EIcon icon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (playerData && playerData.GetDisplayIcon() == icon)
			return;

		RpcDo_UpdatePlayerDisplayIcon(playerID, icon);
		Rpc(RpcDo_UpdatePlayerDisplayIcon, playerID, icon);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerColorTeam(PlayerId playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (playerData && playerData.GetColorTeam() == colorTeam)
			return;

		RpcDo_UpdatePlayerColorTeam(playerID, colorTeam);
		Rpc(RpcDo_UpdatePlayerColorTeam, playerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerOverrideIcon(PlayerId playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (playerData && playerData.GetOverrideIcon() == overrideIcon)
			return;

		RpcDo_UpdatePlayerOverrideIcon(playerID, overrideIcon);
		Rpc(RpcDo_UpdatePlayerOverrideIcon, playerID, overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerRank(PlayerId playerID, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (playerData && playerData.GetRank() == rank)
			return;

		RpcDo_UpdatePlayerRank(playerID, rank);
		Rpc(RpcDo_UpdatePlayerRank, playerID, rank);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerTeamLeader(PlayerId playerID, bool isTL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (playerData && playerData.GetIsTeamLeader() == isTL)
			return;

		RpcDo_UpdatePlayerTeamLeader(playerID, isTL);
		Rpc(RpcDo_UpdatePlayerTeamLeader, playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerSquadLeader(PlayerId playerID, bool isSL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (playerData && playerData.GetIsSquadLeader() == isSL)
			return;

		RpcDo_UpdatePlayerSquadLeader(playerID, isSL);
		Rpc(RpcDo_UpdatePlayerSquadLeader, playerID, isSL);
	}

	//------------------------------------------------------------------------------------------------
	protected CSI_PlayerData GetPlayerData(PlayerId playerID)
	{
		if (!m_PlayerDataManager)
			return null;

		return m_PlayerDataManager.GetPlayerData(playerID);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 CLIENT-SIDE RPC HANDLERS - Executed on every client (and called directly on the server)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RegisterPlayerData(PlayerId playerID)
	{
		m_PlayerDataManager.RegisterPlayerData(playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RemovePlayerData(PlayerId playerID)
	{
		m_PlayerDataManager.RemovePlayerData(playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerDisplayIcon(PlayerId playerID, CSI_EIcon icon)
	{
		m_PlayerDataManager.UpdatePlayerDisplayIcon(playerID, icon);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerColorTeam(PlayerId playerID, CSI_EColorTeam colorTeam)
	{
		m_PlayerDataManager.UpdatePlayerColorTeam(playerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerOverrideIcon(PlayerId playerID, CSI_EOverrideIcon overrideIcon)
	{
		m_PlayerDataManager.UpdatePlayerOverrideIcon(playerID, overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerRank(PlayerId playerID, SCR_ECharacterRank rank)
	{
		m_PlayerDataManager.UpdatePlayerRank(playerID, rank);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerTeamLeader(PlayerId playerID, bool isTL)
	{
		m_PlayerDataManager.UpdatePlayerTeamLeader(playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePlayerSquadLeader(PlayerId playerID, bool isSL)
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