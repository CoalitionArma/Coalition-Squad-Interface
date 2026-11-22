class CSI_RplToAuthorityManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-22
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Client to server bridge on each player controller: Owner_* sends RpcAsk_* to the server,
//! 				which hands off to CSI_RplBroadcastManager, CSI_SettingsManager, CSI_VehicleOutlineSystem or the
//! 				group APIs; the server sends vehicle outlines back to the owner (RpcDo_VehicleOutline).
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

	// Owner: values asked for and not yet echoed by the server's broadcast, -1 for none
	protected int m_iAskedDisplayIcon = -1;
	protected int m_iAskedRank = -1;
	protected int m_iAskedSquadLeader = -1;

	protected ref set<ResourceName> m_aSentOutlinePrefabs = new set<ResourceName>();	//!< server only: outlines sent to this player

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
	void Owner_UpdatePlayerDisplayIcon(PlayerId playerID, CSI_EIcon displayIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		if (!playerData)
			return;

		// Only send when the value changed (called every update cycle), once until the server's broadcast echoes it
		if (playerData.GetDisplayIcon() == m_iAskedDisplayIcon)
			m_iAskedDisplayIcon = -1;

		if (displayIcon == playerData.GetDisplayIcon() || displayIcon == m_iAskedDisplayIcon)
			return;

		m_iAskedDisplayIcon = displayIcon;
		Rpc(RpcAsk_UpdatePlayerDisplayIcon, playerID, displayIcon);
	}

	//-------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerColorTeam(PlayerId playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		if(!playerData || (playerData && colorTeam == playerData.GetColorTeam()))
			return;

		Rpc(RpcAsk_UpdatePlayerColorTeam, playerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] overrideIcon AUTO removes the override
	void Owner_UpdatePlayerOverrideIcon(PlayerId playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		if(!playerData || (playerData && overrideIcon == playerData.GetOverrideIcon()))
			return;

		Rpc(RpcAsk_UpdatePlayerOverrideIcon, playerID, overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerRank(PlayerId playerID, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		if (!playerData)
			return;

		// Only send when the value changed (called every update cycle), once until the server's broadcast echoes it
		if (playerData.GetRank() == m_iAskedRank)
			m_iAskedRank = -1;

		if (rank == playerData.GetRank() || rank == m_iAskedRank)
			return;

		m_iAskedRank = rank;
		Rpc(RpcAsk_UpdatePlayerRank, playerID, rank);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerTeamLeader(PlayerId playerID, bool isTL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		if(!playerData || (playerData && isTL == playerData.GetIsTeamLeader()))
			return;

		Rpc(RpcAsk_UpdatePlayerTeamLeader, playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerSquadLeader(PlayerId playerID, bool isSL)
	{
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		if (!playerData)
			return;

		// Only send when the value changed (called every update cycle), once until the server's broadcast echoes it
		int isSLValue = 0;
		if (isSL)
			isSLValue = 1;

		int currentValue = 0;
		if (playerData.GetIsSquadLeader())
			currentValue = 1;

		if (currentValue == m_iAskedSquadLeader)
			m_iAskedSquadLeader = -1;

		if (isSLValue == currentValue || isSLValue == m_iAskedSquadLeader)
			return;

		m_iAskedSquadLeader = isSLValue;
		Rpc(RpcAsk_UpdatePlayerSquadLeader, playerID, isSL);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_PromotePlayerToSL(PlayerId playerID)
	{
		Rpc(RpcAsk_PromotePlayerToSL, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID leader of the group to resize
	//! \param[in] maxMembers
	void Owner_SetMaxGroupMembers(PlayerId playerID, int maxMembers)
	{
		Rpc(RpcAsk_SetMaxGroupMembers, playerID, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID player to kick from the local player's group
	void Owner_RemovePlayerFromGroup(PlayerId playerID)
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

	//------------------------------------------------------------------------------------------------
	//! Admin: sets the top of the leaders' group size slider.
	//! \param[in] maxGroupSize
	void Owner_SetServerMaxGroupSize(int maxGroupSize)
	{
		Rpc(RpcAsk_SetServerMaxGroupSize, maxGroupSize);
	}

	//------------------------------------------------------------------------------------------------
	//! Admin: sets the defaults for players without saved settings.
	//! \param[in] values by settings index; a negative value keeps that setting's default as it is
	void Owner_SetServerDefaults(notnull array<int> values)
	{
		Rpc(RpcAsk_SetServerDefaults, values);
	}

	//------------------------------------------------------------------------------------------------
	//! Admin: clears the server's default settings.
	void Owner_ClearServerDefaults()
	{
		Rpc(RpcAsk_ClearServerDefaults);
	}

	//------------------------------------------------------------------------------------------------
	//! Asks the server for the radar outlines of the vehicle's hull and turrets.
	//! \param[in] vehicle
	//! \return false if the vehicle isn't replicated
	bool Owner_RequestVehicleOutlines(notnull IEntity vehicle)
	{
		RplComponent rplComponent = RplComponent.Cast(vehicle.FindComponent(RplComponent));
		if (!rplComponent)
			return false;

		Rpc(RpcAsk_RequestVehicleOutlines, rplComponent.Id());
		return true;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SERVER-SIDE RPC HANDLERS - Executed on the authority (server)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerDisplayIcon(PlayerId playerID, CSI_EIcon displayIcon)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerDisplayIcon(playerID, displayIcon);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerColorTeam(PlayerId playerID, CSI_EColorTeam colorTeam)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerColorTeam(playerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerRank(PlayerId playerID, SCR_ECharacterRank rank)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerRank(playerID, rank);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerOverrideIcon(PlayerId playerID, CSI_EOverrideIcon overrideIcon)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerOverrideIcon(playerID, overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerSquadLeader(PlayerId playerID, bool isSL)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerSquadLeader(playerID, isSL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerTeamLeader(PlayerId playerID, bool isTL)
	{
		if (playerID <= 0 || !m_RplBroadcastManager)
			return;

		m_RplBroadcastManager.UpdatePlayerTeamLeader(playerID, isTL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(PlayerId playerID)
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
	//! Sets the max size of the group led by playerID.
	//! \param[in] playerID
	//! \param[in] maxMembers
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(PlayerId playerID, int maxMembers)
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
	protected void RpcAsk_RemovePlayerFromGroup(PlayerId playerID)
	{
		if (playerID <= 0)
			return;

		// The sender's group controller validates the kick and moves the player
		SCR_PlayerControllerGroupComponent senderGroupController = SCR_PlayerControllerGroupComponent.Cast(GetOwner().FindComponent(SCR_PlayerControllerGroupComponent));
		if (!senderGroupController)
			return;

		senderGroupController.RPC_AskKickPlayer(playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ChangeAuthoritySetting(string setting, int value, bool serverOverrideEnabled)
	{
		// Only admins, and only known setting names
		if (!m_SettingsManager || !SCR_Global.IsAdmin(GetSenderPlayerID()) || !CSI_GameSettings.GetSettingsArray().Contains(setting))
			return;

		m_SettingsManager.UpdateAuthoritySetting(setting, value, serverOverrideEnabled);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetServerMaxGroupSize(int maxGroupSize)
	{
		if (!m_SettingsManager || !SCR_Global.IsAdmin(GetSenderPlayerID()))
			return;

		m_SettingsManager.UpdateMaxGroupSize(maxGroupSize);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetServerDefaults(array<int> values)
	{
		if (!values || !m_SettingsManager || !SCR_Global.IsAdmin(GetSenderPlayerID()))
			return;

		m_SettingsManager.UpdateServerDefaults(values);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ClearServerDefaults()
	{
		if (!m_SettingsManager || !SCR_Global.IsAdmin(GetSenderPlayerID()))
			return;

		m_SettingsManager.ClearServerDefaults();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestVehicleOutlines(RplId vehicleId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(vehicleId));
		if (!rplComponent)
			return;

		IEntity vehicle = rplComponent.GetEntity();
		CSI_VehicleOutlineSystem outlineSystem = CSI_VehicleOutlineSystem.GetInstance();
		if (!vehicle || !outlineSystem)
			return;

		outlineSystem.RequestOutlines(vehicle, this);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: ID of the player who sent the RpcAsk_ (this controller's owner).
	//! \return 0 if the owner isn't a player controller
	protected int GetSenderPlayerID()
	{
		PlayerController controller = PlayerController.Cast(GetOwner());
		if (!controller)
			return 0;

		return controller.GetPlayerId();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SERVER TO OWNER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Server: sends a vehicle outline to this controller's owner, once per prefab.
	//! \param[in] prefab the hull's or turret's prefab
	//! \param[in] outline
	void SendVehicleOutline(ResourceName prefab, notnull CSI_VehicleOutline outline)
	{
		if (m_aSentOutlinePrefabs.Contains(prefab))
			return;

		m_aSentOutlinePrefabs.Insert(prefab);
		Rpc(RpcDo_VehicleOutline, prefab, outline.ToData());
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_VehicleOutline(string prefab, array<float> data)
	{
		if (!data || data.IsEmpty())
			return;

		CSI_VehicleOutline.CacheData(prefab, data);

	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected static CSI_RplToAuthorityManager m_sInstance;	//!< last constructed instance, only a fallback
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