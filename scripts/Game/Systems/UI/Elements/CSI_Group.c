class CSI_Group : SCR_ScriptedWidgetComponent
{
	protected CSI_ClientManager m_ClientComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	//------------------------------------------------------------------------------------------------
	void Update()
	{
		if (!m_ClientComponent || !m_GroupsManagerComponent) 
		{
			m_ClientComponent = CSI_ClientManager.GetInstance();
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			return;
		};
		
		string groupDisplayVisible = m_ClientComponent.ReturnLocalCSISettings()[2];
		
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		if (groupDisplayVisible == "false" || !playersGroup) 
		{
			ClearGroupDisplay(0);
			return;
		};
		
		array<int> groupArray = CSI_UIHelper.GetSortedGroupArray(playersGroup);

		foreach (int i, int playerId : groupArray) 
			UpdatePlayerWidget(i, playerId);
		
		ClearGroupDisplay(groupArray.Count());
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearGroupDisplay(int positionToStartClearing)
	{
		for (int e = positionToStartClearing; e <= 24; e++)
			UpdatePlayerWidget(e, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerWidget(int widgetNumber, int playerId)
	{
		Widget player = m_wRoot.FindAnyWidget(string.Format("Player%1", widgetNumber));

		if (player) 
			CSI_Player.Cast(player.FindHandler(CSI_Player)).PlayerUpdate(playerId);
	}
}
