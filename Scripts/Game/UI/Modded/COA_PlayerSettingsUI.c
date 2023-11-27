class COA_PlayerSettingsUI : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected InputManager m_InputManager;
	protected SCR_ButtonTextComponent comp;

	protected XComboBoxWidget IconOveride;
	protected ImageWidget Icon;
	protected TextWidget PlayerName;
	
	protected int SelectedPlayerID = 0;
	protected COA_GroupDisplayManagerComponent groupManagerCOA = null;
	protected SCR_AIGroup playersGroup = null;
	protected COA_GroupDisplayComponent groupBackendComponent = null;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_wRoot = GetRootWidget();
		
		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent Back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		Back.m_OnClicked.Insert(OnMenuBack);

		// Get Global Player Controller and Group Manager.
		groupManagerCOA = COA_GroupDisplayManagerComponent.GetInstance();
		if (!groupManagerCOA) return;

		IconOveride = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconOveride"));
		PlayerName = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		Icon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		
		SCR_InputButtonComponent ConfirmIOButton = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmIOButton").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent Red             = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Red").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent Blue            = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Blue").FindHandler(SCR_InputButtonComponent));		
		SCR_InputButtonComponent Yellow          = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Yellow").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent Green           = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Green").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent None            = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("None").FindHandler(SCR_InputButtonComponent));
		
		ConfirmIOButton.m_OnClicked.Insert(OnOverrideIconClicked);
		Red.m_OnClicked.Insert(OnColorTeamClicked);
		Blue.m_OnClicked.Insert(OnColorTeamClicked);
		Yellow.m_OnClicked.Insert(OnColorTeamClicked);
		Green.m_OnClicked.Insert(OnColorTeamClicked);
		None.m_OnClicked.Insert(OnColorTeamClicked);
	}
	
	void SetPlayerStr(string PlayerStr)
	{
		array<string> removeValueArray = {};
		PlayerStr.Split("|", removeValueArray, false);
		PlayerStr = removeValueArray[1];
				
		array<string> localPlayerStringSplit = {};
		PlayerStr.Split(";", localPlayerStringSplit, false);
		
		string SelectedPlayerIDStr = localPlayerStringSplit[3];
		SelectedPlayerID = SelectedPlayerIDStr.ToInt();
		
		UpdatePlayerIcon();
	}

	protected void OnColorTeamClicked(SCR_ButtonBaseComponent CTcomponent)
	{
		if (PlayerName.GetText() == "No Player Selected") return;
		string ColorTeam = CTcomponent.GetRootWidget().GetName();
		
		switch (ColorTeam)
		{
			case "Red"    : {PlayerName.SetColorInt(ARGB(255, 200, 65, 65));   Icon.SetColorInt(ARGB(255, 200, 65, 65));   break; };
			case "Blue"   : {PlayerName.SetColorInt(ARGB(255, 0, 92, 255));    Icon.SetColorInt(ARGB(255, 0, 92, 255));    break; };
			case "Yellow" : {PlayerName.SetColorInt(ARGB(255, 230, 230, 0));   Icon.SetColorInt(ARGB(255, 230, 230, 0));   break; };
			case "Green"  : {PlayerName.SetColorInt(ARGB(255, 0, 190, 85));    Icon.SetColorInt(ARGB(255, 0, 190, 85));    break; };
			case "None"   : {PlayerName.SetColorInt(ARGB(255, 255, 255, 255)); Icon.SetColorInt(ARGB(255, 255, 255, 255)); break; };
		};
		
		if (!SelectedPlayerID) return;
		groupBackendComponent.Owner_UpdatePlayerMapValue(SelectedPlayerID, "ColorTeam", ColorTeam);
		
		string iconOI = groupManagerCOA.ReturnMapValue("StoredIcon", SelectedPlayerID);
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
		groupBackendComponent.Owner_UpdatePlayerMapValue(SelectedPlayerID, "OverrideIcon", iconToOverrideStr);
		GetGame().GetCallqueue().CallLater(OnOverrideIconClickedSecondary, 225, true);
	}
	
	protected void OnOverrideIconClickedSecondary() {
		string iconOI = groupManagerCOA.ReturnMapValue("StoredIcon", SelectedPlayerID);
		Icon.LoadImageTexture(0, iconOI);
	};
	
	protected void UpdatePlayerIcon() 
	{
		groupBackendComponent = COA_GroupDisplayComponent.GetInstance();
		
		string PlayerString = groupManagerCOA.ReturnMapValue("PlayerGroupValues", SelectedPlayerID);
		
		array<string> removeValueArray = {};
		PlayerString.Split("|", removeValueArray, false);
		PlayerString = removeValueArray[1];
				
		array<string> localPlayerStringSplit = {};
		PlayerString.Split(";", localPlayerStringSplit, false);
		
		string playerNameOI        = localPlayerStringSplit[0];
		string ColorTeamOI         = localPlayerStringSplit[1];
		string iconOverride        = localPlayerStringSplit[4];
		
		string iconOI = groupManagerCOA.ReturnMapValue("StoredIcon", SelectedPlayerID);
		
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

		Icon.LoadImageTexture(0, iconOI);
		PlayerName.SetText(playerNameOI);

		if (iconOverride && iconOverride != "") {
			int playerOverideIcon = 0;
			switch (iconOverride)
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
