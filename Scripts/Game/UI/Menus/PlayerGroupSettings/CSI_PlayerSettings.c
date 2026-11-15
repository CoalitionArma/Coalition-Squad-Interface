//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Player settings panel (group menu or CSI_PlayerSettingsMenu): shows a player's name and icon and sends
//! 				color team, icon override, promote and kick requests to the server; hidden while it shows no player.
//! 				Group leaders and team leaders set color teams and icons; only the leader promotes and kicks others.
//! 
//------------------------------------------------------------------------------------------------
 class CSI_PlayerSettings : SCR_ScriptedWidgetComponent
{
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;

	protected XComboBoxWidget m_wIconOveride;
	protected TextWidget m_wPlayerName;
	protected ImageWidget m_wIcon;
	protected Widget m_wAdvancedSettings;	//!< promote to SL, promote to TL and kick
	protected Widget m_wKickOverlay;
	protected PlayerId m_iPlayerID = -1;		//!< player the panel shows
	protected ScriptInvoker m_PlayerUpdate;	//!< that player's GetOnDataUpdate(), while subscribed

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Hidden until it shows a player; visible in the layout editor
		if (!SCR_Global.IsEditMode())
			w.SetVisible(false);

		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();

		m_wIconOveride = XComboBoxWidget.Cast(w.FindAnyWidget("IconOveride"));
		m_wPlayerName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wAdvancedSettings = w.FindAnyWidget("AdvancedSettingsOverlay");
		m_wKickOverlay = w.FindAnyWidget("KickOverlay");

		if (!m_wIconOveride)
			Print("CSI_PlayerSettings: combo box 'IconOveride' not found, the panel won't show player details", LogLevel.WARNING);

		if (!m_wPlayerName)
			Print("CSI_PlayerSettings: text 'PlayerName' not found, the panel won't show player details", LogLevel.WARNING);

		if (!m_wIcon)
			Print("CSI_PlayerSettings: image 'Icon' not found, the panel won't show player details", LogLevel.WARNING);

		if (!m_wAdvancedSettings)
			Print("CSI_PlayerSettings: 'AdvancedSettingsOverlay' not found, promote and kick won't hide for team leaders or on the leader's own tile", LogLevel.WARNING);

		if (!m_wKickOverlay)
			Print("CSI_PlayerSettings: 'KickOverlay' not found, kick won't hide in reserves", LogLevel.WARNING);

		SCR_ModularButtonComponent confirmIOButton = FindButton("ConfirmOverrideButton", w);
		SCR_ModularButtonComponent red = FindButton("Red", w);
		SCR_ModularButtonComponent blue = FindButton("Blue", w);
		SCR_ModularButtonComponent yellow = FindButton("Yellow", w);
		SCR_ModularButtonComponent green = FindButton("Green", w);
		SCR_ModularButtonComponent none = FindButton("None", w);
		SCR_ModularButtonComponent promoteToSLComp = FindButton("PromoteToSL", w);
		SCR_ModularButtonComponent promoteToTLComp = FindButton("PromoteToTL", w);
		SCR_ModularButtonComponent kickComp = FindButton("Kick", w);

		if (promoteToSLComp)
			promoteToSLComp.m_OnClicked.Insert(OnPromoteToSLClicked);

		if (promoteToTLComp)
			promoteToTLComp.m_OnClicked.Insert(OnPromoteToTLClicked);

		if (kickComp)
			kickComp.m_OnClicked.Insert(OnKickClicked);

		if (confirmIOButton)
			confirmIOButton.m_OnClicked.Insert(OnOverrideIconClicked);

		if (red)
			red.m_OnClicked.Insert(OnColorTeamClicked);

		if (blue)
			blue.m_OnClicked.Insert(OnColorTeamClicked);

		if (yellow)
			yellow.m_OnClicked.Insert(OnColorTeamClicked);

		if (green)
			green.m_OnClicked.Insert(OnColorTeamClicked);

