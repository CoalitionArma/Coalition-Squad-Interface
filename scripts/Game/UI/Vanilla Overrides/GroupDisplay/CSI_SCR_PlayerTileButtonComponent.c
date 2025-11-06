modded class SCR_PlayerTileButtonComponent
{
	Widget m_PlayerSettings;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_PlayerSettings = GetRootFrame(w).FindAnyWidget("PlayersSettings");
	}
	
	protected Widget GetRootFrame(Widget w)
	{
		Widget currentWidget = w;
		
		for (int e; e <= 25; e++)
		{
			if (currentWidget.GetName() == "rootFrame")
				break;
			currentWidget = currentWidget.GetParent();
		}
		
		return currentWidget;
	}

	protected SCR_GroupTileButton GetGroupTileButton(Widget w)
	{
		Widget currentWidget = w;
		SCR_GroupTileButton tileButton;

		for (int e; e <= 25; e++)
		{
            tileButton = SCR_GroupTileButton.Cast(currentWidget.FindHandler(SCR_GroupTileButton));

			if (tileButton)
				break;
        
			currentWidget = currentWidget.GetParent();
		}
		
		return tileButton;
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
            GetGroupTileButton(w).UpdateScriptInvoker(playerData);
			
			m_PlayerSettings.SetVisible(true);
			advSettings.SetVisible(true);
		} else {
			m_PlayerSettings.SetVisible(false);
			advSettings.SetVisible(false);
		}
		
		return super.OnClick(w, x, y, button);
	}
}