modded class SCR_GroupMenu
{
	Widget m_PlayerSettings;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();		
		
		m_PlayerSettings = GetRootWidget().FindAnyWidget("PlayersSettings");
		
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, GetRootWidget());
		if (cancel)
			cancel.m_OnActivated.Insert(CheckClose);
		
		SCR_AIGroup.GetOnJoinPrivateGroupRequest().Insert(UpdateTabs);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateTabs);
		
		if (m_PlayerSettings)
			m_PlayerSettings.SetVisible(false);
	}
	
	protected void CheckClose()
	{
		if (!m_PlayerSettings || !m_PlayerSettings.IsVisible())
			Close();
		else
			m_PlayerSettings.SetVisible(false);
	}
}