[ComponentEditorProps(category: "GameScripted/UI", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_ClientComponentClass: ScriptComponentClass {};

class CSI_ClientComponent : ScriptComponent
{	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------
	
	static CSI_ClientComponent GetInstance()
	{
		if (GetGame().GetPlayerController())
			return CSI_ClientComponent.Cast(GetGame().GetPlayerController().FindComponent(CSI_ClientComponent));
		else
			return null;
	}
	
	override protected void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		GetGame().GetCallqueue().CallLater(SetupClientSettings, 250);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to change Server Override Settings
	
	//------------------------------------------------------------------------------------------------
	
	//- Update A Server Setting -\\
	//------------------------------------------------------------------------------------------------
	void Owner_ChangeAuthoritySetting(string setting, int value)
	{	
		Rpc(RpcAsk_ChangeAuthoritySetting, setting, value);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ChangeAuthoritySetting(string setting, int value)
	{
		CSI_AuthorityComponent authorityComponent = CSI_AuthorityComponent.GetInstance();
		if (authorityComponent)
			authorityComponent.UpdateAuthoritySetting(setting, value);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to setup Client Settings
	
	//------------------------------------------------------------------------------------------------
	
	protected void SetupClientSettings()
	{
		bool clientSetupCompleted;
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("clientSetupCompleted", clientSetupCompleted);
		
		//  0 = false.
		//  1 = true.
		if (!clientSetupCompleted) {
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("compassVisible",      1);
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("compassTexture",   	  "{D19C93F5109F3E1D}UI\Textures\HUD\Modded\compass_shadow360.edds");
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("squadRadarVisible",   1);
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("groupDisplayVisible", 1);
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("staminaBarVisible",   1);
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("nametagsVisible",     1);
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("rankVisible",         0);
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("squadRadarIconSize",  100);
			
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("clientSetupCompleted", true);
			GetGame().UserSettingsChanged();
			GetGame().SaveUserSettings();
		};
	};
	
	//------------------------------------------------------------------------------------------------

	// Functions for Group/Player Settings replication
	
	//------------------------------------------------------------------------------------------------
	
	//- Promote Player To SL -\\
	//------------------------------------------------------------------------------------------------
	void Owner_PromotePlayerToSL(int playerID)
	{	
		Rpc(RpcAsk_PromotePlayerToSL, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);
		playersGroup.SetGroupLeader(playerID);
	}
	
	//- Set Max Group Members -\\
	//------------------------------------------------------------------------------------------------
	void Owner_SetMaxGroupMembers(int playerID, int maxMembers)
	{	
		Rpc(RpcAsk_SetMaxGroupMembers, playerID, maxMembers);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);
		playersGroup.SetMaxMembers(maxMembers);
	}
	
	//- Remove Player From Group -\\
	//------------------------------------------------------------------------------------------------
	void Owner_RemovePlayerFromGroup(int playerID)
	{	
		Rpc(RpcAsk_RemovePlayerFromGroup, playerID);
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

	// Functions for Group Display replication
	
	//------------------------------------------------------------------------------------------------

	void Owner_UpdatePlayerMapValue(int groupID, int playerID, string write, string value)
	{	
		Rpc(RpcAsk_UpdatePlayerMapValue, groupID, playerID, write, value);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerMapValue(int groupID, int playerID, string write, string value)
	{
		CSI_AuthorityComponent authorityComponent = CSI_AuthorityComponent.GetInstance();
		if (authorityComponent)
			authorityComponent.UpdateAuthorityPlayerMap(groupID, playerID, write, value);
	}
};