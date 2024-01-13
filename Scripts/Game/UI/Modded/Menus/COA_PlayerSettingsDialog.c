class COA_PlayerSettingsDialog : ChimeraMenuBase
{
	protected Widget m_wRoot;

	protected XComboBoxWidget m_wIconOveride;
	protected ImageWidget m_wIcon;
	protected TextWidget m_wPlayerName;
	
	protected int m_iSelectedPlayerID = 0;
	protected COA_GroupDisplayManagerComponent customGroupManager = null;
	protected SCR_AIGroup playersGroup = null;
	protected int m_iGroupID = 0;
	protected COA_GroupDisplayComponent groupBackendComponent = null;
	protected string m_sStoredSpecialtIcon = "";
	
	protected SCR_GroupsManagerComponent groupManager = null;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override void OnMenuOpen()
	{
		m_wRoot = GetRootWidget();

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		back.m_OnClicked.Insert(OnMenuBack);
		
		groupBackendComponent = COA_GroupDisplayComponent.GetInstance();

		// Get Global Player Controller and Group Manager.
		customGroupManager = COA_GroupDisplayManagerComponent.GetInstance();
		if (!customGroupManager || !groupBackendComponent) return;

		m_wIconOveride = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		
		GetGame().GetCallqueue().CallLater(UpdatePlayerIcon, 215, true);
		GetGame().GetCallqueue().CallLater(UpdateIconOverride, 145);
		
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
		
		groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup OpeningPlayersGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (OpeningPlayersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId())) ShowAdvSettings();
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions moddified by the Player Selection menu

	//------------------------------------------------------------------------------------------------
	
	void SetPlayerStr(string playerStringToSplit)
	{	
		array<string> playerSplitArray = {};
		playerStringToSplit.Split("╣", playerSplitArray, false);
		string playerIDString = playerSplitArray[1];
		
		m_iSelectedPlayerID = playerIDString.ToInt();
		
		playersGroup = groupManager.GetPlayerGroup(m_iSelectedPlayerID);
		m_iGroupID = playersGroup.GetGroupID();
		
		UpdatePlayerIcon();
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to update visual icons/text (player icon, player name, etc.)

	//------------------------------------------------------------------------------------------------
	
	protected void UpdatePlayerIcon() 
	{	
		string playerName   = GetGame().GetPlayerManager().GetPlayerName(m_iSelectedPlayerID);
		string colorTeam    = customGroupManager.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "ColorTeam");
		
		m_sStoredSpecialtIcon = customGroupManager.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "StoredSpecialtyIcon");
		
		if (colorTeam || colorTeam != "") {
			switch (colorTeam)
			{
				case "Red"    : {m_wPlayerName.SetColorInt(ARGB(255, 200, 65, 65));   m_wIcon.SetColorInt(ARGB(255, 200, 65, 65));   break;};
				case "Blue"   : {m_wPlayerName.SetColorInt(ARGB(255, 0, 92, 255));    m_wIcon.SetColorInt(ARGB(255, 0, 92, 255));    break;};
				case "Yellow" : {m_wPlayerName.SetColorInt(ARGB(255, 230, 230, 0));   m_wIcon.SetColorInt(ARGB(255, 230, 230, 0));   break;};
				case "Green"  : {m_wPlayerName.SetColorInt(ARGB(255, 0, 190, 85));    m_wIcon.SetColorInt(ARGB(255, 0, 190, 85));    break;};
				case "None"   : {m_wPlayerName.SetColorInt(ARGB(255, 235, 235, 235)); m_wIcon.SetColorInt(ARGB(255, 235, 235, 235)); break;};
			};
		};

		m_wIcon.LoadImageTexture(0, m_sStoredSpecialtIcon);
		m_wPlayerName.SetText(playerName);
	}
	
	protected void UpdateIconOverride() 
	{
		string iconOverride = customGroupManager.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon");
	
		int playerOverideIcon = 0;
		
		switch (iconOverride)
		{
			case "Medic"          : {playerOverideIcon = 1; break;};
			case "Sniper"         : {playerOverideIcon = 2; break;};
			case "Machine Gunner" : {playerOverideIcon = 3; break;};
			case "Anti-Tank"      : {playerOverideIcon = 4; break;};
			case "Grenadier"      : {playerOverideIcon = 5; break;};
			case "Man"            : {playerOverideIcon = 6; break;};
		};
		
		m_wIconOveride.SetCurrentItem(playerOverideIcon);
	};
	
	//------------------------------------------------------------------------------------------------

	// Additional menu functions

	//------------------------------------------------------------------------------------------------
	
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseMenu(this);
	}
	
	protected void ShowAdvSettings()
	{
		for (int b = 0; b <= 12; b++)
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
	
	//------------------------------------------------------------------------------------------------

	// Button functions

	//------------------------------------------------------------------------------------------------
	
	protected void OnColorTeamClicked(SCR_ButtonBaseComponent CTcomponent)
	{
		if (m_wPlayerName.GetText() == "No Player Selected") return;
		string colorTeam = CTcomponent.GetRootWidget().GetName();
		
		if (!m_iSelectedPlayerID) return;
		groupBackendComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "ColorTeam", colorTeam);
		
		m_sStoredSpecialtIcon = customGroupManager.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "StoredSpecialtyIcon");
	}

	protected void OnOverrideIconClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || playersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		if (m_sStoredSpecialtIcon == "{6D45BA2CCC322312}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds") return;
		
		int iconToOverride = m_wIconOveride.GetCurrentItem();
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

		if (!m_iSelectedPlayerID) return;
		groupBackendComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", iconToOverrideStr);
	}
	
	protected void OnPromoteToSLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || playersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		groupBackendComponent.Owner_PromotePlayerToSL(m_iSelectedPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};
	
	protected void OnPromoteToTLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || playersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		if (m_sStoredSpecialtIcon == "{6D45BA2CCC322312}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds") {
			groupBackendComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", "Auto");
			return;
		};
		groupBackendComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", "Team Lead");
	};
	
	protected void OnKickClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || playersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		groupBackendComponent.Owner_RemovePlayerFromGroup(m_iSelectedPlayerID);
		GetGame().GetCallqueue().CallLater(OnMenuBack, 265);
	};
}
