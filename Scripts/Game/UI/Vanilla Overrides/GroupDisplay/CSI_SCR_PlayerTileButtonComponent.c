//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: clicking a player tile opens CSI's player settings panel (for the group's leader) instead of
//! 				vanilla's options combo.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_PlayerTileButtonComponent
{
	Widget m_PlayerSettings;			//!< CSI player settings panel in the group menu
	SCR_GroupTileButton m_TileButton;

	//------------------------------------------------------------------------------------------------
	//! Called from SCR_GroupTileButton.SetupPlayerTile; also finds the CSI player settings panel.
	//! \param[in] tileButton
	void SetGroupTileButton(SCR_GroupTileButton tileButton)
	{
		m_TileButton = tileButton;

		if (m_TileButton)
			m_PlayerSettings = m_TileButton.GetRootFrame(m_wRoot).FindAnyWidget("PlayersSettings");
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the CSI player settings for this tile's player if the local player leads their group, otherwise hides them.
	//! Falls back to vanilla when the panel isn't in the layout.
	//! \param[in] w
	//! \param[in] x
	//! \param[in] y
	//! \param[in] button
	//! \return
	override bool OnClick(Widget w, int x, int y, int button)
	{
		Widget advSettings = m_PlayerSettings.FindAnyWidget("AdvancedSettingsOverlay");

        if (m_PlayerSettings && SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(GetTilePlayerID()).IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()))
		{
			CSI_PlayerSettings playerSettings = CSI_PlayerSettings.Cast(m_PlayerSettings.FindHandler(CSI_PlayerSettings));
				playerSettings.UpdatePlayerSettingsPlayerID(GetTilePlayerID());

			m_PlayerSettings.SetVisible(true);
			advSettings.SetVisible(true);
			return true;
		} else {
			m_PlayerSettings.SetVisible(false);
			advSettings.SetVisible(false);
			return true;
		};

		return super.OnClick(w, x, y, button);
	}
}