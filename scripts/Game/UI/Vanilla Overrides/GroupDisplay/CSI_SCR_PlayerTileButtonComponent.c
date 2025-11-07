modded class SCR_PlayerTileButtonComponent
{
	Widget m_PlayerSettings;
	SCR_GroupTileButton m_TileButton;

	//------------------------------------------------------------------------------------------------
	void SetGroupTileButton(SCR_GroupTileButton tileButton)
	{
		m_TileButton = tileButton;
		
		if (m_TileButton)
			m_PlayerSettings = m_TileButton.GetRootFrame(m_wRoot).FindAnyWidget("PlayersSettings");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		Widget advSettings = m_PlayerSettings.FindAnyWidget("AdvancedSettingsOverlay");

        if (m_PlayerSettings && SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(GetTilePlayerID()).IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()))
		{
			CSI_PlayerSettings playerSettings = CSI_PlayerSettings.Cast(m_PlayerSettings.FindHandler(CSI_PlayerSettings));
            playerSettings.UpdatePlayerSettingsPlayerID(GetTilePlayerID());

            CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(GetTilePlayerID());
            
			if (m_TileButton)
				m_TileButton.UpdateScriptInvoker(playerData);
			
			m_PlayerSettings.SetVisible(true);
			advSettings.SetVisible(true);
		} else {
			m_PlayerSettings.SetVisible(false);
			advSettings.SetVisible(false);
		}
		
		return super.OnClick(w, x, y, button);
	}
}