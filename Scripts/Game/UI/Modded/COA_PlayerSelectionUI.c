modded enum ChimeraMenuPreset {
	COA_PlayerSettingsUI,
	COA_PlayerSelectionUI
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
	protected ref array<string> PlayerGroupString = {};
	
	protected SCR_PlayerController playerController = null;
	protected SCR_GroupsManagerComponent vanillaGroupManager = null;
	protected COA_GroupDisplayManagerComponent groupManagerCOA = null;
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
		SCR_InputButtonComponent Back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		Back.m_OnClicked.Insert(OnMenuBack);

		// Get Global Player Controller and Group Manager.
		vanillaGroupManager = SCR_GroupsManagerComponent.GetInstance();
		groupManagerCOA = COA_GroupDisplayManagerComponent.GetInstance();

		if (!vanillaGroupManager) {
			GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
			GetGame().GetMenuManager().CloseMenu(this);
			return;
		};
		
		for (int b = 0; b < 12; b++)
		{
			SCR_InputButtonComponent buttonList = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget(string.Format("Button%1", b)).FindHandler(SCR_InputButtonComponent));
			buttonList.m_OnClicked.Insert(OnPlayerEntryClicked);
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
		
		colorTeamMenu.SetPlayerStr(PlayerGroupString[PlayerInt]);
	}
	
	protected void UpdatePlayerList()
	{
		playersGroup = vanillaGroupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		// If player doesn't have a group we can't do much, better check then clear their group display, just in case.
		if (!playersGroup) {			
			GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
			GetGame().GetMenuManager().CloseMenu(this);
			return; 
		};
			
		array<int> PlayerIDsArray = playersGroup.GetPlayerIDs();
		int groupCount = PlayerIDsArray.Count();
			
		if (groupCount <= 1) {
			GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
			GetGame().GetMenuManager().CloseMenu(this);
			return;  
		};
			
		PlayerGroupString = {};
			
		foreach (int PlayerID : PlayerIDsArray) {
			string PlayerStr = groupManagerCOA.ReturnLocalPlayerMapValue("PlayerGroupValues", PlayerID);
			PlayerGroupString.Insert(PlayerStr);
		}
			
		PlayerGroupString.Sort(false);
			
		foreach (int i, string PlayerString : PlayerGroupString) {
				
			array<string> removeValueArray = {};
			PlayerString.Split("╣", removeValueArray, false);
			PlayerString = removeValueArray[1];
				
			array<string> localPlayerStringSplit = {};
			PlayerString.Split("║", localPlayerStringSplit, false);
			
			string playerName       = localPlayerStringSplit[0];
			string colorTeam        = localPlayerStringSplit[1];
			string localPlayerIDStr = localPlayerStringSplit[3];
			int localPlayerID = localPlayerIDStr.ToInt();
			
			string icon = groupManagerCOA.ReturnLocalPlayerMapValue("StoredIcon", localPlayerID);

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
