//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD group list beside the compass: fills the layout's 25 fixed Player0..24 widgets in CSI_HUDSystem's sorted order.
//! 				Only shown in groups with more than one player.
//! 
//------------------------------------------------------------------------------------------------
class CSI_Group : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;

	protected ref array<int> m_aStoredGroupPlayerIDs;	//!< last array applied; null forces a refresh
	protected ref array<Widget> m_aPlayerWidgets;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();

		m_aPlayerWidgets = CSI_UIHelper.GetAllIcons(m_wRoot, "Player", 25);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD; only does work when the group array changed.
	void Update()
	{
		array<int> groupArray = m_HUDSystem.GetLocalGroupPlayerIds();

		// Same array reference = same group (CSI_HUDSystem hands out a new array on change)
		if(m_aStoredGroupPlayerIDs == groupArray)
			return;

		int groupCount = m_HUDSystem.GetLocalGroupCount();
		m_aStoredGroupPlayerIDs = groupArray;

		if (groupCount > 1 && m_SettingsManager.GetSettingBool(CSI_GameSettings.GROUP_VISIBLE))
			foreach (int i, int playerID : groupArray)
				UpdatePlayerWidget(i, playerID);
		else
			groupCount = 0;

		// Hide the unused widgets
		for (int e = groupCount; e <= 24; e++)
			UpdatePlayerWidget(e, -1);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] widgetNumber index of the Player widget
	//! \param[in] playerID player to show; -1 hides the widget
	protected void UpdatePlayerWidget(int widgetNumber, int playerID)
	{
		// Groups larger than the layout's 25 player widgets would otherwise index out of bounds
		if (!m_aPlayerWidgets.IsIndexValid(widgetNumber))
			return;

		Widget player = m_aPlayerWidgets[widgetNumber];

		if (player)
			CSI_Player.Cast(player.FindHandler(CSI_Player)).PlayerUpdate(playerID);
	}
}
