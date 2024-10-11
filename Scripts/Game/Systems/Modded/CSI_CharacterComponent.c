[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Character Component For Setting Default Values", color: "0 0 255 255")]
class CSI_CharacterComponentClass : ScriptComponentClass {};

class CSI_CharacterComponent : ScriptComponent
{		
	[Attribute(defvalue: "true", desc: "Should we set players starting color team/icon on respawn", category: "Default Player Settings")]
	protected bool m_bOverrideOnRespawn;
	
	[RplProp()]
	protected  ref array<string> m_aUnitPrefabColorTeams;
	[RplProp()]
	protected  ref array<string> m_aUnitPrefabOverrideIcons;
	[RplProp()]
	protected  int m_iUnitPrefabIndex;
	
	//------------------------------------------------------------------------------------------------
	void SetDefaults(int index, array<string> colorTeamArray, array<string> overrideIconArray)
	{
		if (!Replication.IsServer())
			return;

		m_aUnitPrefabColorTeams    = colorTeamArray;
		m_aUnitPrefabOverrideIcons = overrideIconArray;
		m_iUnitPrefabIndex         = index;
		
		Replication.BumpMe();
	}
	
	override protected void OnPostInit(IEntity owner)
	{
		if (Replication.IsServer())
			return;
		
		GetGame().GetCallqueue().CallLater(WaitUntilWeSetDefaults, 1000, true, owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void WaitUntilWeSetDefaults(IEntity owner)
	{				
		if(!SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning())
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(SCR_ChimeraCharacter.Cast(owner));
		
		if (playerID <= 0 && (!SCR_PlayerController.GetLocalControlledEntity() && !SCR_PlayerController.GetLocalMainEntity())) 
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
		
		if ((m_aUnitPrefabColorTeams.IsEmpty() && m_aUnitPrefabOverrideIcons.IsEmpty()) || (!m_bOverrideOnRespawn && (!authorityComponent.ReturnLocalPlayerMapValue(groupID, playerID, "CT").IsEmpty() || !authorityComponent.ReturnLocalPlayerMapValue(groupID, playerID, "OI").IsEmpty())))
			return;

		string colorTeam;
		string overrideIcon;
			
		if((m_aUnitPrefabColorTeams.Count() - 1) >= m_iUnitPrefabIndex)
			colorTeam = m_aUnitPrefabColorTeams.Get(m_iUnitPrefabIndex);
			
		if((m_aUnitPrefabOverrideIcons.Count() - 1) >= m_iUnitPrefabIndex)
			overrideIcon = m_aUnitPrefabOverrideIcons.Get(m_iUnitPrefabIndex);
			
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
				case "None"           : {clientComponent.Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "");    break;}; // CT = ColorTeam
			};
		};
	}
}