class CSI_RplToAuthorityManagerClass : ScriptComponentClass {};

class CSI_RplToAuthorityManager : ScriptComponent
{	
    protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_SettingsManager m_SettingsManager;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	// CLIENT-SIDE METHODS - These send RPCs from client to server
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerData(int playerId, bool isSL, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerId);
		
		// Check if any data has updated
		if(playerData && (icon == playerData.GetDisplayIcon() && rank == playerData.GetRank() && isSL == playerData.GetIsSquadLeader()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerData, playerId, isSL, icon, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerColorTeam(int playerId, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerId);
		
		// Check if any data has updated
		if(playerData && colorTeam == playerData.GetColorTeam())
			return;
		
		Rpc(RpcAsk_UpdatePlayerColorTeam, playerId, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerOverrideIcon(int playerId, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerId);
		
		// Check if any data has updated
		if(playerData && overrideIcon == playerData.GetOverrideIcon())
			return;
		
		Rpc(RpcAsk_UpdatePlayerOverrideIcon, playerId, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerTeamLeader(int playerId, bool isTL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerId);
		
		// Check if any data has updated
		if(playerData && isTL == playerData.GetIsTeamLeader())
			return;
		
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
	void Owner_ChangeAuthoritySetting(string setting, int value, bool serverOverrideEnabled)
	{
		Rpc(RpcAsk_ChangeAuthoritySetting, setting, value, serverOverrideEnabled);
	}

	//------------------------------------------------------------------------------------------------
	// SERVER-SIDE RPC HANDLERS - Executed on the authority (server)
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_UpdatePlayerData(int playerId, bool isSL, CSI_EIcon icon, SCR_ECharacterRank rank)
	{	
		m_PlayerDataManager.UpdatePlayerData(playerId, isSL, icon, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerColorTeam(int playerId, CSI_EColorTeam colorTeam)
	{
		m_PlayerDataManager.UpdatePlayerColorTeam(playerId, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerOverrideIcon(int playerId, CSI_EOverrideIcon overrideIcon)
	{
		m_PlayerDataManager.UpdatePlayerOverrideIcon(playerId, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerTeamLeader(int playerId, bool isTL)
	{
		m_PlayerDataManager.UpdatePlayerTeamLeader(playerId, isTL);
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

		if (!playersGroup.IsPlayerLeader(playerId))
			return;
		
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
	protected void RpcAsk_ChangeAuthoritySetting(string setting, int value, bool serverOverrideEnabled)
	{
		m_SettingsManager.UpdateServerSetting(setting, value, serverOverrideEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the RplToAuthorityManager
	protected static CSI_RplToAuthorityManager m_sInstance;
	static CSI_RplToAuthorityManager GetInstance()
	{
		return m_sInstance;
	}

    //------------------------------------------------------------------------------------------------
	void CSI_RplToAuthorityManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}