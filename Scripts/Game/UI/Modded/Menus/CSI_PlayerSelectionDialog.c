modded class GroupSettingsDialogUI : DialogUI
{
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SCR_InputButtonComponent colorTeams = SCR_InputButtonComponent.Cast(GetRootWidget().FindAnyWidget("ColorTeams").FindHandler(SCR_InputButtonComponent));
		colorTeams.m_OnClicked.Insert(CTButtonClicked);
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
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	
	protected Widget m_wRoot;
	protected XComboBoxWidget m_wMaxPlayers;
	
	protected int m_iGroupCount;
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
		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();

		if (!m_GroupsManagerComponent || !m_AuthorityComponent) {OnMenuBack(); return;};
		
		m_PlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (!m_PlayersGroup) {OnMenuBack(); return;};
		
		array<int> playerIDsArray = m_PlayersGroup.GetPlayerIDs();
		m_iGroupCount = playerIDsArray.Count();
			
		if (m_iGroupCount <= 1) {OnMenuBack(); return;};
		
		string storedSpecialtyIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), SCR_PlayerController.GetLocalPlayerId(), "StoredSpecialtyIcon");
		
		if ((m_PlayersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()) && storedSpecialtyIcon == "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds") || storedSpecialtyIcon == "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds")
		{
			UpdatePlayerList();
		} else {
			OnMenuBack();
			return;
		};
		
		if (m_PlayersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()) && storedSpecialtyIcon == "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds")
		{
			ShowGroupSettings();
		}
		
		GetGame().GetCallqueue().CallLater(UpdatePlayerList, 425, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{	
		super.OnMenuInit();
		
		m_wRoot = GetRootWidget();
		
		for (int b = 0; b <= 24; b++)
		{
			SCR_InputButtonComponent buttonList = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget(string.Format("Button%1", b)).FindHandler(SCR_InputButtonComponent));
			buttonList.m_OnClicked.Insert(OnPlayerEntryClicked);
		};
				
		SCR_InputButtonComponent back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		back.m_OnClicked.Insert(OnMenuBack);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to update visual icons/text (player icon, player name, etc.)

	//------------------------------------------------------------------------------------------------
	
	protected void UpdatePlayerList()
	{		
		m_aGroupArray = m_AuthorityComponent.GetLocalGroupArray();
			
		if (m_aGroupArray.Count() <= 1) {OnMenuBack(); return;};
		
		foreach (int i, string playerStringToSplit : m_aGroupArray) {
			
			array<string> playerSplitArray = {};
			playerStringToSplit.Split("╣", playerSplitArray, false);
			
			// Get all values we need to display this player.
			int playerID = playerSplitArray[1].ToInt();
			string colorTeam = playerSplitArray[2];
			string icon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), playerID, "StoredSpecialtyIcon");

			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));
			
			// Check if we need to add ... to the end of players names.
			playerName = CheckEllipsis(192, playerDisplay, playerName);

		  playerDisplay.SetColorInt(colorTeam.ToInt());
			playerDisplay.SetText(playerName);
			statusDisplay.SetColorInt(colorTeam.ToInt());
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, icon);
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
	string CheckEllipsis(float maxLength, TextWidget nameWidget, string name)
	{	
		float sx = 0;
		float yx = 0;
		nameWidget.GetTextSize(sx, yx);
		
		if (sx > maxLength) {
			for (int e = 0; sx > maxLength - 3.5; e++)
			{
				int nameLength = name.Length();
				nameLength = nameLength - 1;
				name = name.Substring(0, nameLength);

				nameWidget.SetText(name);
				nameWidget.GetTextSize(sx, yx);
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
		ImageWidget background0           = ImageWidget.Cast(m_wRoot.FindAnyWidget("Background0"));
		ImageWidget pretty0               = ImageWidget.Cast(m_wRoot.FindAnyWidget("Pretty0"));
		TextWidget prettyText0            = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyText0"));
		TextWidget confirmMaxPlayersText  = TextWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayersText"));		
		ImageWidget confirmMaxPlayersIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayersIcon"));
		ButtonWidget confirmMaxPlayers    = ButtonWidget.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayers"));
		m_wMaxPlayers                     = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("MaxPlayers"));
		
		
		SCR_InputButtonComponent confirmMaxPlayersComp = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("ConfirmMaxPlayers").FindHandler(SCR_InputButtonComponent));
		confirmMaxPlayersComp.m_OnClicked.Insert(OnConfirmMaxPlayersClicked);
		
		background0.SetOpacity(1);
		pretty0.SetOpacity(1);
		prettyText0.SetOpacity(1);
		confirmMaxPlayersText.SetOpacity(1);
		confirmMaxPlayersIcon.SetOpacity(1);
		confirmMaxPlayers.SetOpacity(1);
		m_wMaxPlayers.SetOpacity(1);
		
		int maxMembers = m_PlayersGroup.GetMaxMembers();
		m_wMaxPlayers.SetCurrentItem(maxMembers - 1);
	};
	
	//------------------------------------------------------------------------------------------------

	// Button functions

	//------------------------------------------------------------------------------------------------
	
	protected void OnConfirmMaxPlayersClicked()
	{
		CSI_ClientComponent groupBackendComponent = CSI_ClientComponent.GetInstance();
		
		if (groupBackendComponent) {
			int maxMembers = m_wMaxPlayers.GetCurrentItem();
			groupBackendComponent.Owner_SetMaxGroupMembers(SCR_PlayerController.GetLocalPlayerId(), maxMembers + 1);
		};
	};
		
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerEntryClicked(SCR_ButtonBaseComponent component)
	{
		string playerIntStr = component.GetRootWidget().GetName();
		
		int playerInt = playerIntStr.ToInt(offset: 6);
		
		ImageWidget statusDisplayList = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", playerInt)));
		int playerOInt = statusDisplayList.GetOpacity();
		if (!playerOInt || playerOInt == 0) return;
		
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSettingsDialog, 0, true);
		CSI_PlayerSettingsDialog colorTeamMenu = CSI_PlayerSettingsDialog.Cast(menu);
		
		colorTeamMenu.SetPlayerStr(m_aGroupArray[playerInt]);
	}
}