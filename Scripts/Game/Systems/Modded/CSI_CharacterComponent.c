[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Group Component For Setting Default Values", color: "0 0 255 255")]
class CSI_CharacterComponentClass : ScriptComponentClass {};

class CSI_CharacterComponent : ScriptComponent
{	
	[Attribute(desc: "Should we set players starting color team/icon on respawn", category: "Default Player Settings")]
	protected bool m_bOverrideOnRespawn;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------
	
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (!GetGame().InPlayMode() || RplSession.Mode() == RplMode.Dedicated)
			return;

		GetGame().GetCallqueue().CallLater(WaitUntilWeSetDefaults, 1000, true, owner);
	}
	
	//------------------------------------------------------------------------------------------------

	// set default functions

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void WaitUntilWeSetDefaults(IEntity owner)
	{				
		if(!SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning())
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(SCR_ChimeraCharacter.Cast(owner));
		
		if (playerID == 0 && (!SCR_PlayerController.GetLocalControlledEntity() && !SCR_PlayerController.GetLocalMainEntity())) 
			return;
		
		if (playerID != SCR_PlayerController.GetLocalPlayerId())
		{
			GetGame().GetCallqueue().Remove(WaitUntilWeSetDefaults);
			return;
		}
		
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

		GetGame().GetCallqueue().Remove(WaitUntilWeSetDefaults);
		
		int index = -1;
		string overrideIcon;
		string colorTeam;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		array<array<RplId>> groupsIDsArray = new array<array<RplId>>;
		groupsIDsArray = groupManager.ReturnAllGroupIDsArray();
		
		foreach(int i, array<RplId> groupIDs : groupsIDsArray)
		{
			if (groupIDs)
				index = groupIDs.Find(Replication.FindId(SCR_PlayerController.GetLocalMainEntity()));
			
			Print(index);
			
			if (index == -1 || !m_bOverrideOnRespawn && (!authorityComponent.ReturnLocalPlayerMapValue(groupID, playerID, "CT").IsEmpty() || !authorityComponent.ReturnLocalPlayerMapValue(groupID, playerID, "OI").IsEmpty()))
				continue;
			
			array<string> colorTeamArray = groupManager.ReturnAllGroupPrefabColorsArray()[i];
			array<string> overrideIconArray = groupManager.ReturnAllGroupPrefabOverridesArray()[i];
			
			Print(colorTeamArray);
			Print(overrideIconArray);
			
			if((colorTeamArray.Count() - 1) >= index)
				colorTeam = colorTeamArray.Get(index);
			
			if((overrideIconArray.Count() - 1) >= index)
				overrideIcon = overrideIconArray.Get(index);
			
			if (!colorTeam.IsEmpty())
			{
				switch (colorTeam)
				{
					case "Red"    : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "R"); break;}; // CT = ColorTeam
					case "Blue"   : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "B"); break;}; // CT = ColorTeam
					case "Yellow" : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "Y"); break;}; // CT = ColorTeam
					case "Green"  : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "G"); break;}; // CT = ColorTeam
				};
			};
			
			if (!overrideIcon.IsEmpty())
			{
				switch (overrideIcon)
				{
					case "Team Lead"      : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "FTL"); break;}; // OI = OverrideIcon
					case "Medic"          : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MED"); break;}; // OI = OverrideIcon
					case "Marksman"       : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MRK"); break;}; // OI = OverrideIcon
					case "Machine Gunner" : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MG");  break;}; // OI = OverrideIcon
					case "Anti-Tank"      : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "AT");  break;}; // OI = OverrideIcon
					case "Grenadier"      : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "GRN"); break;}; // OI = OverrideIcon
					case "Demolitionist"  : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "EXP"); break;}; // OI = OverrideIcon
					case "Engineer"       : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "ENG"); break;}; // OI = OverrideIcon
					case "Man"            : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MAN"); break;}; // OI = OverrideIcon
				};
			};
			
			return;
		};
	}
}