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
	// CLIENT-SIDE METHODS - These send RPCs from client to authority (server)
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	/*!
	 * Updates player data on authority (host) with provided icon, squad leader status and rank
	 * @param playerID: ID of player to update
	 * @param isSL: whether player is squad leader
	 * @param icon: player's icon type
	 * @param rank: rank of player
	*/
	void Owner_UpdatePlayerData(int playerID, bool isSL, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(playerData && (icon == playerData.GetDisplayIcon() && rank == playerData.GetRank() && isSL == playerData.GetIsSquadLeader()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerData, playerID, isSL, icon, rank);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	 * Updates player data to clear any group-specific values
	 * @param playerID: ID of player to clear
	*/
	void Owner_ClearGroupSpecificData(int playerID)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if player has any data
		if(!playerData)
			return;
		
		Rpc(RpcAsk_ClearGroupSpecificData, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Update the specified player's team color on the authority.
	 * @param playerID: ID of player to update
	 * @param colorTeam: New color team to assign (CSI_EColorTeam)
	 */
	void Owner_UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(playerData && colorTeam == playerData.GetColorTeam())
			return;
		
		Rpc(RpcAsk_UpdatePlayerColorTeam, playerID, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates a player's icon override state on the authority.
	 * @param playerID: ID of player to update
	 * @param overrideIcon: The new override icon to apply
	 */
	void Owner_UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(playerData && overrideIcon == playerData.GetOverrideIcon())
			return;
		
		Rpc(RpcAsk_UpdatePlayerOverrideIcon, playerID, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates team leader status on the authority.
	 * @param playerID: ID of player to update
	 * @param isTL: Whether player should be team leader
	 */
	void Owner_UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(playerData && isTL == playerData.GetIsTeamLeader())
			return;
		
		Rpc(RpcAsk_UpdatePlayerTeamLeader, playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_PromotePlayerToSL(int playerID)
	{
		Rpc(RpcAsk_PromotePlayerToSL, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		Rpc(RpcAsk_SetMaxGroupMembers, playerID, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_RemovePlayerFromGroup(int playerID)
	{
		Rpc(RpcAsk_RemovePlayerFromGroup, playerID);
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
	void RpcAsk_UpdatePlayerData(int playerID, bool isSL, CSI_EIcon icon, SCR_ECharacterRank rank)
	{	
		m_PlayerDataManager.UpdatePlayerData(playerID, isSL, icon, rank);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ClearGroupSpecificData(int playerID)
	{	
		m_PlayerDataManager.ClearGroupSpecificData(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		m_PlayerDataManager.UpdatePlayerColorTeam(playerID, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		m_PlayerDataManager.UpdatePlayerOverrideIcon(playerID, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		m_PlayerDataManager.UpdatePlayerTeamLeader(playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);
		playersGroup.SetGroupLeader(playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);

		if (!playersGroup.IsPlayerLeader(playerID))
			return;
		
		if (maxMembers < playersGroup.GetPlayerCount()) 
			maxMembers = playersGroup.GetPlayerCount();

		playersGroup.SetMaxMembers(maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemovePlayerFromGroup(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerID);
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);

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