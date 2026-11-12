class CSI_RplToAuthorityManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Client to server bridge on each player controller (child of CSI_PlayerControllerManager).
//! 				Owner_* methods run on the owning client and send RpcAsk_* to the server, which hands off to
//! 				CSI_RplBroadcastManager, CSI_SettingsManager or vanilla group APIs.
//! 
//------------------------------------------------------------------------------------------------
class CSI_RplToAuthorityManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_RplBroadcastManager m_RplBroadcastManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_SettingsManager m_SettingsManager;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SYSTEM INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 CLIENT-SIDE METHODS - These send RPCs from client to authority (server)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Asks the server to create the player's data, unless this client already has it.
	//! \param[in] playerID
	void Owner_RegisterPlayerData(int playerID)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		if(playerData)
			return;

		Rpc(RpcAsk_RegisterPlayerData, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Asks the server to reset the player's group-specific data (color team, override icon, team leader).
	//! \param[in] playerID
	void Owner_ClearGroupSpecificData(int playerID)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		if(!playerData)
			return;

		Rpc(RpcAsk_ClearGroupSpecificData, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerDisplayIcon(int playerID, CSI_EIcon displayIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		// Only send when the value changed (called every update cycle)
		if(!playerData || (playerData && displayIcon == playerData.GetDisplayIcon()))
			return;

		Rpc(RpcAsk_UpdatePlayerDisplayIcon, playerID, displayIcon);
	}

	//-------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		// Only send when the value changed
		if(!playerData || (playerData && colorTeam == playerData.GetColorTeam()))
			return;

		Rpc(RpcAsk_UpdatePlayerColorTeam, playerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] overrideIcon AUTO removes the override
	void Owner_UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		// Only send when the value changed
		if(!playerData || (playerData && overrideIcon == playerData.GetOverrideIcon()))
			return;

		Rpc(RpcAsk_UpdatePlayerOverrideIcon, playerID, overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		// Only send when the value changed (called every update cycle)
		if(!playerData || (playerData && rank == playerData.GetRank()))
			return;

		Rpc(RpcAsk_UpdatePlayerRank, playerID, rank);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		// Only send when the value changed
		if(!playerData || (playerData && isTL == playerData.GetIsTeamLeader()))
			return;

		Rpc(RpcAsk_UpdatePlayerTeamLeader, playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		// Only send when the value changed (called every update cycle)
		if(!playerData || (playerData && isSL == playerData.GetIsSquadLeader()))
			return;

		Rpc(RpcAsk_UpdatePlayerSquadLeader, playerID, isSL);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID player to make leader of their group
	void Owner_PromotePlayerToSL(int playerID)
	{
		Rpc(RpcAsk_PromotePlayerToSL, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID leader of the group to resize
	//! \param[in] maxMembers
	void Owner_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		Rpc(RpcAsk_SetMaxGroupMembers, playerID, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID player to kick from the local player's group
	void Owner_RemovePlayerFromGroup(int playerID)
	{
		Rpc(RpcAsk_RemovePlayerFromGroup, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value
	//! \param[in] serverOverrideEnabled false clears the server override
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
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.RegisterPlayerData(playerID);
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
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerDisplayIcon(playerID, displayIcon);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerColorTeam(playerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerRank(playerID, rank);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerOverrideIcon(playerID, overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerSquadLeader(playerID, isSL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerTeamLeader(playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	//! Makes the player leader of their group, using the same call as vanilla's groups manager.
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerID)
	{
		if (playerID <= 0)
			return;

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);

		if (!playersGroup || playersGroup.IsPlayerLeader(playerID))
			return;

		playersGroup.SetGroupLeader(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the max size of the group led by playerID. No vanilla RPC does this.
	//! \param[in] playerID
	//! \param[in] maxMembers
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		if (playerID <= 0 || maxMembers <= 0)
			return;

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);

		if (!playersGroup || !playersGroup.IsPlayerLeader(playerID))
			return;

		// Never below the current member count
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
		if (!m_SettingsManager || !CSI_GameSettings.GetSettingsArray().Contains(setting))
			return;

		m_SettingsManager.UpdateAuthoritySetting(setting, value, serverOverrideEnabled);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected static CSI_RplToAuthorityManager m_sInstance;	//!< last constructed instance, only a fallback
	//! Every player controller has one of these components, so on a listen server the last constructed
	//! one isn't necessarily the host's. Resolves through the local player controller instead.
	//! \return the local player's instance
	static CSI_RplToAuthorityManager GetInstance()
	{
		PlayerController localController = GetGame().GetPlayerController();
		if (localController)
		{
			CSI_PlayerControllerManager controllerManager = CSI_PlayerControllerManager.Cast(localController.FindComponent(CSI_PlayerControllerManager));
			if (controllerManager)
			{
				CSI_RplToAuthorityManager localInstance = CSI_RplToAuthorityManager.Cast(controllerManager.FindComponent(CSI_RplToAuthorityManager));
				if (localInstance)
					return localInstance;
			}
		}

		return m_sInstance;
	}

    //------------------------------------------------------------------------------------------------
	void CSI_RplToAuthorityManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}