		if (none)
			none.m_OnClicked.Insert(OnColorTeamClicked);
	}

	//------------------------------------------------------------------------------------------------
	//! Finds a button's SCR_ModularButtonComponent and warns if it's missing.
	//! \param[in] buttonName
	//! \param[in] parent
	//! \return null if not found
	protected SCR_ModularButtonComponent FindButton(string buttonName, Widget parent)
	{
		Widget button = parent.FindAnyWidget(buttonName);
		SCR_ModularButtonComponent buttonComponent;
		if (button)
			buttonComponent = SCR_ModularButtonComponent.FindComponent(button);

		if (!buttonComponent)
			PrintFormat("CSI_PlayerSettings: button '%1' with SCR_ModularButtonComponent not found, it won't do anything", buttonName, level: LogLevel.WARNING);

		return buttonComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows a player in the panel and subscribes to their data.
	//! \param[in] playerID <= 0 hides the panel
	void UpdatePlayerSettingsPlayerID(PlayerId playerID)
	{
		// Stop listening to the previous player's data
		if (m_PlayerUpdate)
		{
			m_PlayerUpdate.Remove(UpdatePlayerSettingsInformation);
			m_PlayerUpdate = null;
		}

		m_iPlayerID = playerID;
		GetRootWidget().SetVisible(playerID > 0);

		if (playerID <= 0)
			return;

		CSI_PlayerData playerData;
		if (m_PlayerDataManager)
			playerData = m_PlayerDataManager.GetPlayerData(playerID);

		if (playerData)
		{
			m_PlayerUpdate = playerData.GetOnDataUpdate();

			if (m_PlayerUpdate)
				m_PlayerUpdate.Insert(UpdatePlayerSettingsInformation);
		}

		UpdateLeaderActions();
		UpdatePlayerSettingsInformation();
	}

	//------------------------------------------------------------------------------------------------
	//! Refreshes the panel from the player's data.
	protected void UpdatePlayerSettingsInformation()
	{
		if (m_iPlayerID <= 0 || !m_wIconOveride || !m_wPlayerName || !m_wIcon)
			return;

		CSI_PlayerData playerData;
		if (m_PlayerDataManager)
			playerData = m_PlayerDataManager.GetPlayerData(m_iPlayerID);

		// No CSI data yet (not spawned since joining): no override, color team or role icon
		if (!playerData)
		{
			m_wIconOveride.SetCurrentItem(0);
			m_wIcon.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
			m_wIcon.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
			m_wPlayerName.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
			m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_iPlayerID));
			return;
		}

		CSI_EOverrideIcon iconOverride = playerData.GetOverrideIcon();
		CSI_EColorTeam colorTeam = playerData.GetColorTeam();
		CSI_EIcon displayIcon = playerData.GetDisplayIcon();
		string displayIconStr = CSI_UIHelper.GetIconString(displayIcon, true);

		// Combo item 0 = AUTO, other items = value - 3 (RTO = 4)
		if (iconOverride != CSI_EOverrideIcon.AUTO)
			m_wIconOveride.SetCurrentItem(iconOverride - 3);
		else
			m_wIconOveride.SetCurrentItem(0);

		m_wIcon.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));

		// MAN uses the pointer icon
		if (displayIconStr == "MAN_ICON")
			m_wIcon.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
		else
           	m_wIcon.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, displayIconStr);

		m_wPlayerName.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));
		m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_iPlayerID));
	};

	//------------------------------------------------------------------------------------------------
	//! Shows promote and kick only to the group leader, not on their own tile, and kick not in a reserves group.
	protected void UpdateLeaderActions()
	{
		bool canUseLeaderActions = CanUseLeaderActions();

		// Only its contents hide, so the settings above keep their size
		if (m_wAdvancedSettings)
		{
			Widget child = m_wAdvancedSettings.GetChildren();
			while (child)
			{
				child.SetVisible(canUseLeaderActions);
				child = child.GetSibling();
			}
		}

		// Kicking sends players to the reserves group
		if (m_wKickOverlay)
			m_wKickOverlay.SetVisible(canUseLeaderActions && GetShownPlayerGroup().GetGroupRole() != SCR_EGroupRole.RESERVES);
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the local player leads the shown player's group and isn't the shown player
	protected bool CanUseLeaderActions()
	{
		SCR_AIGroup group = GetShownPlayerGroup();
		PlayerId localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		return group && group.IsPlayerLeader(localPlayerID) && m_iPlayerID != localPlayerID;
	}

	//------------------------------------------------------------------------------------------------
	//! \return null if the shown player has no group
	protected SCR_AIGroup GetShownPlayerGroup()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return null;

		return groupsManager.GetPlayerGroup(m_iPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \return true if the local player leads the group or is a team leader in it, so can set its members' color teams and icons
	static bool CanLocalPlayerEditMembers(SCR_AIGroup group)
	{
		if (!group)
			return false;

		PlayerId localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		if (group.IsPlayerLeader(localPlayerID))
			return true;

		CSI_PlayerDataManager playerDataManager = CSI_PlayerDataManager.GetInstance();
		if (!playerDataManager || !group.IsPlayerInGroup(localPlayerID))
			return false;

		CSI_PlayerData localPlayerData = playerDataManager.GetPlayerData(localPlayerID);
		return localPlayerData && localPlayerData.GetIsTeamLeader();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the color team named by the clicked button.
	//! \param[in] CTcomponent
	protected void OnColorTeamClicked(SCR_ModularButtonComponent CTcomponent)
	{
		if (m_iPlayerID <= 0 || !m_RplToAuthorityManager)
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
		if (m_iPlayerID <= 0 || !m_wIconOveride || !m_RplToAuthorityManager)
			return;

		CSI_EOverrideIcon iconToOverride = m_wIconOveride.GetCurrentItem();

		// Combo item to enum value (item 1 = RTO = 4); item 0 stays AUTO
		if (iconToOverride > 0)
			iconToOverride = iconToOverride + 3;

		m_RplToAuthorityManager.Owner_UpdatePlayerOverrideIcon(m_iPlayerID, iconToOverride);
	}

	//------------------------------------------------------------------------------------------------
	//! Makes the player squad leader, then closes the menu the panel is in.
	protected void OnPromoteToSLClicked()
	{
		if (m_iPlayerID <= 0 || !m_RplToAuthorityManager || !CanUseLeaderActions())
			return;

		m_RplToAuthorityManager.Owner_PromotePlayerToSL(m_iPlayerID);

		ChimeraMenuBase menu = ChimeraMenuBase.GetOwnerMenu(GetRootWidget());
		if (menu)
			menu.Close();
	};

	//------------------------------------------------------------------------------------------------
	//! Toggles the player's team leader flag.
	protected void OnPromoteToTLClicked()
	{
		if (m_iPlayerID <= 0 || !m_PlayerDataManager || !m_RplToAuthorityManager || !CanUseLeaderActions())
			return;

		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(m_iPlayerID);
		if (!playerData)
			return;

		m_RplToAuthorityManager.Owner_UpdatePlayerTeamLeader(m_iPlayerID, !playerData.GetIsTeamLeader());
	};

	//------------------------------------------------------------------------------------------------
	//! Kicks the player from the group.
	protected void OnKickClicked()
	{
		if (m_iPlayerID <= 0 || !m_RplToAuthorityManager || !CanUseLeaderActions())
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
