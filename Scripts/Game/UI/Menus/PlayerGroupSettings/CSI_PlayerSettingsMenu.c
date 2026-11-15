//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Menu with the local player's own CSI_PlayerSettings panel, for players who aren't group leaders.
//! 
//------------------------------------------------------------------------------------------------
class CSI_PlayerSettingsMenu : ChimeraMenuBase
{
    protected Widget m_wRoot;
	protected SCR_InputButtonComponent m_CancelButton;

    //------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
	    super.OnMenuOpen();

		m_wRoot = GetRootWidget();

		Widget settingsWidget = m_wRoot.FindAnyWidget("PlayersSettings");
		CSI_PlayerSettings playerSettings;
		if (settingsWidget)
			playerSettings = CSI_PlayerSettings.Cast(settingsWidget.FindHandler(CSI_PlayerSettings));

		if (playerSettings)
			playerSettings.UpdatePlayerSettingsPlayerID(SCR_PlayerController.GetLocalPlayerId());
		else
			Print("CSI_PlayerSettingsMenu: 'PlayersSettings' with CSI_PlayerSettings not found, the menu won't show the player", LogLevel.WARNING);

		// The settings fill the menu; promote and kick are in the group menu
		Widget advancedSettings = m_wRoot.FindAnyWidget("AdvancedSettingsOverlay");
		if (advancedSettings)
			advancedSettings.SetVisible(false);

		// Removed in OnMenuClose
		m_CancelButton = SCR_InputButtonComponent.GetInputButtonComponent("Cancel", m_wRoot);
		if (m_CancelButton)
			m_CancelButton.m_OnActivated.Insert(Close);
		else
			Print("CSI_PlayerSettingsMenu: back button 'Cancel' not found, the menu can't be closed", LogLevel.WARNING);
    };

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (m_CancelButton)
			m_CancelButton.m_OnActivated.Remove(Close);

		super.OnMenuClose();
	}
}
