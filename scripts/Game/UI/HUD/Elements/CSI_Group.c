class CSI_Group : SCR_ScriptedWidgetComponent
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerControllerManager m_ClientComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected CSI_HUDManager m_HUDManager;
	
	protected ref array<int> m_aStoredGroupPlayerIDs;
	protected ref array<Widget> m_aPlayerWidgets;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_ClientComponent = CSI_PlayerControllerManager.GetInstance();
		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDManager = CSI_HUDManager.GetInstance();
		
		m_aPlayerWidgets = CSI_UIHelper.GetAllIcons(m_wRoot, "Player", 24);
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{
		array<int> groupArray = m_HUDManager.GetLocalGroupPlayerIds();

		if(m_aStoredGroupPlayerIDs == groupArray)
			return;
		
		int groupCount = m_HUDManager.GetLocalGroupCount();
		m_aStoredGroupPlayerIDs = groupArray;
		
		if (!m_SettingsManager.GetCSISettingBool(CSI_SettingsManager.GROUP_VISIBLE)) 
			groupCount = 1;

		if (groupCount > 1)
			foreach (int i, int playerId : groupArray) 
				UpdatePlayerWidget(i, playerId);
		else
			groupCount = 0;
		
		for (int e = groupCount; e <= 24; e++)
			UpdatePlayerWidget(e, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerWidget(int widgetNumber, int playerId)
	{
		Widget player = m_aPlayerWidgets[widgetNumber];

		if (player) 
			CSI_Player.Cast(player.FindHandler(CSI_Player)).PlayerUpdate(playerId);
	}
}
