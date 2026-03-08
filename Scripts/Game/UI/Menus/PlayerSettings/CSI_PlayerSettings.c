 class CSI_PlayerSettings : SCR_ScriptedWidgetComponent
{
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_RplToAuthoritySystem m_RplToAuthoritySystem;

	protected XComboBoxWidget m_wIconOveride;
	protected TextWidget m_wPlayerName;
	protected ImageWidget m_wIcon;
	protected int m_iPlayerID = -1;
	protected ScriptInvoker m_PlayerUpdate;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_RplToAuthoritySystem = CSI_RplToAuthoritySystem.GetInstance();
		
		m_wIconOveride = XComboBoxWidget.Cast(w.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));

		SCR_ModularButtonComponent confirmIOButton = SCR_ModularButtonComponent.Cast(w.FindAnyWidget("ConfirmOverrideButton").FindHandler(SCR_ModularButtonComponent));
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
	void UpdatePlayerSettingsPlayerID(int playerID)
	{
		m_iPlayerID = playerID;

		if (playerID <= 0)
			return;
		else
			UpdatePlayerSettingsInformation();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerSettingsInformation()
	{
		if (m_iPlayerID <= 0)
			return;
		
		if (m_PlayerUpdate)
		{
			m_PlayerUpdate.Remove(UpdatePlayerSettingsInformation);
			m_PlayerUpdate = null;
		};
		
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(m_iPlayerID);
		
		m_PlayerUpdate = playerData.GetOnDataUpdate();

		if (m_PlayerUpdate)
			m_PlayerUpdate.Insert(UpdatePlayerSettingsInformation);
		
		CSI_EOverrideIcon iconOverride = playerData.GetOverrideIcon();
		CSI_EColorTeam colorTeam = playerData.GetColorTeam();
		CSI_EIcon displayIcon = playerData.GetDisplayIcon();
		string displayIconStr = CSI_UIHelper.GetIconString(displayIcon, true);

		if (iconOverride != CSI_EOverrideIcon.AUTO) 
			m_wIconOveride.SetCurrentItem(iconOverride - 3);

		m_wIcon.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));
		
		if (displayIconStr == "MAN_ICON")
			m_wIcon.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
		else
           	m_wIcon.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, displayIconStr);

		m_wPlayerName.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));
		m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_iPlayerID));
	};

	//------------------------------------------------------------------------------------------------
	protected void OnColorTeamClicked(SCR_ModularButtonComponent CTcomponent)
	{
		if (m_iPlayerID <= 0)
			return;

		CSI_EColorTeam colorTeam;
		string colorTeamButtonName = CTcomponent.GetRootWidget().GetName();
		
		switch (colorTeamButtonName)
		{
			case "Red" : colorTeam = CSI_EColorTeam.RED; break;
			case "Blue" : colorTeam = CSI_EColorTeam.BLUE; break;
			case "Yellow" : colorTeam = CSI_EColorTeam.YELLOW; break;
			case "Green" : colorTeam = CSI_EColorTeam.GREEN; break;
			default : colorTeam = CSI_EColorTeam.NONE;
		}

		m_RplToAuthoritySystem.Owner_UpdatePlayerColorTeam(m_iPlayerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOverrideIconClicked()
	{
		if (m_iPlayerID <= 0)
			return;

		CSI_EOverrideIcon iconToOverride = m_wIconOveride.GetCurrentItem();

		if (iconToOverride > 0)
			iconToOverride = iconToOverride + 3;
		
		m_RplToAuthoritySystem.Owner_UpdatePlayerOverrideIcon(m_iPlayerID, iconToOverride);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToSLClicked()
	{
		if (m_iPlayerID <= 0 && SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId()).IsPlayerLeader(m_iPlayerID))
			return;

		m_RplToAuthoritySystem.Owner_PromotePlayerToSL(m_iPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};

	//------------------------------------------------------------------------------------------------
	protected void OnPromoteToTLClicked()
	{
		if (m_iPlayerID <= 0)
			return;

		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(m_iPlayerID);
		m_RplToAuthoritySystem.Owner_UpdatePlayerTeamLeader(m_iPlayerID, !playerData.GetIsTeamLeader());
	};

	//------------------------------------------------------------------------------------------------
	protected void OnKickClicked()
	{
		if (m_iPlayerID <= 0 && SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId()).IsPlayerLeader(m_iPlayerID))
			return;

		m_RplToAuthoritySystem.Owner_RemovePlayerFromGroup(m_iPlayerID);
	};
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_PlayerUpdate)
		{
			m_PlayerUpdate.Remove(UpdatePlayerSettingsInformation);
			m_PlayerUpdate = null;
		};
	}
}
