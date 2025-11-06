modded class SCR_GroupTileButton
{	
    protected CSI_PlayerData m_StoredPlayerData;

    void UpdateScriptInvoker(CSI_PlayerData playerData)
    {
        if (m_StoredPlayerData)
            m_StoredPlayerData.GetOnDataUpdate().Remove(RefreshPlayers);

        playerData.GetOnDataUpdate().Insert(RefreshPlayers);
        m_StoredPlayerData = playerData;
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

		playerButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerButton"));
		if (!playerButton)
			return;

		m_PlayerTileComponent = SCR_PlayerTileButtonComponent.Cast(playerButton.FindHandler(SCR_PlayerTileButtonComponent));
		m_PlayerTileComponent.SetTilePlayerID(playerID);

		SetupOptionsCombo(playerTile);

        string playerText = CSI_UIHelper.GetPlayersName(playerID);

        if (playerText.IsEmpty())
            playerText = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);

		playerName.SetText(playerText);
		
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

		if (playerID == GetGame().GetPlayerController().GetPlayerId())
			playerName.SetColor(m_PlayerNameSelfColor);
		
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
			}
		}

		//look for loadout and set the appropriate icon
		if (loadoutIcon)
		{
			CSI_EIcon icon = CSI_PlayerDataManager.GetInstance().GetPlayerData(playerID).GetDisplayIcon();
            string iconString = CSI_UIHelper.GetIconString(icon, true);

            loadoutIcon.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, iconString);
		}

		m_PlayerTileComponent.GetOnTileFocus().Insert(OnPlayerTileFocus);
		m_PlayerTileComponent.GetOnTileFocusLost().Insert(OnPlayerTileFocusLost);

		m_aPlayerComponentsList.Insert(m_PlayerTileComponent);
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
			m_JoinGroupButton.SetEnabled(false);
		else
			m_JoinGroupButton.SetEnabled(true);
		
		SetSquadName(squadName, groupDetailType, group);

		if (!group.GetCustomDescription().IsEmpty())
			description.SetText(group.GetCustomDescription());
		else
			description.SetText(string.Empty);

		CheckLeaderOptions();

		frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");

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
		
		array<int> playerIDs = CSI_HUDManager.GetInstance().GetSortedGroupArray(group.GetPlayerIDs());
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