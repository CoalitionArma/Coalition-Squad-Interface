modded class GroupSettingsDialogUI : DialogUI
{
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SCR_InputButtonComponent colorTeams = SCR_InputButtonComponent.Cast(GetRootWidget().FindAnyWidget("ColorTeams").FindHandler(SCR_InputButtonComponent));
		colorTeams.m_OnActivated.Insert(CTButtonClicked);
	}

	protected void CTButtonClicked()
	{
		GetGame().GetMenuManager().CloseMenu(this);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSelectionDialog);
	}
}

class CSI_PlayerSelectionDialog : ChimeraMenuBase
{
	protected SCR_AIGroup m_PlayersGroup;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected CSI_ClientComponent m_ClientComponent;

	protected Widget m_wRoot;
	protected XComboBoxWidget m_wMaxPlayers;
	protected ref array<string> m_aGroupArray;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------

	override void OnMenuShow()
	{
		super.OnMenuShow();

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);

		m_wRoot = GetRootWidget();

		// Get Global Player Controller and Group Manager.
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		m_ClientComponent = CSI_ClientComponent.GetInstance();

		if (!groupsManagerComponent || !m_AuthorityComponent) 
		{
			OnMenuBack(); 
			return;
		};
		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		m_PlayersGroup = groupsManagerComponent.GetPlayerGroup(playerID);

		m_aGroupArray = m_ClientComponent.GetLocalGroupArray();

		if (!m_PlayersGroup || !m_aGroupArray || m_aGroupArray.Count() <= 0) 
		{
			OnMenuBack();
			return;
		};

