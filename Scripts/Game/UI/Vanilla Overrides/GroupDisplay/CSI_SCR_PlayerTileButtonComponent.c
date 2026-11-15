//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: clicking a player tile opens the CSI player settings panel (group leader only)
//! 				instead of the options combo.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_PlayerTileButtonComponent
{
	Widget m_PlayerSettings;			//!< CSI player settings panel
	SCR_GroupTileButton m_TileButton;

	//------------------------------------------------------------------------------------------------
	//! Also finds the CSI player settings panel.
	//! \param[in] tileButton
	void SetGroupTileButton(SCR_GroupTileButton tileButton)
	{
		m_TileButton = tileButton;

		if (!m_TileButton)
			return;

		Widget rootFrame = m_TileButton.GetRootFrame(m_wRoot);
		if (rootFrame)
			m_PlayerSettings = rootFrame.FindAnyWidget("PlayersSettings");
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the CSI player settings panel if the local player leads this player's group.
	//! \param[in] w
	//! \param[in] x
	//! \param[in] y
	//! \param[in] button
	//! \return
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_PlayerSettings)
			return super.OnClick(w, x, y, button);

		Widget advSettings = m_PlayerSettings.FindAnyWidget("AdvancedSettingsOverlay");
		SCR_AIGroup tilePlayersGroup = SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(GetTilePlayerID());
		bool showSettings = tilePlayersGroup && tilePlayersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId());

		if (showSettings)
		{
			CSI_PlayerSettings playerSettings = CSI_PlayerSettings.Cast(m_PlayerSettings.FindHandler(CSI_PlayerSettings));
			if (playerSettings)
				playerSettings.UpdatePlayerSettingsPlayerID(GetTilePlayerID());
		}

		m_PlayerSettings.SetVisible(showSettings);

		if (advSettings)
			advSettings.SetVisible(showSettings);

		return true;
	}
}