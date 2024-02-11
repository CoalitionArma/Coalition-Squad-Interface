class CSI_PlayerSettingsDialog : ChimeraMenuBase
{	
	protected string m_sCTRed    = ARGB(255, 200, 65, 65).ToString();
	protected string m_sCTBlue   = ARGB(255, 0, 92, 255).ToString();
	protected string m_sCTYellow = ARGB(255, 230, 230, 0).ToString();
	protected string m_sCTGreen  = ARGB(255, 0, 190, 85).ToString();
	protected string m_sCTNone   = ARGB(255, 215, 215, 215).ToString();
	
	protected SCR_AIGroup m_PlayersGroup = null;
	protected CSI_ClientComponent m_ClientComponent = null;
	protected CSI_AuthorityComponent m_AuthorityComponent = null;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent = null;
	
	protected Widget m_wRoot;
	protected XComboBoxWidget m_wIconOveride;
	protected ImageWidget m_wIcon;
	protected TextWidget m_wPlayerName;
	
	protected int m_iSelectedPlayerID;
	protected int m_iGroupID;
	protected string m_sStoredSpecialtIcon;
	protected string m_sRankVisible;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Cancel").FindHandler(SCR_InputButtonComponent));
		cancel.m_OnClicked.Insert(OnMenuBack);
		
		m_ClientComponent = CSI_ClientComponent.GetInstance();

		// Get Global Player Controller and Group Manager.
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		if (!m_AuthorityComponent || !m_ClientComponent) return;

		m_wIconOveride = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		
		GetGame().GetCallqueue().CallLater(UpdatePlayerIcon, 215, true);
		GetGame().GetCallqueue().CallLater(UpdateIconOverride, 145);
		
		SCR_ModularButtonComponent confirmIOButton = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmIOButton").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent red             = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Red").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent blue            = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Blue").FindHandler(SCR_ModularButtonComponent));		
		SCR_ModularButtonComponent yellow          = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Yellow").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent green           = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Green").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent none            = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("None").FindHandler(SCR_ModularButtonComponent));
		
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
		playerStringToSplit.Split("«╣║╢║»", playerSplitArray, false);
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
		string colorTeam  = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "ColorTeam");
		
		m_sStoredSpecialtIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "StoredSpecialtyIcon"); 
		
		string rankVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[5];
		switch (true)
		{				
			case(rankVisibleSO == "true" || rankVisibleSO == "false") : { m_sRankVisible = rankVisibleSO; break; };
			default : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("rankVisible", m_sRankVisible); break; };
		};
		
		if (m_sRankVisible == "true") {
			string rank = SCR_CharacterRankComponent.GetCharacterRankNameShort(GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iSelectedPlayerID));
			playerName = string.Format("%1 %2", rank, playerName);
		};
		 
		m_wIcon.SetColorInt(colorTeam.ToInt());
		m_wIcon.LoadImageTexture(0, m_sStoredSpecialtIcon);
		m_wPlayerName.SetColorInt(colorTeam.ToInt());   
		m_wPlayerName.SetText(playerName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateIconOverride() 
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		string iconOverride = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon");
	
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
		promoteToSL.SetEnabled(true);
		promoteToTL.SetEnabled(true);
		kick.SetEnabled(true);
		promoteToSL.SetOpacity(1);
		promoteToTL.SetOpacity(1);
		kick.SetOpacity(1);
		
		SCR_ModularButtonComponent promoteToSLComp = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("PromoteToSL").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent promoteToTLComp = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("PromoteToTL").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent kickComp = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Kick").FindHandler(SCR_ModularButtonComponent));
		
		promoteToSLComp.m_OnClicked.Insert(OnPromoteToSLClicked);
		promoteToTLComp.m_OnClicked.Insert(OnPromoteToTLClicked);
		kickComp.m_OnClicked.Insert(OnKickClicked);
	};
	
	//------------------------------------------------------------------------------------------------

	// Button functions

	//------------------------------------------------------------------------------------------------
	
	protected void OnColorTeamClicked(SCR_ModularButtonComponent CTcomponent)
	{
		if (m_wPlayerName.GetText() == "No Player Selected") return;
		string colorTeamButtonName = CTcomponent.GetRootWidget().GetName();
		
		if (!m_iSelectedPlayerID) return;
		
		string colorTeamIntStr = m_sCTNone;
		if (colorTeamButtonName || colorTeamButtonName != "") {
			switch (colorTeamButtonName)
			{
				case "Red"    : {colorTeamIntStr = m_sCTRed;    break;};
				case "Blue"   : {colorTeamIntStr = m_sCTBlue;   break;};
				case "Yellow" : {colorTeamIntStr = m_sCTYellow; break;};
				case "Green"  : {colorTeamIntStr = m_sCTGreen;  break;};
				case "None"   : {colorTeamIntStr = m_sCTNone;   break;};
			};
		};
		
		m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "ColorTeam", colorTeamIntStr);
		
		m_sStoredSpecialtIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "StoredSpecialtyIcon");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOverrideIconClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		if (m_sStoredSpecialtIcon == "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds") return;
		
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
		m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", iconToOverrideStr);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToSLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		m_ClientComponent.Owner_PromotePlayerToSL(m_iSelectedPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};
	
	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToTLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		if (m_sStoredSpecialtIcon == "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds") {
			m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", "Auto");
			return;
		};
		m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OverrideIcon", "Team Lead");
	};
	
	//------------------------------------------------------------------------------------------------
	protected void OnKickClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) return;
		
		m_ClientComponent.Owner_RemovePlayerFromGroup(m_iSelectedPlayerID);
		GetGame().GetCallqueue().CallLater(OnMenuBack, 265);
	};
}
