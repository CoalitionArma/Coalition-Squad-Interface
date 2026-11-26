//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: player tiles show CSI names, role icons (vanilla's without CSI data) and color teams;
//! 				members are listed in CSI's sorted order; a squadmate's CSI change updates the tiles shown, and
//! 				only rebuilds the list when the order changes; clicking a group hides the CSI player settings
//! 				panel.
//!
//------------------------------------------------------------------------------------------------
modded class SCR_GroupTileButton
{
	// Player tile widgets
	protected static const string PLAYER_NAME_WIDGET_NAME = "PlayerName";
	protected static const string LOADOUT_ICON_WIDGET_NAME = "LoadoutIcon";
	protected static const string LINE_BACKGROUND_WIDGET_NAME = "LineBackground";

	protected ref array<CSI_PlayerData> m_aSubscribedPlayerData = {};	//!< squadmates' data whose changes refresh the group's details
	protected ref array<Widget> m_aPlayerTiles = {};	//!< the tiles shown, in m_aPlayerComponentsList order
	protected ref Color m_LineBackgroundColor;			//!< a tile line's color in the layout, for members without a fire team
	protected Widget m_wPlayerSettings;					//!< CSI player settings panel, found once (GetPlayerSettingsPanel)

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

		Widget playerSettings = GetPlayerSettingsPanel(m_wRoot);
		if (playerSettings)
			playerSettings.SetVisible(false);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] w a widget of the group menu
	//! \return the menu's CSI player settings panel, found once; null if it has none
	Widget GetPlayerSettingsPanel(Widget w)
	{
		if (m_wPlayerSettings)
			return m_wPlayerSettings;

		Widget rootFrame = GetRootFrame(w);
		if (rootFrame)
			m_wPlayerSettings = rootFrame.FindAnyWidget("PlayersSettings");

		return m_wPlayerSettings;
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
		SmartPanelWidget lineBackground = SmartPanelWidget.Cast(playerTile.FindAnyWidget(LINE_BACKGROUND_WIDGET_NAME));	// CSI: tinted by color team

		playerButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerButton"));
		if (!playerButton)
			return;

		m_PlayerTileComponent = SCR_PlayerTileButtonComponent.Cast(playerButton.FindHandler(SCR_PlayerTileButtonComponent));
		m_PlayerTileComponent.SetGroupTileButton(this);	// CSI: lets the tile find the CSI player settings panel
		m_PlayerTileComponent.SetTilePlayerID(playerID);

		SetupOptionsCombo(playerTile);

		// CSI: the name is set with the role icon below (SetPlayerTileCSIData)

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

		// CSI: display name with rank; role icon and color team replace loadout/leader icons and badge color
		bool hasRoleIcon = SetPlayerTileCSIData(playerName, loadoutIcon, lineBackground, playerID, isLocalPlayerInGroup);

		//look for loadout and set the appropriate icon
		if (!hasRoleIcon && loadoutIcon)	// CSI: only where the CSI icon isn't set (no CSI data)
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
		m_aPlayerTiles.Insert(playerTile);	// CSI: for UpdatePlayerTiles
	}

	//------------------------------------------------------------------------------------------------
	//! Sets a tile's name (with rank) and, for a player with CSI data, their role icon and color team.
	//! \param[in] playerName
	//! \param[in] loadoutIcon
	//! \param[in] lineBackground
	//! \param[in] playerID
	//! \param[in] isLocalPlayerInGroup color teams are only shown to members of this group
	//! \return false if the role icon wasn't set: no CSI data, or a widget is missing
	protected bool SetPlayerTileCSIData(TextWidget playerName, ImageWidget loadoutIcon, Widget lineBackground, int playerID, bool isLocalPlayerInGroup)
	{
		if (playerName)
			playerName.SetText(CSI_UIHelper.GetPlayersName(playerID));

		CSI_PlayerData playerData;
		CSI_PlayerDataManager playerDataManager = CSI_PlayerDataManager.GetInstance();
		if (playerDataManager)
			playerData = playerDataManager.GetPlayerData(playerID);

		if (!playerData || !loadoutIcon || !playerName || !lineBackground)
			return false;

		// Remove first so repeated setups don't stack subscriptions
		playerData.GetOnDataUpdate().Remove(OnPlayerDataUpdate);

		CSI_UIHelper.LoadSimpleRoleIcon(loadoutIcon, playerData.GetDisplayIcon());

		if (!isLocalPlayerInGroup)
			return true;

		playerData.GetOnDataUpdate().Insert(OnPlayerDataUpdate);

		// Removed in HandlerDeattached
		if (!m_aSubscribedPlayerData.Contains(playerData))
			m_aSubscribedPlayerData.Insert(playerData);

		// Read from the first tile set up, before any is tinted
		if (!m_LineBackgroundColor)
			m_LineBackgroundColor = lineBackground.GetColor();

		CSI_EColorTeam colorTeam = playerData.GetColorTeam();
		playerName.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));
		loadoutIcon.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));

		if (colorTeam != CSI_EColorTeam.NONE)
			lineBackground.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam, 8));
		else
			lineBackground.SetColor(m_LineBackgroundColor);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The players RefreshPlayers makes tiles for, in order: the leader, then the others in CSI's sorted order (SL,
	//! color teams, unassigned), then those without CSI data.
	//! \param[in] group
	//! \return player IDs
	protected array<int> GetTilePlayerIDs(notnull SCR_AIGroup group)
	{
		array<int> tilePlayerIDs = {};
		int leaderID = group.GetLeaderID();
		if (leaderID >= 0)
			tilePlayerIDs.Insert(leaderID);

		array<int> groupPlayerIDs = group.GetPlayerIDs();
		array<int> sortedPlayerIDs = {};
		CSI_HUDSystem hudSystem = CSI_HUDSystem.GetInstance();
		if (hudSystem)
			sortedPlayerIDs = hudSystem.GetSortedGroupArray(groupPlayerIDs);

		// GetSortedGroupArray skips members without CSI data
		foreach (int groupPlayerID : groupPlayerIDs)
		{
			if (!sortedPlayerIDs.Contains(groupPlayerID))
				sortedPlayerIDs.Insert(groupPlayerID);
		}

		foreach (int playerID : sortedPlayerIDs)
		{
			if (playerID != leaderID)
				tilePlayerIDs.Insert(playerID);
		}

		return tilePlayerIDs;
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the tiles shown with their players' CSI data, which keeps the focus where it is.
	//! \return false if RefreshPlayers would list other players or another order, so the list has to be rebuilt
	protected bool UpdatePlayerTiles()
	{
		if (!m_GroupManager)
			return false;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return false;

		// Color teams are only shown to members of this group
		if (!group.GetPlayerIDs().Contains(SCR_PlayerController.GetLocalPlayerId()))
			return false;

		array<int> tilePlayerIDs = GetTilePlayerIDs(group);
		int tileCount = tilePlayerIDs.Count();
		if (tileCount != m_aPlayerComponentsList.Count() || tileCount != m_aPlayerTiles.Count())
			return false;

		foreach (int i, int tilePlayerID : tilePlayerIDs)
		{
			SCR_PlayerTileButtonComponent tileComponent = m_aPlayerComponentsList[i];
			if (!m_aPlayerTiles[i] || !tileComponent || tileComponent.GetTilePlayerID() != tilePlayerID)
				return false;
		}

		foreach (int i, int tilePlayerID : tilePlayerIDs)
		{
			Widget playerTile = m_aPlayerTiles[i];
			TextWidget playerName = TextWidget.Cast(playerTile.FindAnyWidget(PLAYER_NAME_WIDGET_NAME));
			ImageWidget loadoutIcon = ImageWidget.Cast(playerTile.FindAnyWidget(LOADOUT_ICON_WIDGET_NAME));
			SmartPanelWidget lineBackground = SmartPanelWidget.Cast(playerTile.FindAnyWidget(LINE_BACKGROUND_WIDGET_NAME));
			SetPlayerTileCSIData(playerName, loadoutIcon, lineBackground, tilePlayerID, true);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows a squadmate's CSI change in the group's details, only while this group is selected.
	protected void OnPlayerDataUpdate()
	{
		if (!m_GroupComponent || m_GroupComponent.GetSelectedGroupID() != m_iGroupID)
			return;

		if (!UpdatePlayerTiles())
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
		m_aPlayerTiles.Clear();	// CSI

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

		// CSI: the leader, then the others in CSI's sorted order (GetTilePlayerIDs)
		array<int> playerIDs = GetTilePlayerIDs(group);
		Widget playerTile;

		int leaderID = group.GetLeaderID();
		foreach (int playerID : playerIDs)
		{
			// CSI: the leader's tile goes in the leader list
			Widget tileList = playerList;
			if (playerID == leaderID)
				tileList = leaderList;

			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, tileList);
			SetupPlayerTile(playerTile, playerID);
		}

		//disable this for now as it has issues with replication of aigroups
		//ShowAIsInGroup();

		SetupJoinGroupButton();
		SetupRemoveGroupButton();
		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
	}
}