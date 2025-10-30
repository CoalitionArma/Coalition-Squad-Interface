class CSI_Group : SCR_ScriptedWidgetComponent
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerControllerManager m_ClientComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_ClientComponent = CSI_PlayerControllerManager.GetInstance();
		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		if (!playersGroup || !m_SettingsManager.GetCSISettingBool(CSI_SettingsManager.GROUP_VISIBLE)) 
		{
			ClearGroupDisplay(0);
			return;
		};
		
		array<int> groupArray = CSI_UIHelper.GetSortedGroupArray(playersGroup);

		if (groupArray.Count() > 1)
			foreach (int i, int playerId : groupArray) 
				UpdatePlayerWidget(i, playerId);
		else
			groupArray.Clear();
		
		ClearGroupDisplay(groupArray.Count());
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearGroupDisplay(int positionToStartClearing)
	{
		for (int e = positionToStartClearing; e <= 24; e++)
			UpdatePlayerWidget(e, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerWidget(int widgetNumber, int playerId)
	{
		Widget player = m_wRoot.FindAnyWidget(string.Format("Player%1", widgetNumber));

		if (player) 
			CSI_Player.Cast(player.FindHandler(CSI_Player)).PlayerUpdate(playerId);
	}
}
