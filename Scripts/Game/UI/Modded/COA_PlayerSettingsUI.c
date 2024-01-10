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
	protected string storedIcon = "";
	
	protected SCR_GroupsManagerComponent vanillaGroupManager = null;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_wRoot = GetRootWidget();
		
		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent Back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		Back.m_OnClicked.Insert(OnMenuBack);
		
		groupBackendComponent = COA_GroupDisplayComponent.GetInstance();

		// Get Global Player Controller and Group Manager.
		groupManagerCOA = COA_GroupDisplayManagerComponent.GetInstance();
		if (!groupManagerCOA || !groupBackendComponent) return;

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
		
		vanillaGroupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup OpeningPlayersGroup = vanillaGroupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (OpeningPlayersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId())) ShowAdvSettings();
	}
	
	void ShowAdvSettings()
	{
		
		for (int b = 0; b < 12; b++)
		{
			ImageWidget Background = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Background%1", b)));
			Background.SetOpacity(1);
		};
		
		TextWidget PromoteToSLText = TextWidget.Cast(m_wRoot.FindAnyWidget("PromoteToSLText"));
		TextWidget PromoteToTLText = TextWidget.Cast(m_wRoot.FindAnyWidget("PromoteToTLText"));
		TextWidget KickText  = TextWidget.Cast(m_wRoot.FindAnyWidget("KickText"));
		
		PromoteToSLText.SetOpacity(1);
		PromoteToTLText.SetOpacity(1);
		KickText.SetOpacity(1);
		
		TextWidget PrettyText0 = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyText0"));
		ImageWidget Pretty0 = ImageWidget.Cast(m_wRoot.FindAnyWidget("Pretty0"));
		
		PrettyText0.SetOpacity(1);
		Pretty0.SetOpacity(1);
		
		ImageWidget PromoteToSLIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("PromoteToSLIcon"));
		ImageWidget PromoteToTLIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("PromoteToTLIcon"));
		ImageWidget KickIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("KickIcon"));
		
		PromoteToSLIcon.SetOpacity(1);
		PromoteToTLIcon.SetOpacity(1);
		KickIcon.SetOpacity(1);
		
		ButtonWidget PromoteToSL = ButtonWidget.Cast(m_wRoot.FindAnyWidget("PromoteToSL"));
		ButtonWidget PromoteToTL = ButtonWidget.Cast(m_wRoot.FindAnyWidget("PromoteToTL"));
		ButtonWidget Kick = ButtonWidget.Cast(m_wRoot.FindAnyWidget("Kick"));
		
		PromoteToSL.SetOpacity(1);
		PromoteToTL.SetOpacity(1);
		Kick.SetOpacity(1);
		
		SCR_InputButtonComponent PromoteToSLComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("PromoteToSL").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent PromoteToTLComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("PromoteToTL").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent KickComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Kick").FindHandler(SCR_InputButtonComponent));
		
		PromoteToSLComp.m_OnClicked.Insert(OnPromoteToSLClicked);
		PromoteToTLComp.m_OnClicked.Insert(OnPromoteToTLClicked);
		KickComp.m_OnClicked.Insert(OnKickClicked);
	};
	
	
	void OnPromoteToSLClicked()
	{
		if (playersGroup.IsPlayerLeader(SelectedPlayerID)) return;
		
		groupBackendComponent.Owner_PromotePlayerToSL(SelectedPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};
	
	void OnPromoteToTLClicked()
	{
		if (playersGroup.IsPlayerLeader(SelectedPlayerID)) return;
		
		if (storedIcon == "{6D45BA2CCC322312}Layouts\UI\Textures\Icons\iconmanteamleader_ca.edds") {
			groupBackendComponent.Owner_UpdatePlayerMapValue(SelectedPlayerID, "OverrideIcon", "Auto");
			GetGame().GetCallqueue().CallLater(OnOverrideIconClickedSecondary, 325, true);
			return;
		};
		groupBackendComponent.Owner_UpdatePlayerMapValue(SelectedPlayerID, "OverrideIcon", "Team Lead");
		GetGame().GetCallqueue().CallLater(OnOverrideIconClickedSecondary, 325, true);
	};
	
	void OnKickClicked()
	{
		if (playersGroup.IsPlayerLeader(SelectedPlayerID)) return;
		
		groupBackendComponent.Owner_RemovePlayerFromGroup(SelectedPlayerID);
		OnMenuBack();
	};
	
	void SetPlayerStr(string PlayerStr)
	{
		array<string> removeValueArray = {};
		PlayerStr.Split("╣", removeValueArray, false);
		PlayerStr = removeValueArray[1];
				
		array<string> localPlayerStringSplit = {};
		PlayerStr.Split("║", localPlayerStringSplit, false);
		
		string SelectedPlayerIDStr = localPlayerStringSplit[3];
		SelectedPlayerID = SelectedPlayerIDStr.ToInt();
		
		playersGroup = vanillaGroupManager.GetPlayerGroup(SelectedPlayerID);
		
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
		
		storedIcon = groupManagerCOA.ReturnLocalPlayerMapValue("StoredIcon", SelectedPlayerID);
	}

	protected void OnOverrideIconClicked()
	{
		if (PlayerName.GetText() == "No Player Selected") return;
		
		if (storedIcon == "{6D45BA2CCC322312}Layouts\UI\Textures\Icons\iconmanteamleader_ca.edds") return;
		
		int iconToOverride = IconOveride.GetCurrentItem();
		string iconToOverrideStr = "";
		switch (iconToOverride)
		{
			case 0 : {iconToOverrideStr = "Auto";           break; };
			case 1 : {iconToOverrideStr = "Medic";          break; };
			case 2 : {iconToOverrideStr = "Sniper";         break; };
			case 3 : {iconToOverrideStr = "Machine Gunner"; break; };
			case 4 : {iconToOverrideStr = "Anti-Tank";      break; };
			case 5 : {iconToOverrideStr = "Grenadier";      break; };
			case 6 : {iconToOverrideStr = "Man";            break; };
		};

		if (!SelectedPlayerID) return;
		groupBackendComponent.Owner_UpdatePlayerMapValue(SelectedPlayerID, "OverrideIcon", iconToOverrideStr);
		GetGame().GetCallqueue().CallLater(OnOverrideIconClickedSecondary, 325, true);
	}
	
	protected void OnOverrideIconClickedSecondary() {
		storedIcon = groupManagerCOA.ReturnLocalPlayerMapValue("StoredIcon", SelectedPlayerID);
		Icon.LoadImageTexture(0, storedIcon);
	};
	
	protected void UpdatePlayerIcon() 
	{	
		string PlayerString = groupManagerCOA.ReturnLocalPlayerMapValue("PlayerGroupValues", SelectedPlayerID);
		
		array<string> removeValueArray = {};
		PlayerString.Split("╣", removeValueArray, false);
		PlayerString = removeValueArray[1];
				
		array<string> localPlayerStringSplit = {};
		PlayerString.Split("║", localPlayerStringSplit, false);
		
		string playerNameOI        = localPlayerStringSplit[0];
		string ColorTeamOI         = localPlayerStringSplit[1];
		string iconOverride        = localPlayerStringSplit[4];
		
		storedIcon = groupManagerCOA.ReturnLocalPlayerMapValue("StoredIcon", SelectedPlayerID);
		
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

		Icon.LoadImageTexture(0, storedIcon);
		PlayerName.SetText(playerNameOI);

		if (iconOverride && iconOverride != "") {
			int playerOverideIcon = 0;
			switch (iconOverride)
			{
				case "Medic"          : {playerOverideIcon = 1; break; };
				case "Sniper"         : {playerOverideIcon = 2; break; };
				case "Machine Gunner" : {playerOverideIcon = 3; break; };
				case "Anti-Tank"      : {playerOverideIcon = 4; break; };
				case "Grenadier"      : {playerOverideIcon = 5; break; };
				case "Man"            : {playerOverideIcon = 6; break; };
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
