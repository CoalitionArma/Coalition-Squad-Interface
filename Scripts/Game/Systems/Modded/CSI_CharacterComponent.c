[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Group Component For Setting Default Values", color: "0 0 255 255")]
class CSI_CharacterComponentClass : ScriptComponentClass {};

class CSI_CharacterComponent : ScriptComponent
{		
	IEntity m_eOwner;
	
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_eOwner = owner
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefaults(int index, array<string> colorTeamArray, array<string> overrideIconArray)
	{
		if (!Replication.IsServer() || SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning())
			return;

		GetGame().GetCallqueue().CallLater(WaitUntilWeSetDefaults, 1000, true, index, colorTeamArray, overrideIconArray);
	}

	//------------------------------------------------------------------------------------------------
	protected void WaitUntilWeSetDefaults(int index, array<string> colorTeamArray, array<string> overrideIconArray)
	{				
		if(!SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning() || !m_eOwner)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(SCR_ChimeraCharacter.Cast(m_eOwner));
		
		if (playerID == 0)
			return;
		
		GetGame().GetCallqueue().Remove(WaitUntilWeSetDefaults);
		
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (!groupsManagerComponent)
			return;
		
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(playerID);
		
		if (!playersGroup)
			return;
		
		int groupID = playersGroup.GetGroupID();
		CSI_ClientComponent clientComponent = CSI_ClientComponent.GetInstance();
		CSI_AuthorityComponent authorityComponent = CSI_AuthorityComponent.GetInstance();
		
		if (groupID == -1 || !clientComponent || !authorityComponent)
			return;

		string colorTeam;
		string overrideIcon;
			
		if((colorTeamArray.Count() - 1) >= index)
			colorTeam = colorTeamArray.Get(index);
			
		if((overrideIconArray.Count() - 1) >= index)
			overrideIcon = overrideIconArray.Get(index);
			
		if (!colorTeam.IsEmpty())
		{
			switch (colorTeam)
			{
				case "Red"    : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "CT", "R"); break;}; // CT = ColorTeam
				case "Blue"   : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "CT", "B"); break;}; // CT = ColorTeam
				case "Yellow" : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "CT", "Y"); break;}; // CT = ColorTeam
				case "Green"  : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "CT", "G"); break;}; // CT = ColorTeam
				case "None"   : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "CT", "");  break;}; // CT = ColorTeam
			};
		};
			
		if (!overrideIcon.IsEmpty())
		{
			switch (overrideIcon)
			{
				case "Team Lead"      : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "FTL"); break;}; // OI = OverrideIcon
				case "Medic"          : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "MED"); break;}; // OI = OverrideIcon
				case "Marksman"       : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "MRK"); break;}; // OI = OverrideIcon
				case "Machine Gunner" : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "MG");  break;}; // OI = OverrideIcon
				case "Anti-Tank"      : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "AT");  break;}; // OI = OverrideIcon
				case "Grenadier"      : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "GRN"); break;}; // OI = OverrideIcon
				case "Demolitionist"  : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "EXP"); break;}; // OI = OverrideIcon
				case "Engineer"       : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "ENG"); break;}; // OI = OverrideIcon
				case "Man"            : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "MAN"); break;}; // OI = OverrideIcon
				case "None"           : {clientComponent.RpcAsk_UpdatePlayerMapValue(groupID, playerID, "OI", "");    break;}; // CT = ColorTeam
			};
		};
	}
}