class CSI_PlayerSettingsMenu : ChimeraMenuBase
{
    protected Widget m_wRoot;

    //------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
	    super.OnMenuOpen();

		m_wRoot = GetRootWidget();

        Widget settingsWidget = m_wRoot.FindAnyWidget("PlayersSettings");
        CSI_PlayerSettings playerSettings = CSI_PlayerSettings.Cast(settingsWidget.FindHandler(CSI_PlayerSettings));

        if (playerSettings)
            playerSettings.UpdatePlayerSettingsPlayerID(SCR_PlayerController.GetLocalPlayerId());

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Cancel").FindHandler(SCR_InputButtonComponent));
		cancel.m_OnClicked.Insert(OnMenuBack);
    };

    //------------------------------------------------------------------------------------------------
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseAllMenus();
	}
}