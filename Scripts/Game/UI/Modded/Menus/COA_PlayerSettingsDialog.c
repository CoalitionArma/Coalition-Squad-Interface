class COA_PlayerSettingsDialog : ChimeraMenuBase
{	
	protected SCR_AIGroup m_PlayersGroup = null;
	protected COA_GroupDisplayManagerComponent m_GroupDisplayManagerComponent = null;
	protected COA_GroupDisplayComponent m_GroupDisplayComponent = null;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent = null;
	
	private Widget m_wRoot;
	private XComboBoxWidget m_wIconOveride;
	private ImageWidget m_wIcon;
	private TextWidget m_wPlayerName;
	
	private int m_iSelectedPlayerID;
	private int m_iGroupID;
	private string m_sStoredSpecialtIcon;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override void OnMenuOpen()
	{
		super.OnMenuShow();
		
		m_wRoot = GetRootWidget();

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		back.m_OnClicked.Insert(OnMenuBack);
		
		m_GroupDisplayComponent = COA_GroupDisplayComponent.GetInstance();

		// Get Global Player Controller and Group Manager.
		m_GroupDisplayManagerComponent = COA_GroupDisplayManagerComponent.GetInstance();
		if (!m_GroupDisplayManagerComponent || !m_GroupDisplayComponent) return;

		m_wIconOveride = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		
		GetGame().GetCallqueue().CallLater(UpdatePlayerIcon, 215, true);
		GetGame().GetCallqueue().CallLater(UpdateIconOverride, 145);
		
		SCR_InputButtonComponent confirmIOButton = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmIOButton").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent red             = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Red").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent blue            = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Blue").FindHandler(SCR_InputButtonComponent));		
		SCR_InputButtonComponent yellow          = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Yellow").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent green           = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Green").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent none            = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("None").FindHandler(SCR_InputButtonComponent));
		
		confirmIOButton.m_OnClicked.Insert(OnOverrideIconClicked);
		red.m_OnClicked.Insert(OnColorTeamClicked);
		blue.m_OnClicked.Insert(OnColorTeamClicked);
		yellow.m_OnClicked.Insert(OnColorTeamClicked);
		green.m_OnClicked.Insert(OnColorTeamClicked);
		none.m_OnClicked.Insert(OnColorTeamClicked);
		
		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup openingPlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (openingPlayersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId())) ShowAdvSettings();
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
		
		m_PlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(m_iSelectedPlayerID);
		m_iGroupID = m_PlayersGroup.GetGroupID();
		
		UpdatePlayerIcon();
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to update visual icons/text (player icon, player name, etc.)

	//------------------------------------------------------------------------------------------------
	
	protected void UpdatePlayerIcon() 
	{	
		string playerName = GetGame().GetPlayerManager().GetPlayerName(m_iSelectedPlayerID);
		string colorTeam  = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "ColorTeam");
		
		m_sStoredSpecialtIcon = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "StoredSpecialtyIcon");
		
		if (colorTeam || colorTeam != "") {
			switch (colorTeam)
			{
				case "Red"    : {m_wPlayerName.SetColorInt(ARGB(255, 200, 65, 65));   m_wIcon.SetColorInt(ARGB(255, 200, 65, 65));   break;};
				case "Blue"   : {m_wPlayerName.SetColorInt(ARGB(255, 0, 92, 255));    m_wIcon.SetColorInt(ARGB(255, 0, 92, 255));    break;};
				case "Yellow" : {m_wPlayerName.SetColorInt(ARGB(255, 230, 230, 0));   m_wIcon.SetColorInt(ARGB(255, 230, 230, 0));   break;};
				case "Green"  : {m_wPlayerName.SetColorInt(ARGB(255, 0, 190, 85));    m_wIcon.SetColorInt(ARGB(255, 0, 190, 85));    break;};
				case "None"   : {m_wPlayerName.SetColorInt(ARGB(255, 215, 215, 215)); m_wIcon.SetColorInt(ARGB(255, 215, 215, 215)); break;};
			};
		};

		m_wIcon.LoadImageTexture(0, m_sStoredSpecialtIcon);
		m_wPlayerName.SetText(playerName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateIconOverride() 
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		string iconOverride = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon");
	
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
		GetGame().GetCallqueue().Remove(UpdatePlayerIcon);
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseMenu(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowAdvSettings()
	{
		for (int b = 0; b <= 12; b++)
		{
			ImageWidget Background = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Background%1", b)));
			Background.SetOpacity(1);
		};
		
		TextWidget promoteToSLText = TextWidget.Cast(m_wRoot.FindAnyWidget("PromoteToSLText"));
		TextWidget promoteToTLText = TextWidget.Cast(m_wRoot.FindAnyWidget("PromoteToTLText"));
		TextWidget kickText  = TextWidget.Cast(m_wRoot.FindAnyWidget("KickText"));
		
		promoteToSLText.SetOpacity(1);
		promoteToTLText.SetOpacity(1);
		kickText.SetOpacity(1);
		
		TextWidget prettyText0 = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyText0"));
		ImageWidget pretty0 = ImageWidget.Cast(m_wRoot.FindAnyWidget("Pretty0"));
		
		prettyText0.SetOpacity(1);
		pretty0.SetOpacity(1);
		
		ImageWidget promoteToSLIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("PromoteToSLIcon"));
		ImageWidget promoteToTLIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("PromoteToTLIcon"));
		ImageWidget kickIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("KickIcon"));
		
		promoteToSLIcon.SetOpacity(1);
		promoteToTLIcon.SetOpacity(1);
		kickIcon.SetOpacity(1);
		
		ButtonWidget promoteToSL = ButtonWidget.Cast(m_wRoot.FindAnyWidget("PromoteToSL"));
		ButtonWidget promoteToTL = ButtonWidget.Cast(m_wRoot.FindAnyWidget("PromoteToTL"));
		ButtonWidget kick = ButtonWidget.Cast(m_wRoot.FindAnyWidget("Kick"));
		
		promoteToSL.SetOpacity(1);
		promoteToTL.SetOpacity(1);
		kick.SetOpacity(1);
		
		SCR_InputButtonComponent promoteToSLComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("PromoteToSL").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent promoteToTLComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("PromoteToTL").FindHandler(SCR_InputButtonComponent));
		SCR_InputButtonComponent kickComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Kick").FindHandler(SCR_InputButtonComponent));
		
		promoteToSLComp.m_OnClicked.Insert(OnPromoteToSLClicked);
		promoteToTLComp.m_OnClicked.Insert(OnPromoteToTLClicked);
		kickComp.m_OnClicked.Insert(OnKickClicked);
	};
	
	//------------------------------------------------------------------------------------------------

	// Button functions

	//------------------------------------------------------------------------------------------------
	
	protected void OnColorTeamClicked(SCR_ButtonBaseComponent CTcomponent)
	{
		if (m_wPlayerName.GetText() == "No Player Selected") return;
		string colorTeam = CTcomponent.GetRootWidget().GetName();
		
		if (!m_iSelectedPlayerID) return;
		m_GroupDisplayComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "ColorTeam", colorTeam);
		
		m_sStoredSpecialtIcon = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "StoredSpecialtyIcon");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOverrideIconClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		if (m_sStoredSpecialtIcon == "{6D45BA2CCC322312}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds") return;
		
		int iconToOverride = m_wIconOveride.GetCurrentItem();
		string iconToOverrideStr = "";
		switch (iconToOverride)
		{
			case 0 : {iconToOverrideStr = "Auto";           break;};
			case 1 : {iconToOverrideStr = "Medic";          break;};
			case 2 : {iconToOverrideStr = "Sniper";         break;};
			case 3 : {iconToOverrideStr = "Machine Gunner"; break;};
			case 4 : {iconToOverrideStr = "Anti-Tank";      break;};
			case 5 : {iconToOverrideStr = "Grenadier";      break;};
			case 6 : {iconToOverrideStr = "Man";            break;};
		};

		if (!m_iSelectedPlayerID) return;
		m_GroupDisplayComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", iconToOverrideStr);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToSLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		m_GroupDisplayComponent.Owner_PromotePlayerToSL(m_iSelectedPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};
	
	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToTLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		if (m_sStoredSpecialtIcon == "{6D45BA2CCC322312}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds") {
			m_GroupDisplayComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", "Auto");
			return;
		};
		m_GroupDisplayComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", "Team Lead");
	};
	
	//------------------------------------------------------------------------------------------------
	protected void OnKickClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		m_GroupDisplayComponent.Owner_RemovePlayerFromGroup(m_iSelectedPlayerID);
		GetGame().GetCallqueue().CallLater(OnMenuBack, 265);
	};
}
