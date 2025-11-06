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
		
		for (int e; e <= 50; e++)
		{
			if (currentWidget.GetName() == "rootFrame")
				break;
			
			currentWidget = currentWidget.GetParent();
		}
		
		return currentWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		Widget advSettings = m_PlayerSettings.FindAnyWidget("AdvancedSettingsOverlay");
        if (m_PlayerSettings && SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(GetTilePlayerID()).IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()))
		{
			CSI_PlayerSettings playerSettings = CSI_PlayerSettings.Cast(m_PlayerSettings.FindHandler(CSI_PlayerSettings));
            playerSettings.UpdatePlayerSettingsPlayerID(GetTilePlayerID());
			
			m_PlayerSettings.SetVisible(true);
			advSettings.SetVisible(true);
		} else {
			m_PlayerSettings.SetVisible(false);
			advSettings.SetVisible(false);
		}
		
		return super.OnClick(w, x, y, button);
	}
}