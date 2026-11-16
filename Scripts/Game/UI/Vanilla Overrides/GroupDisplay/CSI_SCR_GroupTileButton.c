//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: player tiles show CSI names, role icons (vanilla's without CSI data) and color teams;
//! 				members are listed in CSI's sorted order; clicking a group hides the CSI player settings panel.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_GroupTileButton
{
	protected ref array<CSI_PlayerData> m_aSubscribedPlayerData = {};	//!< squadmates' data whose changes refresh the group's details

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		foreach (CSI_PlayerData playerData : m_aSubscribedPlayerData)
		{
			if (playerData)
				playerData.GetOnDataUpdate().Remove(OnPlayerDataUpdate);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Also hides the CSI player settings panel.
	//! \param[in] w
	//! \param[in] x
	//! \param[in] y
	//! \param[in] button
	//! \return
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		Widget rootFrame = GetRootFrame(m_wRoot);
		Widget playerSettings;
		if (rootFrame)
			playerSettings = rootFrame.FindAnyWidget("PlayersSettings");

		if (playerSettings)
			playerSettings.SetVisible(false);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Walks up from w (at most 50 levels) to the group menu's "rootFrame" widget.
	//! \param[in] w
	//! \return "rootFrame", or null if the top of the hierarchy is reached first
	Widget GetRootFrame(Widget w)
	{
		Widget currentWidget = w;

		for (int e; e <= 50; e++)
		{
			if (!currentWidget || currentWidget.GetName() == "rootFrame")
				break;
			currentWidget = currentWidget.GetParent();
		}

		return currentWidget;
	}

	//------------------------------------------------------------------------------------------------
	//TODO: setup should be taken care of by the player tile component
	override void SetupPlayerTile(Widget playerTile, int playerID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		// CSI: color teams are only shown to members of this group
		bool isLocalPlayerInGroup = group.GetPlayerIDs().Contains(SCR_PlayerController.GetLocalPlayerId());
		CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(playerID);

		SCR_GadgetManagerComponent gadgetManager;
		TextWidget playerName, playerFrequency;
		ImageWidget taskIcon, muteIcon, background, loadoutIcon;
		//string m_sTaskText;
		SizeLayoutWidget m_wTaskLayout;
		ButtonWidget playerButton;
		SCR_TaskExecutor taskExecutor;
		SCR_BasePlayerLoadout playerLoadout;
		Resource res;
		IEntityComponentSource source;
		BaseContainer container;
		SCR_EditableEntityUIInfo info;
		set<int> frequencies = new set<int>();

		playerName = TextWidget.Cast(playerTile.FindAnyWidget("PlayerName"));
		playerFrequency = TextWidget.Cast(playerTile.FindAnyWidget("Frequency"));
		m_wTaskLayout = SizeLayoutWidget.Cast(playerTile.FindAnyWidget("TaskLayout"));
		Widget wBackground = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconBackground"));
		Widget m_wOutline = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconOutline"));
		Widget m_wSymbol = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconSymbol"));
		muteIcon = ImageWidget.Cast(playerTile.FindAnyWidget("MuteIcon"));
		background = ImageWidget.Cast(playerTile.FindAnyWidget("Background"));
		loadoutIcon = ImageWidget.Cast(playerTile.FindAnyWidget("LoadoutIcon"));
		ImageWidget m_wIconSymbol = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconSymbol"));
		ImageWidget platformIcon = ImageWidget.Cast(playerTile.FindAnyWidget("PlatformIcon"));
		SmartPanelWidget lineBackground = SmartPanelWidget.Cast(playerTile.FindAnyWidget("LineBackground"));	// CSI: tinted by color team

		playerButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerButton"));
		if (!playerButton)
			return;

		m_PlayerTileComponent = SCR_PlayerTileButtonComponent.Cast(playerButton.FindHandler(SCR_PlayerTileButtonComponent));
		m_PlayerTileComponent.SetGroupTileButton(this);	// CSI: lets the tile find the CSI player settings panel
		m_PlayerTileComponent.SetTilePlayerID(playerID);

		SetupOptionsCombo(playerTile);

		// CSI: display name with rank
		playerName.SetText(CSI_UIHelper.GetPlayersName(playerID));

		SCR_PlayerController playerCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerCtrl)
			playerCtrl.SetPlatformImageTo(playerID, platformIcon);

		ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
		if (controlledEntity)
		{
			gadgetManager = SCR_GadgetManagerComponent.Cast(controlledEntity.FindComponent(SCR_GadgetManagerComponent));
			SCR_Global.GetFrequencies(gadgetManager, frequencies);
			if (!frequencies.IsEmpty())
			{
				playerFrequency.SetText(SCR_FormatHelper.FormatFrequencies(frequencies));
				background.SetOpacity(0.85);
			}
		}

		// CSI: no own-name color, names use the color team

		//set the state of mute
		PlayerController pc = GetGame().GetPlayerController();
		// Using another PlayerController variable to not alter rest of code
		if (muteIcon && pc)
		{
			SocialComponent sc = SocialComponent.Cast(pc.FindComponent(SocialComponent));
			if (sc && sc.IsRestricted(playerID, EUserInteraction.VoiceChat))
			{
				muteIcon.SetColor(m_PlayerNameSelfColor);
				muteIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, "sound-off");
			};
		};

		// CSI: role icon and color team replace loadout/leader icons and badge color
		if (playerData && loadoutIcon && playerName && lineBackground)
		{
			// Remove first so repeated setups don't stack subscriptions
			playerData.GetOnDataUpdate().Remove(OnPlayerDataUpdate);

			CSI_EIcon displayIcon = playerData.GetDisplayIcon();

			bool setAlt = false;
			if (displayIcon == CSI_EIcon.DRIVER)
				setAlt = true;

            string iconString = CSI_UIHelper.GetIconString(displayIcon, true, setAlt);

			// MAN uses the pointer icon
			if (iconString == "MAN_ICON")
				loadoutIcon.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
			else
            	loadoutIcon.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, iconString);

			if (isLocalPlayerInGroup)
			{
				playerData.GetOnDataUpdate().Insert(OnPlayerDataUpdate);

				// Removed in HandlerDeattached
				if (!m_aSubscribedPlayerData.Contains(playerData))
					m_aSubscribedPlayerData.Insert(playerData);

				CSI_EColorTeam colorTeam = playerData.GetColorTeam();
				playerName.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));
				loadoutIcon.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));

				if (colorTeam != CSI_EColorTeam.NONE)
				{
					lineBackground.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam, 8));
				};
			};
		}
		//look for loadout and set the appropriate icon
		else if (loadoutIcon)	// CSI: only where the CSI icon isn't set (no CSI data)
		{
			SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
			if (loadoutManager)
				playerLoadout = loadoutManager.GetPlayerLoadout(playerID);

			if (playerLoadout)
			{
				res = Resource.Load(playerLoadout.GetLoadoutResource());
				source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
				if (source)
				{
					container = source.GetObject("m_UIInfo");
					info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
					info.SetIconTo(loadoutIcon);
				}
			}

			//set leader icon if given player is a leader
			if (group.IsPlayerLeader(playerID))
			{
				m_LeaderInfo.SetIconTo(loadoutIcon);
				loadoutIcon.SetColor(m_PlayerNameSelfColor);
			}

			// CSI: badge keeps its layout color, as for members with CSI data
		}

		m_PlayerTileComponent.GetOnTileFocus().Insert(OnPlayerTileFocus);
		m_PlayerTileComponent.GetOnTileFocusLost().Insert(OnPlayerTileFocusLost);

		m_aPlayerComponentsList.Insert(m_PlayerTileComponent);
	}

	//------------------------------------------------------------------------------------------------
	//! Refreshes the group's details for a squadmate's CSI change, only while this group is selected.
	protected void OnPlayerDataUpdate()
	{
		if (!m_GroupComponent || m_GroupComponent.GetSelectedGroupID() != m_iGroupID)
			return;

		RefreshPlayers();
	}

	//------------------------------------------------------------------------------------------------
	override void RefreshPlayers()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		SetupSelectGroupFlagButton(group);

		if (m_ParentSubMenu == null)
			FindParentMenu();

		SetSelectedGroupTaskInfo(group);

		m_aPlayerComponentsList.Clear();

		VerticalLayoutWidget playerList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("PlayerList"));
		if (!playerList)
			return;

		VerticalLayoutWidget leaderList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Leader"));
		if (!leaderList)
			return;

		RichTextWidget squadName = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailCallsign"));
		if (!squadName)
			return;

		RichTextWidget squadType = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Type"));
		if (!squadType)
			return;

		RichTextWidget description = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Description"));
		if (!description)
			return;

		RichTextWidget frequency = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailFrequency"));
		if (!frequency)
			return;

		RichTextWidget groupDetailType = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailType"));
		if (!groupDetailType)
			return;

		SCR_PlayerControllerGroupComponent s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!s_PlayerGroupController)
			return;

		SocialComponent socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
		if (!socialComp)
			return;

		if (!s_PlayerGroupController.CanPlayerJoinGroup(playerController.GetPlayerId(), m_GroupManager.FindGroup(m_iGroupID)))
			SetJoinButtonVisible(false);
		else
			SetJoinButtonVisible(true);

		// This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
		squadName.SetText(SCR_GroupHelperUI.GetTranslatedGroupName(group));

		Widget groupTypeOverlay = m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupTypeOverlay");
		if (groupTypeOverlay)
		{
			string roleName = SCR_GroupHelperUI.GetRoleName(group);
			groupTypeOverlay.SetVisible(!roleName.IsEmpty());
			groupDetailType.SetText(roleName);
		}

		if (!group.GetCustomDescription().IsEmpty())
			description.SetText(group.GetCustomDescription());
		else
			description.SetText(string.Empty);

		CheckLeaderOptions();

		frequency.SetTextFormat("%1 #AR-VON_FrequencyUnits_MHz", group.GetRadioFrequency() * 0.001);

		Widget children = playerList.GetChildren();
		while (children)
		{
			playerList.RemoveChild(children);
			children = playerList.GetChildren();
		}

		children = leaderList.GetChildren();
		while (children)
		{
			leaderList.RemoveChild(children);
			children = leaderList.GetChildren();
		}

		// CSI: list members in CSI's sorted order (SL, color teams, unassigned)
		array<int> groupPlayerIDs = group.GetPlayerIDs();
		array<int> playerIDs = CSI_HUDSystem.GetInstance().GetSortedGroupArray(groupPlayerIDs);

		// CSI: also list members without CSI data (GetSortedGroupArray skips them)
		foreach (int groupPlayerID : groupPlayerIDs)
		{
			if (!playerIDs.Contains(groupPlayerID))
				playerIDs.Insert(groupPlayerID);
		}

		Widget playerTile;

		int leaderID = group.GetLeaderID();
		if (leaderID >= 0)
		{
			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, leaderList);
			SetupPlayerTile(playerTile, leaderID);
		}

		foreach (int playerID : playerIDs)
		{
			if (playerID == leaderID)
				continue;

			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, playerList);
			SetupPlayerTile(playerTile, playerID);
		}

		//disable this for now as it has issues with replication of aigroups
		//ShowAIsInGroup();

		SetupJoinGroupButton();
		SetupRemoveGroupButton();
		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
	}
}