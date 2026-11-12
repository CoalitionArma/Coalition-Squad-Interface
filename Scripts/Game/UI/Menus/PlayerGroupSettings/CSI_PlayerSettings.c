//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Player settings panel (in the group menu for leaders, in CSI_PlayerSettingsMenu otherwise): shows a player's
//! 				name and icon and sends color team, icon override, promote to SL/TL and kick requests to the server.
//! 
//------------------------------------------------------------------------------------------------
 class CSI_PlayerSettings : SCR_ScriptedWidgetComponent
{
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;

	protected XComboBoxWidget m_wIconOveride;
	protected TextWidget m_wPlayerName;
	protected ImageWidget m_wIcon;
	protected int m_iPlayerID = -1;		//!< player the panel shows
	protected ScriptInvoker m_PlayerUpdate;	//!< that player's GetOnDataUpdate(), while subscribed

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();

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
	//! Shows a player in the panel.
	//! \param[in] playerID
	void UpdatePlayerSettingsPlayerID(int playerID)
	{
		m_iPlayerID = playerID;

		if (playerID <= 0)
			return;
		else
			UpdatePlayerSettingsInformation();
	}

	//------------------------------------------------------------------------------------------------
	//! Refreshes the panel from the player's data and (re)subscribes to its updates.
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

		// Player hasn't registered CSI data yet (e.g. hasn't spawned since joining)
		if (!playerData)
		{
			m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_iPlayerID));
			return;
		}

		m_PlayerUpdate = playerData.GetOnDataUpdate();

		if (m_PlayerUpdate)
			m_PlayerUpdate.Insert(UpdatePlayerSettingsInformation);

		CSI_EOverrideIcon iconOverride = playerData.GetOverrideIcon();
		CSI_EColorTeam colorTeam = playerData.GetColorTeam();
		CSI_EIcon displayIcon = playerData.GetDisplayIcon();
		string displayIconStr = CSI_UIHelper.GetIconString(displayIcon, true);

		// Combo items: 0 = AUTO, then RTO (4) onwards, so item = value - 3.
		// Reset to AUTO (item 0) too, otherwise the previously viewed player's override stays selected
		if (iconOverride != CSI_EOverrideIcon.AUTO)
			m_wIconOveride.SetCurrentItem(iconOverride - 3);
		else
			m_wIconOveride.SetCurrentItem(0);

		m_wIcon.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));

		// MAN uses vanilla's pointer icon
		if (displayIconStr == "MAN_ICON")
			m_wIcon.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
		else
           	m_wIcon.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, displayIconStr);

		m_wPlayerName.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));
		m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_iPlayerID));
	};

	//------------------------------------------------------------------------------------------------
	//! Sets the color team named by the clicked button ("Red", "Blue", "Yellow", "Green", else NONE).
	//! \param[in] CTcomponent
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

		m_RplToAuthorityManager.Owner_UpdatePlayerColorTeam(m_iPlayerID, colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	//! Applies the icon override selected in the combo.
	protected void OnOverrideIconClicked()
	{
		if (m_iPlayerID <= 0)
			return;

		CSI_EOverrideIcon iconToOverride = m_wIconOveride.GetCurrentItem();

		// Combo item to enum value (item 1 = RTO = 4); item 0 stays AUTO
		if (iconToOverride > 0)
			iconToOverride = iconToOverride + 3;

		m_RplToAuthorityManager.Owner_UpdatePlayerOverrideIcon(m_iPlayerID, iconToOverride);
	}

	//------------------------------------------------------------------------------------------------
	//! Makes the player squad leader, then closes the menus.
	protected void OnPromoteToSLClicked()
	{
		if (m_iPlayerID <= 0)
			return;

		SCR_AIGroup localGroup = SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!localGroup || localGroup.IsPlayerLeader(m_iPlayerID))
			return;

		m_RplToAuthorityManager.Owner_PromotePlayerToSL(m_iPlayerID);
		GetGame().GetMenuManager().CloseAllMenus();
	};

	//------------------------------------------------------------------------------------------------
	//! Toggles the player's team leader flag.
	protected void OnPromoteToTLClicked()
	{
		if (m_iPlayerID <= 0)
			return;

		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(m_iPlayerID);
		if (!playerData)
			return;

		m_RplToAuthorityManager.Owner_UpdatePlayerTeamLeader(m_iPlayerID, !playerData.GetIsTeamLeader());
	};

	//------------------------------------------------------------------------------------------------
	//! Kicks the player from the group (vanilla kick, see CSI_RplToAuthorityManager.RpcAsk_RemovePlayerFromGroup).
	protected void OnKickClicked()
	{
		if (m_iPlayerID <= 0)
			return;

		SCR_AIGroup localGroup = SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!localGroup || localGroup.IsPlayerLeader(m_iPlayerID))
			return;

		m_RplToAuthorityManager.Owner_RemovePlayerFromGroup(m_iPlayerID);
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
