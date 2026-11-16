//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: clicking a player tile opens the CSI player settings panel (group leader and team
//! 				leaders) instead of the options combo.
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
	//! Shows the CSI player settings panel if the local player leads this player's group or is a team leader in it.
	//! \param[in] w
	//! \param[in] x
	//! \param[in] y
	//! \param[in] button
	//! \return
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_PlayerSettings)
			return super.OnClick(w, x, y, button);

		SCR_AIGroup tilePlayersGroup;
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
			tilePlayersGroup = groupsManager.GetPlayerGroup(GetTilePlayerID());

		// The panel shows promote and kick to the leader only
		bool showSettings = CSI_PlayerSettings.CanLocalPlayerEditMembers(tilePlayersGroup);
		if (showSettings)
		{
			CSI_PlayerSettings playerSettings = CSI_PlayerSettings.Cast(m_PlayerSettings.FindHandler(CSI_PlayerSettings));
			if (playerSettings)
				playerSettings.UpdatePlayerSettingsPlayerID(GetTilePlayerID());
		}

		m_PlayerSettings.SetVisible(showSettings);
		return true;
	}
}