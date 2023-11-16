class COA_PlayerSettingsUI : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected InputManager m_InputManager;
	protected SCR_ButtonTextComponent comp;

	protected XComboBoxWidget IconOveride;
	protected ImageWidget Icon;
	protected TextWidget PlayerName;

	protected int PlayerInt;
	protected int SelectedPlayerID = 0;
	protected SCR_PlayerController playerController = null;
	protected SCR_GroupsManagerComponent groupManager = null;
	protected SCR_AIGroup playersGroup = null;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_wRoot = GetRootWidget();
		
		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_ButtonBaseComponent.GetButtonBase("Back", m_wRoot).m_OnClicked.Insert(OnMenuBack);

		// Get Global Player Controller and Group Manager.
		playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		groupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));

		if (!playerController || !groupManager) return;

		playersGroup = groupManager.GetPlayerGroup(playerController.GetPlayerId());

		IconOveride = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconOveride"));
		PlayerName = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		Icon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		
		SCR_ButtonBaseComponent.GetButtonBase("ConfirmIOButton", m_wRoot).m_OnClicked.Insert(OnOverrideIconClicked);
		SCR_ButtonBaseComponent.GetButtonBase("Red", m_wRoot).m_OnClicked.Insert(OnColorTeamClicked);
		SCR_ButtonBaseComponent.GetButtonBase("Blue", m_wRoot).m_OnClicked.Insert(OnColorTeamClicked);
		SCR_ButtonBaseComponent.GetButtonBase("Yellow", m_wRoot).m_OnClicked.Insert(OnColorTeamClicked);
		SCR_ButtonBaseComponent.GetButtonBase("Green", m_wRoot).m_OnClicked.Insert(OnColorTeamClicked);
		SCR_ButtonBaseComponent.GetButtonBase("None", m_wRoot).m_OnClicked.Insert(OnColorTeamClicked);
	}
	
	void SetPlayerInt(int PI)
	{
		PlayerInt = PI;
		
		UpdatePlayaerIcon();
	}

	protected void OnColorTeamClicked(SCR_ButtonBaseComponent CTcomponent)
	{
		if (PlayerName.GetText() == "No Player Selected") return;
		string ColorTeam = CTcomponent.GetRootWidget().GetName();
		if (!SelectedPlayerID) return;
		groupManager.UpdateUIvalue(SelectedPlayerID, "ColorTeam", ColorTeam);
		GetGame().GetCallqueue().CallLater(UpdatePlayaerIcon, 685);
	}

	protected void OnOverrideIconClicked()
	{
		if (PlayerName.GetText() == "No Player Selected") return;
		int iconToOverride = IconOveride.GetCurrentItem();
		string iconToOverrideStr = "";
		switch (iconToOverride)
		{
			case 0 : {iconToOverrideStr = "Auto";           break; };
			case 1 : {iconToOverrideStr = "Team Lead";      break; };
			case 2 : {iconToOverrideStr = "Medic";          break; };
			case 3 : {iconToOverrideStr = "Sniper";         break; };
			case 4 : {iconToOverrideStr = "Machine Gunner"; break; };
			case 5 : {iconToOverrideStr = "Anti-Tank";      break; };
			case 6 : {iconToOverrideStr = "Grenadier";      break; };
			case 7 : {iconToOverrideStr = "Man";            break; };
		};

		if (!SelectedPlayerID) return;
		groupManager.UpdateUIvalue(SelectedPlayerID, "OverrideIcon", iconToOverrideStr);
		GetGame().GetCallqueue().CallLater(UpdatePlayaerIcon, 685);
	}
	
	protected void UpdatePlayaerIcon() 
	{
		string groupStringOI = groupManager.ReturnGroupMapValue(playersGroup.ToString());
		
		array<string> localGroupSplitStringOI = {};
		groupStringOI.Split(";", localGroupSplitStringOI, true);
		
		int groupCountOI = localGroupSplitStringOI.Count();
		groupCountOI = groupCountOI/ 4;
		
		int playerPlaceOI = PlayerInt * 4;
		
		string playerNameOI = localGroupSplitStringOI[playerPlaceOI];
		string ColorTeamOI = localGroupSplitStringOI[playerPlaceOI + 1];
		string iconOI = localGroupSplitStringOI[playerPlaceOI + 2];
		string SelectedPlayerIDStr = localGroupSplitStringOI[playerPlaceOI + 3];
		
		if (ColorTeamOI || ColorTeamOI != "") {
			switch (ColorTeamOI)
			{
				case "Red"    : {PlayerName.SetColorInt(ARGB(255, 200, 65, 65));   Icon.SetColorInt(ARGB(255, 200, 65, 65));   break; };
				case "Blue"   : {PlayerName.SetColorInt(ARGB(255, 0, 92, 255));    Icon.SetColorInt(ARGB(255, 0, 92, 255));    break; };
				case "Yellow" : {PlayerName.SetColorInt(ARGB(255, 230, 230, 0));   Icon.SetColorInt(ARGB(255, 230, 230, 0));   break; };
				case "Green"  : {PlayerName.SetColorInt(ARGB(255, 0, 190, 85));    Icon.SetColorInt(ARGB(255, 0, 190, 85));    break; };
				case "None"   : {PlayerName.SetColorInt(ARGB(255, 255, 255, 255)); Icon.SetColorInt(ARGB(255, 255, 255, 255)); break; };
			};
		};
		
		SelectedPlayerID = SelectedPlayerIDStr.ToInt();

		Icon.LoadImageTexture(0, iconOI);
		PlayerName.SetText(playerNameOI);
		
		string playerOverideIconStr = groupManager.ReturnUIValue(SelectedPlayerID, "OverrideIcon");
		if (playerOverideIconStr && playerOverideIconStr != "") {
			int playerOverideIcon = 0;
			switch (playerOverideIconStr)
			{
				case "Team Lead"      : {playerOverideIcon = 1; break; };
				case "Medic"          : {playerOverideIcon = 2; break; };
				case "Sniper"         : {playerOverideIcon = 3; break; };
				case "Machine Gunner" : {playerOverideIcon = 4; break; };
				case "Anti-Tank"      : {playerOverideIcon = 5; break; };
				case "Grenadier"      : {playerOverideIcon = 6; break; };
				case "Man"            : {playerOverideIcon = 7; break; };
			};
			IconOveride.SetCurrentItem(playerOverideIcon);
		};
	}
	
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseMenu(this);
	}
}
