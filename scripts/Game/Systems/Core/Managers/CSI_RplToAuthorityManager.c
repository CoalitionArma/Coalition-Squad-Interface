[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_RplToAuthorityManagerClass : ScriptComponentClass {};

class CSI_RplToAuthorityManager : ScriptComponent
{	
    protected CSI_AuthorityManager m_AuthorityManager;
	protected static CSI_RplToAuthorityManager m_sInstance;
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the RplToAuthorityManager
	static CSI_RplToAuthorityManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		if(!Replication.IsServer())
			return;
		
		m_AuthorityManager = CSI_AuthorityManager.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	// CLIENT-SIDE METHODS - These send RPCs from client to server
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerData(int playerId, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		Rpc(RpcAsk_UpdatePlayerColorTeam, playerId, icon, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerColorTeam(int playerId, CSI_EColorTeam colorTeam)
	{
		Rpc(RpcAsk_UpdatePlayerColorTeam, playerId, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerOverrideIcon(int playerId, CSI_EOverrideIcon overrideIcon)
	{
		Rpc(RpcAsk_UpdatePlayerOverrideIcon, playerId, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerTeamLeader(int playerId, bool isTL)
	{
		Rpc(RpcAsk_UpdatePlayerTeamLeader, playerId, isTL);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_PromotePlayerToSL(int playerId)
	{
		Rpc(RpcAsk_PromotePlayerToSL, playerId);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_SetMaxGroupMembers(int playerId, int maxMembers)
	{
		Rpc(RpcAsk_SetMaxGroupMembers, playerId, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_RemovePlayerFromGroup(int playerId)
	{
		Rpc(RpcAsk_RemovePlayerFromGroup, playerId);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_ChangeAuthoritySetting(string setting, string value)
	{
		Rpc(RpcAsk_ChangeAuthoritySetting, setting, value);
	}

	//------------------------------------------------------------------------------------------------
	// SERVER-SIDE RPC HANDLERS - Executed on the authority (server)
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_UpdatePlayerData(int playerId, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = m_AuthorityManager.GetPlayerData(playerId);
		
		// Check if any data has updated
		if(playerData && (icon == playerData.GetDisplayIcon() && rank == playerData.GetRank()))
			return;
		
		m_AuthorityManager.UpdatePlayerData(playerId, icon, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerColorTeam(int playerId, CSI_EColorTeam colorTeam)
	{
		m_AuthorityManager.UpdatePlayerColorTeam(playerId, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerOverrideIcon(int playerId, CSI_EOverrideIcon overrideIcon)
	{
		m_AuthorityManager.UpdatePlayerOverrideIcon(playerId, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerTeamLeader(int playerId, bool isTL)
	{
		m_AuthorityManager.UpdatePlayerTeamLeader(playerId, isTL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerId)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerId);
		playersGroup.SetGroupLeader(playerId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerId, int maxMembers)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerId);

		if (maxMembers < playersGroup.GetPlayerCount()) 
			maxMembers = playersGroup.GetPlayerCount();

		playersGroup.SetMaxMembers(maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemovePlayerFromGroup(int playerId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerId);
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerId);

		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(group.GetFaction());

		if (!newGroup)
			return;
		playerGroupController.RequestJoinGroup(newGroup.GetGroupID());
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ChangeAuthoritySetting(string setting, string value)
	{
		//m_AuthorityManager.UpdateAuthoritySetting(setting, value);
	}

    //------------------------------------------------------------------------------------------------
	void CSI_RplToAuthorityManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}