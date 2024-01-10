[ComponentEditorProps(category: "GameScripted/UI", description: "COA Player Component for RPC", color: "0 0 255 255")]
class COA_GroupDisplayComponentClass: ScriptComponentClass {};

class COA_GroupDisplayComponent : ScriptComponent
{	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------
	
	static COA_GroupDisplayComponent GetInstance()
	{
		if (GetGame().GetPlayerController())
			return COA_GroupDisplayComponent.Cast(GetGame().GetPlayerController().FindComponent(COA_GroupDisplayComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions for Player Settings replication
	
	//------------------------------------------------------------------------------------------------
	
	//- Promote Player To SL -\\
	
	void Owner_PromotePlayerToSL(int playerID)
	{	
		Rpc(RpcAsk_PromotePlayerToSL, playerID);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerID)
	{
		SCR_GroupsManagerComponent vanillaGroupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = vanillaGroupManager.GetPlayerGroup(playerID);
		playersGroup.SetGroupLeader(playerID);
	}
	
	//- Set Max Group Members -\\
	
	void Owner_SetMaxGroupMembers(int playerID, int maxMembers)
	{	
		Rpc(RpcAsk_SetMaxGroupMembers, playerID, maxMembers);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		SCR_GroupsManagerComponent vanillaGroupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = vanillaGroupManager.GetPlayerGroup(playerID);
		playersGroup.SetMaxMembers(maxMembers);
	}
	
	//- Remove Player From Group -\\
	
	void Owner_RemovePlayerFromGroup(int playerID)
	{	
		Rpc(RpcAsk_RemovePlayerFromGroup, playerID);
	}
	
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
	
	void Owner_UpdatePlayerMapValue(int playerID, string writeUpdate, string valueUpdate)
	{	
		Rpc(RpcAsk_UpdatePlayerMapValue, playerID, writeUpdate, valueUpdate);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerMapValue(int playerID, string writeUpdate, string valueUpdate)
	{
		COA_GroupDisplayManagerComponent groupBackendManagerComponent = COA_GroupDisplayManagerComponent.GetInstance();
		if (groupBackendManagerComponent)
			groupBackendManagerComponent.UpdateAuthorityPlayerMap(playerID, writeUpdate, valueUpdate);
	}
};