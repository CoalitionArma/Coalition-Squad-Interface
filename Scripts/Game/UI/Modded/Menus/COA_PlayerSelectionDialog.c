modded enum ChimeraMenuPreset {
	COA_PlayerSettingsDialog,
	COA_PlayerSelectionDialog
}

modded class GroupSettingsDialogUI : DialogUI
{
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SCR_InputButtonComponent ColorTeams = SCR_InputButtonComponent.Cast(GetRootWidget().FindAnyWidget("ColorTeams").FindHandler(SCR_InputButtonComponent));
		ColorTeams.m_OnClicked.Insert(CTButtonClicked);
	}
	
	protected void CTButtonClicked()
	{
		GetGame().GetMenuManager().CloseMenu(this);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.COA_PlayerSelectionDialog);
	}
}

class COA_PlayerSelectionDialog : ChimeraMenuBase
{
	protected Widget m_wRoot;
	
	protected XComboBoxWidget m_wMaxPlayers = null;
	
	protected SCR_PlayerController playerController = null;
	protected SCR_GroupsManagerComponent groupManager = null;
	protected COA_GroupDisplayManagerComponent customGroupManager = null;
	protected SCR_AIGroup playersGroup = null;
	protected int m_iGroupCount = 0;
	protected ref array<string> m_aGroupArray = {};
	
	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override void OnMenuShow() 
	{	
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		
		m_wRoot = GetRootWidget();
		
		// Get Global Player Controller and Group Manager.
		groupManager = SCR_GroupsManagerComponent.GetInstance();
		customGroupManager = COA_GroupDisplayManagerComponent.GetInstance();

		if (!groupManager || !customGroupManager) {OnMenuBack(); return;};
		
		playersGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (!playersGroup) {OnMenuBack(); return;};
		
		array<int> playerIDsArray = playersGroup.GetPlayerIDs();
		m_iGroupCount = playerIDsArray.Count();
			
		if (m_iGroupCount <= 1) {OnMenuBack(); return;};
		
		string storedSpecialtyIcon = customGroupManager.ReturnLocalPlayerMapValue(playersGroup.GetGroupID(), SCR_PlayerController.GetLocalPlayerId(), "StoredSpecialtyIcon");
		
		if ((playersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()) && storedSpecialtyIcon == "{5ECE094ED4662B33}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds") || storedSpecialtyIcon == "{6D45BA2CCC322312}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds")
		{
			UpdatePlayerList();
		} else {
			OnMenuBack();
			return;
		};
		
