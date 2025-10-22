class CSI_PlayerSettingsDialog : ChimeraMenuBase
{
	protected SCR_AIGroup m_PlayersGroup;
	protected CSI_ClientComponent m_ClientComponent;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	protected Widget m_wRoot;
	protected XComboBoxWidget m_wIconOveride;
	protected ImageWidget m_wIcon;
	protected TextWidget m_wPlayerName;

	protected int m_iSelectedPlayerID;
	protected int m_iGroupID;
	protected string m_sStoredSpecialtyIcon;

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
		if (!m_AuthorityComponent || !m_ClientComponent) 
			return;

		m_wIconOveride = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));

		GetGame().GetCallqueue().CallLater(UpdatePlayerIcon, 215, true);
		GetGame().GetCallqueue().CallLater(UpdateIconOverride, 145);

		SCR_ModularButtonComponent confirmIOButton = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmIOButton").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent red = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Red").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent blue = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Blue").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent yellow = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Yellow").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent green = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("Green").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent none = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("None").FindHandler(SCR_ModularButtonComponent));

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
		playerStringToSplit.Split(":", playerSplitArray, false);
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
		string colorTeamString = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "CT"); // CT = ColorTeam

		m_sStoredSpecialtyIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "SSI"); // SSI = StoredSpecialtyIcon
	
		if (m_sStoredSpecialtyIcon.IsEmpty()) 
			return;

		string rankVisible = m_ClientComponent.ReturnLocalCSISettings()[5];

		if (rankVisible == "true") 
		{
			string rank = m_AuthorityComponent.ReturnLocalPlayerMapValue(-1, m_iSelectedPlayerID, "PR"); // PR = PlayerRank
			if (!rank.IsEmpty()) 
				playerName = string.Format("%1 %2", rank, playerName);
		};

		m_wIcon.LoadImageTexture(0, m_ClientComponent.SwitchStringToIcon(m_sStoredSpecialtyIcon));
		m_wIcon.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));
		m_wPlayerName.SetText(playerName);
		m_wPlayerName.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateIconOverride()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) 
			return;

		string iconOverride = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OI"); // OI = OverrideIcon

		if (iconOverride.IsEmpty()) 
			return;

		int playerOverideIcon = 0;
		switch (iconOverride)
		{
			case "MED" : {playerOverideIcon = 1; break;};
			case "MRK" : {playerOverideIcon = 2; break;};
			case "MG"  : {playerOverideIcon = 3; break;};
			case "AT"  : {playerOverideIcon = 4; break;};
			case "GRN" : {playerOverideIcon = 5; break;};
			case "EXP" : {playerOverideIcon = 6; break;};
			case "ENG" : {playerOverideIcon = 7; break;};
			case "MAN" : {playerOverideIcon = 8; break;};
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
		TextWidget kickText = TextWidget.Cast(m_wRoot.FindAnyWidget("KickText"));

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
		if (m_wPlayerName.GetText() == "No Player Selected") 
			return;
		string colorTeamButtonName = CTcomponent.GetRootWidget().GetName();

		if (!m_iSelectedPlayerID) 
			return;
		
		if (colorTeamButtonName == "None") 
		{
			m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "CT", "N/A"); // CT = ColorTeam
			return;
		}

		m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "CT", colorTeamButtonName[0]); // CT = ColorTeam
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOverrideIconClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID) || m_sStoredSpecialtyIcon == "FTL") 
			return;

		int iconToOverride = m_wIconOveride.GetCurrentItem();
		string iconToOverrideStr = "";
		switch (iconToOverride)
		{
			case 1 : {iconToOverrideStr = "MED";  break;};
			case 2 : {iconToOverrideStr = "MRK";  break;};
			case 3 : {iconToOverrideStr = "MG";   break;};
			case 4 : {iconToOverrideStr = "AT";   break;};
			case 5 : {iconToOverrideStr = "GRN";  break;};
			case 6 : {iconToOverrideStr = "EXP";  break;};
			case 7 : {iconToOverrideStr = "ENG";  break;};
			case 8 : {iconToOverrideStr = "MAN";  break;};
		};

		if (!m_iSelectedPlayerID) 
			return;
		
		if (iconToOverrideStr.IsEmpty()) 
		{
			m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OI", "N/A"); // OI = OverrideIcon
			return;
		}
		
		m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OI", iconToOverrideStr); // OI = OverrideIcon
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToSLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) 
			return;

		m_ClientComponent.Owner_PromotePlayerToSL(m_iSelectedPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};

	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToTLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) 
			return;

		if (m_sStoredSpecialtyIcon == "FTL") 
		{
			m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OI", "N/A"); // OI = OverrideIcon
			return;
		};
		m_ClientComponent.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OI", "FTL"); // OI = OverrideIcon
	};

	//------------------------------------------------------------------------------------------------
	protected void OnKickClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) 
			return;

		m_ClientComponent.Owner_RemovePlayerFromGroup(m_iSelectedPlayerID);
		GetGame().GetCallqueue().CallLater(OnMenuBack, 265);
	};
}
