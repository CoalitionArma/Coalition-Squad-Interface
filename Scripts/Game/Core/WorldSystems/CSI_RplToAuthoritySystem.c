class CSI_RplToAuthoritySystem : GameSystem
{	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================
	
	protected CSI_RplBroadcastSystem m_RplBroadcastSystem;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_SettingsSystem m_SettingsSystem;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SYSTEM INITILIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false)
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{	
		super.OnInit();
		
		m_RplBroadcastSystem = CSI_RplBroadcastSystem.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_SettingsSystem = CSI_SettingsSystem.GetInstance();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 CLIENT-SIDE METHODS - These send RPCs from client to authority (server)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Registers player data on authority (host)
	//! \param[in] playerID: ID of player to update
	void Owner_RegisterPlayerData(int playerID)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data exists
		if(playerData)
			return;
		
		Rpc(RpcAsk_RegisterPlayerData, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates player data to clear any group-specific values
	//! \param[in] playerID: ID of player to clear
	void Owner_ClearGroupSpecificData(int playerID)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if player has any data
		if(!playerData)
			return;
		
		Rpc(RpcAsk_ClearGroupSpecificData, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update the specified player's team color on the authority.
	//! \param[in] playerID: ID of player to update
	//! \param[in] displayIcon: Icon to assign to player (CSI_EIcon)
	void Owner_UpdatePlayerDisplayIcon(int playerID, CSI_EIcon displayIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(!playerData || (playerData && displayIcon == playerData.GetDisplayIcon()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerDisplayIcon, playerID, displayIcon);
	}
	
	//-------------------------------------------------------------------------------------------------
	//! Update the specified player's team color on the authority.
	//! \param[in] playerID: ID of player to update
	//! \param[in] colorTeam: Color team to assign (CSI_EColorTeam)
	void Owner_UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(!playerData || (playerData && colorTeam == playerData.GetColorTeam()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerColorTeam, playerID, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates a player's icon override state on the authority.
	//! \param[in] playerID: ID of player to update
	//! \param[in] overrideIcon: The override icon to apply
	void Owner_UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(!playerData || (playerData && overrideIcon == playerData.GetOverrideIcon()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerOverrideIcon, playerID, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update the specified player's team color on the authority.
	//! \param[in] playerID: ID of player to update
	//! \param[in] rank: Rank to assign to player (SCR_ECharacterRank)
	void Owner_UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(!playerData || (playerData && rank == playerData.GetRank()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerRank, playerID, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates team leader status on the authority.
	//! \param[in] playerID: ID of player to update
	//! \param[in] isTL: Whether player should be team leader
	void Owner_UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(!playerData || (playerData && isTL == playerData.GetIsTeamLeader()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerTeamLeader, playerID, isTL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update the specified player's team color on the authority.
	//! \param[in] playerID: ID of player to update
	//! \param[in] isSL: New color team to assign (CSI_EColorTeam)
	void Owner_UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		
		// Check if any data has updated
		if(!playerData || (playerData && isSL == playerData.GetIsSquadLeader()))
			return;
		
		Rpc(RpcAsk_UpdatePlayerSquadLeader, playerID, isSL);
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

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SERVER-SIDE RPC HANDLERS - Executed on the authority (server)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RegisterPlayerData(int playerID)
	{	
		if (playerID <= 0 || !m_RplBroadcastSystem)
			return;
		
		m_RplBroadcastSystem.RegisterPlayerData(playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ClearGroupSpecificData(int playerID)
	{	
		if (playerID <= 0 || !m_PlayerDataManager)
			return;
		
		m_PlayerDataManager.ClearGroupSpecificData(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_UpdatePlayerDisplayIcon(int playerID, CSI_EIcon displayIcon)
	{
		if (playerID <= 0 || !m_RplBroadcastSystem)
			return;
		
		m_RplBroadcastSystem.UpdatePlayerDisplayIcon(playerID, displayIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		if (playerID <= 0 || !m_RplBroadcastSystem)
			return;
		
		m_RplBroadcastSystem.UpdatePlayerColorTeam(playerID, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		if (playerID <= 0 || !m_RplBroadcastSystem)
			return;
		
		m_RplBroadcastSystem.UpdatePlayerRank(playerID, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		if (playerID <= 0 || !m_RplBroadcastSystem)
			return;
		
		m_RplBroadcastSystem.UpdatePlayerOverrideIcon(playerID, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		if (playerID <= 0 || !m_RplBroadcastSystem)
			return;
		
		m_RplBroadcastSystem.UpdatePlayerSquadLeader(playerID, isSL);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		if (playerID <= 0 || !m_RplBroadcastSystem)
			return;
		
		m_RplBroadcastSystem.UpdatePlayerTeamLeader(playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerID)
	{
		if (playerID <= 0)
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);
		playersGroup.SetGroupLeader(playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		if (playerID <= 0 || maxMembers <= 0)
			return;
		
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
		if (playerID <= 0)
			return;
		
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
		if (!m_SettingsSystem || !CSI_GameSettings.GetSettingsArray().Contains(setting))
			return;
		
		m_SettingsSystem.UpdateAuthoritySetting(setting, value, serverOverrideEnabled);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	static CSI_RplToAuthoritySystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;
		return CSI_RplToAuthoritySystem.Cast(world.FindSystem(CSI_RplToAuthoritySystem));
	}
}