		string storedSpecialtyIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), playerID, "SSI"); // SSI = StoredSpecialtyIcon

		if (m_PlayersGroup.IsPlayerLeader(playerID) || storedSpecialtyIcon == "FTL")
		{
			UpdatePlayerList();
		} else {
			OnMenuBack();
			return;
		};

		if (m_PlayersGroup.IsPlayerLeader(playerID) && storedSpecialtyIcon == "SL")
			ShowGroupSettings();

		GetGame().GetCallqueue().CallLater(UpdatePlayerList, 425, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		super.OnMenuInit();

		m_wRoot = GetRootWidget();

		for (int b = 0; b <= 24; b++)
		{
			SCR_ModularButtonComponent buttonList = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget(string.Format("Button%1", b)).FindHandler(SCR_ModularButtonComponent));
			buttonList.m_OnClicked.Insert(OnPlayerEntryClicked);
		};

		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Cancel").FindHandler(SCR_InputButtonComponent));
		cancel.m_OnClicked.Insert(OnMenuBack);
	}

	//------------------------------------------------------------------------------------------------

	// Functions to update visual icons/text (player icon, player name, etc.)

	//------------------------------------------------------------------------------------------------

	protected void UpdatePlayerList()
	{
		string rankVisible = m_ClientComponent.ReturnLocalCSISettings()[5];

		m_aGroupArray = m_ClientComponent.GetLocalGroupArray();

		if (m_aGroupArray.Count() <= 0) 
		{
			OnMenuBack(); 
			return; 
		};

		foreach (int i, string playerStringToSplit : m_aGroupArray) 
		{
			array<string> playerSplitArray = {};
			playerStringToSplit.Split(":", playerSplitArray, false);

			// Get all values we need to display this player.
			int playerID = playerSplitArray[1].ToInt();
			string colorTeamString = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), playerID, "CT"); // CT = ColorTeam
			string iconString = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), playerID, "SSI"); // SSI = StoredSpecialtyIcon

			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));

			if (!playerDisplay || !statusDisplay) continue;

			if (rankVisible == "true") 
			{
				string rank = m_AuthorityComponent.ReturnLocalPlayerMapValue(-1, playerID, "PR"); // PR = PlayerRank
				if (!rank.IsEmpty()) 
					playerName = string.Format("%1 %2", rank, playerName);
			};

			// Check if we need to add ... to the end of players names.
			playerName = CheckEllipsis(190, playerName);

			playerDisplay.SetText(playerName);
			playerDisplay.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));			
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, m_ClientComponent.SwitchStringToIcon(iconString));
			statusDisplay.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));
		};
		for (int e = m_aGroupArray.Count(); e <= 24; e++)
		{
			// Get group display widgets.
			TextWidget playerRemoveDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", e)));
			ImageWidget statusRemoveDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", e)));

			// Skip ahead to next for-loop iteration if either of these are false.
			if (!playerRemoveDisplay || !statusRemoveDisplay) continue;

			// Clear widgets.
			playerRemoveDisplay.SetText("");
			statusRemoveDisplay.SetOpacity(0);
		}
	};

	//------------------------------------------------------------------------------------------------
	string CheckEllipsis(float maxLength, string name)
	{
		float sx = 0;
		float yx = 0;
		
		TextWidget testWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("TestPlayerName"));
		testWidget.SetText(name);
		testWidget.GetTextSize(sx, yx);

		if (sx > maxLength) 
		{
			for (int e = 0; sx > maxLength - 3.5; e++)
			{
				int nameLength = name.Length();
				nameLength = nameLength - 1;
				name = name.Substring(0, nameLength);

				testWidget.SetText(name);
				testWidget.GetTextSize(sx, yx);
			};
			name = string.Format("%1...", name);
		};
		return name;
	};

	//------------------------------------------------------------------------------------------------

	// Additional menu functions

	//------------------------------------------------------------------------------------------------

	protected void OnMenuBack()
	{
		GetGame().GetCallqueue().Remove(UpdatePlayerList);
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseMenu(this);
	}

	//------------------------------------------------------------------------------------------------
	void ShowGroupSettings()
	{
		ImageWidget background0 = ImageWidget.Cast(m_wRoot.FindAnyWidget("Background0"));
		ImageWidget pretty0 = ImageWidget.Cast(m_wRoot.FindAnyWidget("Pretty0"));
		TextWidget prettyText0 = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyText0"));
		TextWidget confirmMaxPlayersText = TextWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayersText"));
		ImageWidget confirmMaxPlayersIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayersIcon"));
		ButtonWidget confirmMaxPlayers = ButtonWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayers"));
		m_wMaxPlayers = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("MaxPlayers"));


		SCR_ModularButtonComponent confirmMaxPlayersComp = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayers").FindHandler(SCR_ModularButtonComponent));
		confirmMaxPlayersComp.m_OnClicked.Insert(OnConfirmMaxPlayersClicked);

		background0.SetOpacity(1);
		pretty0.SetOpacity(1);
		prettyText0.SetOpacity(1);
		confirmMaxPlayersText.SetOpacity(1);
		confirmMaxPlayersIcon.SetOpacity(1);
		confirmMaxPlayers.SetOpacity(1);
		m_wMaxPlayers.SetOpacity(1);

		confirmMaxPlayers.SetEnabled(true);
		m_wMaxPlayers.SetEnabled(true);

		int maxMembers = m_PlayersGroup.GetMaxMembers();
		m_wMaxPlayers.SetCurrentItem(maxMembers - 1);
	};

	//------------------------------------------------------------------------------------------------

	// Button functions

	//------------------------------------------------------------------------------------------------

	protected void OnConfirmMaxPlayersClicked()
	{
		CSI_ClientComponent groupBackendComponent = CSI_ClientComponent.GetInstance();

		if (groupBackendComponent) 
		{
			int maxMembers = m_wMaxPlayers.GetCurrentItem();
			groupBackendComponent.Owner_SetMaxGroupMembers(SCR_PlayerController.GetLocalPlayerId(), maxMembers + 1);
		};
	};

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerEntryClicked(SCR_ModularButtonComponent component)
	{
		string playerIntStr = component.GetRootWidget().GetName();

		int playerInt = playerIntStr.ToInt(offset: 6);

		ImageWidget statusDisplayList = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", playerInt)));
		int playerOInt = statusDisplayList.GetOpacity();
		if (!playerOInt || playerOInt == 0) 
			return;

		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);

		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSettingsDialog, 0, true);
		CSI_PlayerSettingsDialog colorTeamMenu = CSI_PlayerSettingsDialog.Cast(menu);

		colorTeamMenu.SetPlayerStr(m_aGroupArray[playerInt]);
	}
}
