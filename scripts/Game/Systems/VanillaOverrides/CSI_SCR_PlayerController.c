modded class SCR_PlayerController
{
	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);
	
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
			
		CSI_CharacterManager characterComp = CSI_CharacterManager.Cast(to.FindComponent(CSI_CharacterManager));
		
		if(characterComp)
		{	
			SCR_GroupsManagerComponent groupsManagerComp = SCR_GroupsManagerComponent.GetInstance();
			GetGame().GetCallqueue().CallLater(SetPlayerMapValues, 1250, false, characterComp, groupsManagerComp);
		};
	}
	
	void SetPlayerMapValues(CSI_CharacterManager characterComp, SCR_GroupsManagerComponent groupsManagerComp)
	{
		/*
		if (!characterComp || !groupsManagerComp)
			return;
		
		SCR_AIGroup playersGroup = groupsManagerComp.GetPlayerGroup(GetLocalPlayerId());
		
		if (!playersGroup)
			return;
		
		string colorTeam;
		string overrideIcon;
		int playerID = GetLocalPlayerId();
		int index = characterComp.GetIndex();
		int groupID = playersGroup.GetGroupID();
		bool overrideOnSpawn = characterComp.GetOverrideOnSpawn();
		TStringArray colorTeamArray = characterComp.GetColorTeamArray();
		TStringArray overrideIconArray = characterComp.GetOverrideIcons();
		CSI_ClientManager clientComponent = CSI_ClientManager.GetInstance();
		CSI_AuthorityManager authorityComponent = CSI_AuthorityManager.GetInstance();
		
		if (groupID == -1 || !clientComponent || !authorityComponent || (colorTeamArray && colorTeamArray.IsEmpty()) && (overrideIconArray && overrideIconArray.IsEmpty()))
			return;
			
		if(colorTeamArray && ((colorTeamArray.Count() - 1) >= index))
			colorTeam = colorTeamArray.Get(index);
			
		if(overrideIconArray && ((overrideIconArray.Count() - 1) >= index))
			overrideIcon = overrideIconArray.Get(index);
			
		if (!colorTeam.IsEmpty())
		{
			switch (colorTeam)
			{
				case "Red"    : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "R"); break;}; // CT = ColorTeam
				case "Blue"   : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "B"); break;}; // CT = ColorTeam
				case "Yellow" : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "Y"); break;}; // CT = ColorTeam
				case "Green"  : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "G"); break;}; // CT = ColorTeam
				case "None"   : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "");  break;}; // CT = ColorTeam
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
				case "None"           : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "");    break;}; // OI = OverrideIcon
			};
		};
		*/
	}
}