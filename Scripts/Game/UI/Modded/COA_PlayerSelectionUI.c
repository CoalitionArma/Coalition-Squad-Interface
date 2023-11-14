modded enum ChimeraMenuPreset {
	COA_PlayerSettingsUI,
	COA_PlayerSelectionUI
}

modded class GroupSettingsDialogUI : DialogUI
{
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SCR_ButtonBaseComponent.GetButtonBase("ColorTeams", GetRootWidget()).m_OnClicked.Insert(CTButtonClicked);
	}
	
	protected void CTButtonClicked()
	{
		GetGame().GetMenuManager().CloseMenu(this);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.COA_PlayerSelectionUI);
	}
}

class COA_PlayerSelectionUI : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected InputManager m_InputManager;
	protected string groupString = "";
	protected string groupStringStored = "N/A";
	
	protected int PlayerInt;
	protected int SelectedPlayerID = 0;
	
	protected SCR_PlayerController playerController = null;
	protected SCR_GroupsManagerComponent groupManager = null;
	protected SCR_AIGroup playersGroup = null;
	
	override void OnMenuShow() 
	{
		UpdatePlayerList();
	}
	
	override void OnMenuInit()
	{
		m_wRoot = GetRootWidget();
		
		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_ButtonBaseComponent.GetButtonBase("Back", m_wRoot).m_OnClicked.Insert(OnMenuBack);

		// Get Global Player Controller and Group Manager.
		playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		groupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));

		if (!playerController || !groupManager) {
			GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
			GetGame().GetMenuManager().CloseMenu(this);
			return;
		};
		
		for (int b = 0; b < 12; b++)
		{
			SCR_ButtonBaseComponent.GetButtonBase(string.Format("Button%1", b), m_wRoot).m_OnClicked.Insert(OnPlayerEntryClicked);
		};
	}
	
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseMenu(this);
	}
		
	protected void OnPlayerEntryClicked(SCR_ButtonBaseComponent PEcomponent)
	{
		string PlayerIntStr = PEcomponent.GetRootWidget().GetName();
		
		PlayerInt = PlayerIntStr.ToInt(offset: 6);
		
		ImageWidget statusDisplayList = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", PlayerInt)));
		int playerOInt = statusDisplayList.GetOpacity();
		if (!playerOInt || playerOInt == 0) return;

		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.COA_PlayerSettingsUI, 0, true);
		COA_PlayerSettingsUI colorTeamMenu = COA_PlayerSettingsUI.Cast(menu);
		
		colorTeamMenu.SetPlayerInt(PlayerInt);
	}
	
	protected void UpdatePlayerList()
	{
		playersGroup = groupManager.GetPlayerGroup(playerController.GetPlayerId());
		
		if (!playersGroup) return;

		groupString = groupManager.ReturnGroupMapValue(playersGroup.ToString());

		if (!groupString || groupString == "") {
			GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
			GetGame().GetMenuManager().CloseMenu(this);
			return;
		};

		if (groupString == groupStringStored) return;
		groupStringStored = groupString;

		array<string> localGroupSplitString = {};
		groupString.Split(";", localGroupSplitString, true);

		int groupCount = localGroupSplitString.Count();
		groupCount = groupCount/ 4;

		int playerPlace = 0;

		// Parse through current group array.
		for (int i = 0; i<groupCount && i < 12; i++)
		{
			string playerName = localGroupSplitString[playerPlace];
			string colorTeam = localGroupSplitString[playerPlace + 1];
			string icon = localGroupSplitString[playerPlace + 2];

			playerPlace = playerPlace + 4;

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));

			playerDisplay.SetText(playerName);
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, icon);
			
			float sx = 0;
			float yx = 0;
			playerDisplay.GetTextSize(sx, yx);
			
			if (sx > 170) {
			  	for (int e = 0; sx > 160; e++)
				{
					int playerNameLength = playerName.Length();
					playerNameLength = playerNameLength - 1;
					playerName = playerName.Substring(0, playerNameLength);
					
					//Players display wont visually update until the UpdatePlayerList function ends, so they don't see this.
					playerDisplay.SetText(playerName);
					playerDisplay.GetTextSize(sx, yx);
				};
				playerName = string.Format("%1......", playerName);
				playerDisplay.SetText(playerName);
			};
			

			if (colorTeam) {
				switch (colorTeam)
				{
					case "Red"    : {statusDisplay.SetColorInt(ARGB(255, 200, 65, 65));   playerDisplay.SetColorInt(ARGB(255, 200, 65, 65));   break; };
					case "Blue"   : {statusDisplay.SetColorInt(ARGB(255, 0, 92, 255));    playerDisplay.SetColorInt(ARGB(255, 0, 92, 255));    break; };
					case "Yellow" : {statusDisplay.SetColorInt(ARGB(255, 230, 230, 0));   playerDisplay.SetColorInt(ARGB(255, 230, 230, 0));   break; };
					case "Green"  : {statusDisplay.SetColorInt(ARGB(255, 0, 190, 85));    playerDisplay.SetColorInt(ARGB(255, 0, 190, 85));    break; };
					case "None"   : {statusDisplay.SetColorInt(ARGB(255, 255, 255, 255)); playerDisplay.SetColorInt(ARGB(255, 255, 255, 255)); break; };
				};
			};
		};
		for (int e = groupCount; e<=12; e++)
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
}
