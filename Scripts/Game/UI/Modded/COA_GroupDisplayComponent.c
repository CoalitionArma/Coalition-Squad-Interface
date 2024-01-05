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

	// Functions for replication
	
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