		if (playersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()) && storedSpecialtyIcon == "{5ECE094ED4662B33}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds")
		{
			ShowGroupSettings();
		}
		
		GetGame().GetCallqueue().CallLater(UpdatePlayerList, 425, true);
	}
	
	override void OnMenuInit()
	{	
		m_wRoot = GetRootWidget();
		
		for (int b = 0; b <= 19; b++)
		{
			SCR_InputButtonComponent buttonList = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget(string.Format("Button%1", b)).FindHandler(SCR_InputButtonComponent));
			buttonList.m_OnClicked.Insert(OnPlayerEntryClicked);
		};
				
		SCR_InputButtonComponent Back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		Back.m_OnClicked.Insert(OnMenuBack);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to update visual icons/text (player icon, player name, etc.)

	//------------------------------------------------------------------------------------------------
	
	protected void UpdatePlayerList()
	{		
		m_aGroupArray = customGroupManager.GetLocalGroupArray();
			
		if (m_aGroupArray.Count() <= 1) {OnMenuBack(); return;};
		
		foreach (int i, string playerStringToSplit : m_aGroupArray) {
			
			array<string> playerSplitArray = {};
			playerStringToSplit.Split("╣", playerSplitArray, false);
			
			// Get all values we need to display this player.
			int playerID = playerSplitArray[1].ToInt();
			string colorTeam = playerSplitArray[2];
			string icon = playerSplitArray[3];

			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));
			
			// Check if we need to add ... to the end of players names.
			playerName = CheckEllipsis(175, playerDisplay, playerName);

			playerDisplay.SetText(playerName);
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, icon);

			// Set color team.
			if (colorTeam) {
				switch (colorTeam)
				{
					case "Red"    : {statusDisplay.SetColorInt(ARGB(255, 200, 65, 65));   playerDisplay.SetColorInt(ARGB(255, 200, 65, 65));   break; };
					case "Blue"   : {statusDisplay.SetColorInt(ARGB(255, 0, 92, 255));    playerDisplay.SetColorInt(ARGB(255, 0, 92, 255));    break; };
					case "Yellow" : {statusDisplay.SetColorInt(ARGB(255, 230, 230, 0));   playerDisplay.SetColorInt(ARGB(255, 230, 230, 0));   break; };
					case "Green"  : {statusDisplay.SetColorInt(ARGB(255, 0, 190, 85));    playerDisplay.SetColorInt(ARGB(255, 0, 190, 85));    break; };
					case "None"   : {statusDisplay.SetColorInt(ARGB(235, 235, 235, 255)); playerDisplay.SetColorInt(ARGB(235, 235, 235, 255)); break; };
				};
			};
		};
		for (int e = m_aGroupArray.Count(); e <= 19; e++)
		{
			// Get group display widgets.
			TextWidget playerRemoveDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", e)));
			ImageWidget statusRemoveDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", e)));

			// Skip ahead to next for-loop iteration if either of these are false.
			if (!playerRemoveDisplay || !statusRemoveDisplay) continue;

			// Clear widgets.
			playerRemoveDisplay.SetText("");
			statusRemoveDisplay.SetOpacity(0);
		}
	};
	
	string CheckEllipsis(float maxLength, TextWidget nameWidget, string name)
	{	
		float sx = 0;
		float yx = 0;
		nameWidget.GetTextSize(sx, yx);
		
		if (sx > maxLength) {
			for (int e = 0; sx > maxLength - 3.5; e++)
			{
				int nameLength = name.Length();
				nameLength = nameLength - 1;
				name = name.Substring(0, nameLength);

				nameWidget.SetText(name);
				nameWidget.GetTextSize(sx, yx);
			};
			name = string.Format("%1...", name);
		};
		return name;
	};
	
	//------------------------------------------------------------------------------------------------

	// Additional menu functions

	//------------------------------------------------------------------------------------------------
	
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseMenu(this);
	}
	
	void ShowGroupSettings()
	{
		ImageWidget background0           = ImageWidget.Cast(m_wRoot.FindAnyWidget("Background0"));
		ImageWidget pretty0               = ImageWidget.Cast(m_wRoot.FindAnyWidget("Pretty0"));
		TextWidget prettyText0            = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyText0"));
		TextWidget confirmMaxPlayersText  = TextWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayersText"));		
		ImageWidget confirmMaxPlayersIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayersIcon"));
		ButtonWidget confirmMaxPlayers    = ButtonWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayers"));
		m_wMaxPlayers                        = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("MaxPlayers"));
		
		
		SCR_InputButtonComponent confirmMaxPlayersComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayers").FindHandler(SCR_InputButtonComponent));
		confirmMaxPlayersComp.m_OnClicked.Insert(OnConfirmMaxPlayersClicked);
		
		background0.SetOpacity(1);
		pretty0.SetOpacity(1);
		prettyText0.SetOpacity(1);
		confirmMaxPlayersText.SetOpacity(1);
		confirmMaxPlayersIcon.SetOpacity(1);
		confirmMaxPlayers.SetOpacity(1);
		m_wMaxPlayers.SetOpacity(1);
		
		int maxMembers = playersGroup.GetMaxMembers();
		maxMembers = maxMembers - 1;
		
		m_wMaxPlayers.SetCurrentItem(maxMembers);
	};
	
	//------------------------------------------------------------------------------------------------

	// Button functions

	//------------------------------------------------------------------------------------------------
	
	protected void OnConfirmMaxPlayersClicked()
	{
		COA_GroupDisplayComponent groupBackendComponent = COA_GroupDisplayComponent.GetInstance();
		
		int maxMembers = m_wMaxPlayers.GetCurrentItem();
		
		maxMembers = maxMembers + 1;
		
		groupBackendComponent.Owner_SetMaxGroupMembers(SCR_PlayerController.GetLocalPlayerId(), maxMembers);
	};
		
	protected void OnPlayerEntryClicked(SCR_ButtonBaseComponent component)
	{
		string playerIntStr = component.GetRootWidget().GetName();
		
		int playerInt = playerIntStr.ToInt(offset: 6);
		
		ImageWidget statusDisplayList = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", playerInt)));
		int playerOInt = statusDisplayList.GetOpacity();
		if (!playerOInt || playerOInt == 0) return;
		
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.COA_PlayerSettingsDialog, 0, true);
		COA_PlayerSettingsDialog colorTeamMenu = COA_PlayerSettingsDialog.Cast(menu);
		
		colorTeamMenu.SetPlayerStr(m_aGroupArray[playerInt]);
	}
}
