[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Charachter Component For Setting Default Values", color: "0 0 255 255")]
class CSI_CharacterComponentClass : ScriptComponentClass {};

class CSI_CharacterComponent : ScriptComponent
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Starting color team", category: "Default Player Settings", enums: { ParamEnum("None", "0"), ParamEnum("Red", "1"), ParamEnum("Blue", "2"), ParamEnum("Yellow", "3"), ParamEnum("Green", "4")})]
	protected int m_iStartingColorTeam;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Starting icon, especially usefull if you want to set teamleads automatically", category: "Default Player Settings", enums: { ParamEnum("None", "0"), ParamEnum("Team Lead", "1"), ParamEnum("Medic", "2"), ParamEnum("Marksman", "3"), ParamEnum("Machine Gunner", "4"), ParamEnum("Anti-Tank", "5"), ParamEnum("Grenadier", "6"), ParamEnum("Man", "7")})]
	protected int m_iStartingIcon;
	
	[Attribute(desc: "Should we set players starting color team/icon on respawn", category: "Default Player Settings")]
	protected bool m_bOverrideOnRespawn;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------
	
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode() || GetOwner() != owner || RplSession.Mode() == RplMode.Dedicated) 
			return;

		GetGame().GetCallqueue().CallLater(WaitUntilWeSetDefaults, 1000, true);
	}
	
	//------------------------------------------------------------------------------------------------

	// set default functions

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void WaitUntilWeSetDefaults()
	{		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		
		if (playerID == 0)
			return;
		
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (!groupsManagerComponent) 
			return;
		
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(playerID);
		
		if (!playersGroup)
			return;
		
		int groupID = playersGroup.GetGroupID();
		
		if (groupID == -1)
			return;
		
		if (GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID))
		{
			GetGame().GetCallqueue().Remove(WaitUntilWeSetDefaults);
		
			CSI_ClientComponent clientComponent = CSI_ClientComponent.GetInstance();
			CSI_AuthorityComponent authorityComponent = CSI_AuthorityComponent.GetInstance();
			
 			if (!clientComponent || !authorityComponent || (!m_bOverrideOnRespawn && !authorityComponent.ReturnLocalPlayerMapValue(groupID, playerID, "CT").IsEmpty()))
			 return;
			
			switch (m_iStartingColorTeam) 
			{
				case 1 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "R"); break;}; // CT = ColorTeam
				case 2 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "B"); break;}; // CT = ColorTeam
				case 3 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "Y"); break;}; // CT = ColorTeam
				case 4 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "G"); break;}; // CT = ColorTeam
			};

			switch (m_iStartingIcon) 
			{
				case 1 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "FTL"); break;}; // OI = OverrideIcon
				case 2 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MED"); break;}; // OI = OverrideIcon
				case 3 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MRK"); break;}; // OI = OverrideIcon
				case 4 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MG");  break;}; // OI = OverrideIcon
				case 5 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "AT");  break;}; // OI = OverrideIcon
				case 6 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "GRN"); break;}; // OI = OverrideIcon
				case 7 : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MAN"); break;}; // OI = OverrideIcon
			};
		};
	}
}