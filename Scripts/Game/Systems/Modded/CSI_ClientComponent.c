[ComponentEditorProps(category: "GameScripted/UI", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_ClientComponentClass : ScriptComponentClass {};

class CSI_ClientComponent : ScriptComponent
{
	protected CSI_AuthorityComponent m_AuthorityComponent;

	// A array where we keep the local clients current group stored and sorted by the value determined for each player.
	protected ref array<string> m_aLocalGroupArray = new array<string>;

	protected ref array<string> m_aLocalCSISettingsArray = new array<string>;
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

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();

		if (!GetGame().InPlayMode()) return;

		CheckLocalCSISettings();

		GetGame().GetCallqueue().CallLater(UpdateLocalGroupArray, 550, true);
		GetGame().GetCallqueue().CallLater(CheckLocalCSISettings, 1225, true);
		GetGame().GetInputManager().AddActionListener("CSISettingsMenu", EActionTrigger.DOWN, ToggleCSISettingsMenu);
		GetGame().GetInputManager().AddActionListener("PlayerSelectionMenu", EActionTrigger.DOWN, TogglePlayerSelectionMenu);
	}

	//------------------------------------------------------------------------------------------------

	// Functions to sort and store the current group array we want to show on players screens.

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	TStringArray GetLocalGroupArray()
	{
		return m_aLocalGroupArray;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLocalGroupArray()
	{
		// Get base group manager component
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManagerComponent || !m_AuthorityComponent) return;

		// Get players current group.
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		if (!playersGroup) return;

		array<string> tempLocalGroupArray = {};
		int groupID = playersGroup.GetGroupID();

		string groupString = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, -1, "GroupString");

		if (groupString.IsEmpty()) return;

		array<string> outGroupStrArray = {};
		groupString.Split("║", outGroupStrArray, false);

		foreach (string playerString : outGroupStrArray) {
			tempLocalGroupArray.Insert(playerString);
		};

		m_aLocalGroupArray = tempLocalGroupArray;
	}

	//------------------------------------------------------------------------------------------------

	// Functions to change Server Override Settings

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	void Owner_ChangeAuthoritySetting(string setting, string value)
	{
		Rpc(RpcAsk_ChangeAuthoritySetting, setting, value);
	}

	//- Server -\\
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ChangeAuthoritySetting(string setting, string value)
	{
		m_AuthorityComponent.UpdateAuthoritySetting(setting, value);
	}

	//------------------------------------------------------------------------------------------------

	// Function for getting/setting local settings

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	TStringArray ReturnLocalCSISettings() {
		return m_aLocalCSISettingsArray;
	}

	//------------------------------------------------------------------------------------------------
	void ChangeLocalCSISetting(string setting, string value)
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckLocalCSISettings()
	{
		array<string> settingsToCheck = {

			// NOTE: even though they say int, it's just a int in a string.
			// Settings that can be overriden by the server
			"compassVisible|str",							//0
			"squadRadarVisible|str",					//1
			"groupDisplayVisible|str",				//2
			"staminaBarVisible|str",					//3
			"nametagsVisible|str",						//4
			"rankVisible|str",								//5
			"nametagsRange|int",							//6
			"roleNametagVisible|str",					//7
			"personalColorTeamMenu|str",			//8

			// Settings that are purely local to each client
			"squadRadarIconSize|int",					//9
			"squadRadarSelfIconVisible|str",	//10
			"nametagsPosition|str",						//11
			"compassTexture|str",							//12
		};

		array<string> tempLocalCSISettingsArray = {};

		foreach (int i, string setting : settingsToCheck)
		{
			array<string> settingTypeArray = {};
			setting.Split("|", settingTypeArray, false);

			setting = settingTypeArray[0];
			string settingType = settingTypeArray[1];

			string settingValue = "";
			string settingServerOverride = "";
			if (i < 9) {
				settingServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[i];
			};
			switch (true)
			{
				case((settingType == "str" && (settingServerOverride == "true" || settingServerOverride == "false")) || (settingType == "int" && !(settingServerOverride == "" || settingServerOverride == "N/A"))) : {settingValue = settingServerOverride; break; };
				default : {GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(setting, settingValue); break; };
			};
			tempLocalCSISettingsArray.Insert(settingValue);
		};
		m_aLocalCSISettingsArray = tempLocalCSISettingsArray;
	}

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

		if (maxMembers < playersGroup.GetPlayerCount()) maxMembers = playersGroup.GetPlayerCount();

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
		if (m_AuthorityComponent)
			m_AuthorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, write, value);
	}

	//------------------------------------------------------------------------------------------------

	// Functions for Menus

	//------------------------------------------------------------------------------------------------

	protected void TogglePlayerSelectionMenu()
	{
		string storedSpecialtyIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId()).GetGroupID(), SCR_PlayerController.GetLocalPlayerId(), "StoredSpecialtyIcon");

		if (ReturnLocalCSISettings()[8] == "false" || (SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId()).IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()) && storedSpecialtyIcon == "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds") || (storedSpecialtyIcon == "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds"))
		{
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSelectionDialog);
			return;
		} else {
			MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSettingsDialog, 0, true);
			CSI_PlayerSettingsDialog colorTeamMenu = CSI_PlayerSettingsDialog.Cast(menu);

			colorTeamMenu.SetPlayerStr(string.Format("PlayerID╣%1", SCR_PlayerController.GetLocalPlayerId()));
			return;
		};
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleCSISettingsMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_SettingsDialog);
	}
}
