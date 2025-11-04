class CSI_PlayerSettings : SCR_ScriptedWidgetComponent
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_RplToAuthorityManagerClass m_RplToAuthorityManagerClass;
	
	/*
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_RplToAuthorityManagerClass = CSI_RplToAuthorityManagerClass.GetInstance();
		
		m_wIconOveride = XComboBoxWidget.Cast(w.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		SCR_ModularButtonComponent confirmIOButton = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("ConfirmIOButton").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent red = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("Red").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent blue = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("Blue").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent yellow = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("Yellow").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent green = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("Green").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent none = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("None").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent promoteToSLComp = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("PromoteToSL").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent promoteToTLComp = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("PromoteToTL").FindHandler(SCR_ModularButtonComponent));
		SCR_ModularButtonComponent kickComp = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("Kick").FindHandler(SCR_ModularButtonComponent));

		promoteToSLComp.m_OnClicked.Insert(OnPromoteToSLClicked);
		promoteToTLComp.m_OnClicked.Insert(OnPromoteToTLClicked);
		kickComp.m_OnClicked.Insert(OnKickClicked);
		confirmIOButton.m_OnClicked.Insert(OnOverrideIconClicked);
		red.m_OnClicked.Insert(OnColorTeamClicked);
		blue.m_OnClicked.Insert(OnColorTeamClicked);
		yellow.m_OnClicked.Insert(OnColorTeamClicked);
		green.m_OnClicked.Insert(OnColorTeamClicked);
		none.m_OnClicked.Insert(OnColorTeamClicked);
	}

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
	protected void UpdatePlayerIcon()
	{
		string playerName = GetGame().GetPlayerManager().GetPlayerName(m_iSelectedPlayerID);
		string colorTeamString = m_PlayerDataManager.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "CT"); // CT = ColorTeam

		m_sStoredSpecialtyIcon = m_PlayerDataManager.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "SSI"); // SSI = StoredSpecialtyIcon
	
		if (m_sStoredSpecialtyIcon.IsEmpty()) 
			return;

		string rankVisible = m_ClientComponent.ReturnLocalCSISettings()[5];

		if (rankVisible == "true") 
		{
			string rank = m_PlayerDataManager.ReturnLocalPlayerMapValue(-1, m_iSelectedPlayerID, "PR"); // PR = PlayerRank
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

		string iconOverride = m_PlayerDataManager.ReturnLocalPlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OI"); // OI = OverrideIcon

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
	protected void OnMenuBack()
	{
		GetGame().GetCallqueue().Remove(UpdatePlayerIcon);
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseMenu(this);
	}

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
		if (m_wPlayerName.GetText() == "No Player Selected") 
			return;

		CSI_EOverrideIcon iconToOverride = m_wIconOveride.GetCurrentItem();

		if (!m_iSelectedPlayerID) 
			return;
		
		m_RplToAuthorityManagerClass.Owner_UpdatePlayerOverrideIcon(m_iSelectedPlayerID, iconToOverride);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToSLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) 
			return;

		m_RplToAuthorityManagerClass.Owner_PromotePlayerToSL(m_iSelectedPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};

	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToTLClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) 
			return;

		if (m_sStoredSpecialtyIcon == "FTL") 
		{
			m_RplToAuthorityManagerClass.Owner_UpdatePlayerMapValue(m_iGroupID, m_iSelectedPlayerID, "OI", "N/A"); // OI = OverrideIcon
			return;
		};
		m_RplToAuthorityManagerClass.Owner_UpdatePlayerTeamLeader(m_iSelectedPlayerID);
	};

	//------------------------------------------------------------------------------------------------
	protected void OnKickClicked()
	{
		if (m_wPlayerName.GetText() == "No Player Selected" || m_PlayersGroup.IsPlayerLeader(m_iSelectedPlayerID)) 
			return;

		m_SettingsManager.Owner_RemovePlayerFromGroup(m_iSelectedPlayerID);
		GetGame().GetCallqueue().CallLater(OnMenuBack, 265);
	};
	*/
}
