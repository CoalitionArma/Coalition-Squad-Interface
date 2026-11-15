//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD group list beside the compass: a CSI_Player row per group member in CSI_HUDSystem's sorted order, up to
//! 				MAX_PLAYER_ROWS. Rows are created from CSI_Player.layout as the group grows and deleted as it shrinks,
//! 				filling the layout's 5 x 5 grid a column at a time. Only shown in groups with more than one player.
//! 
//------------------------------------------------------------------------------------------------
class CSI_Group : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;

	protected Widget m_wGrid;	//!< parent of the rows

	protected ref array<int> m_aStoredGroupPlayerIDs;	//!< last array applied; null forces a refresh
	protected ref array<CSI_Player> m_aPlayerRows = {};	//!< rows in grid order, one per listed group member

	protected static const int MAX_PLAYER_ROWS = 25;	//!< the grid's cells; larger groups list their first 25 members
	protected static const int GRID_ROWS = 5;			//!< rows per grid column (the layout's RowFill), filled top to bottom

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();

		// CSI_HUDSystem only hands out a new group array when it refreshes, so also refresh when settings change
		// (group visibility). Same settings invoker CSI_Icon listens to.
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Insert(OnSettingsUpdate);

		m_wGrid = w.FindAnyWidget("GroupGridLayout");
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Remove(OnSettingsUpdate);

		// The rows are children of this layout and get destroyed with it
		m_aPlayerRows.Clear();

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	//! Forces the next Update to re-apply the rows
	protected void OnSettingsUpdate()
	{
		m_aStoredGroupPlayerIDs = null;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD; only does work when the group array was refreshed or the settings changed.
	void Update()
	{
		array<int> groupArray = m_HUDSystem.GetLocalGroupPlayerIds();

		// Same array reference = no refresh since the last update (CSI_HUDSystem hands out a new array each refresh)
		if (m_aStoredGroupPlayerIDs == groupArray)
			return;

		m_aStoredGroupPlayerIDs = groupArray;

		int rowCount;
		if (m_HUDSystem.GetLocalGroupCount() > 1 && m_SettingsManager.GetSettingBool(CSI_GameSettings.GROUP_VISIBLE))
			rowCount = Math.ClampInt(groupArray.Count(), 0, MAX_PLAYER_ROWS);

		SetPlayerRowCount(rowCount);

		// A row only resubscribes when the member at its place changed
		foreach (int i, CSI_Player row : m_aPlayerRows)
		{
			if (row)
				row.PlayerUpdate(groupArray[i]);
		}
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Creates or deletes rows at the end of the list until there are count of them.
	//! \param[in] count
	protected void SetPlayerRowCount(int count)
	{
		// Deleting a row also unsubscribes its CSI_Player and CSI_Icon (HandlerDeattached)
		for (int i = m_aPlayerRows.Count() - 1; i >= count; i--)
		{
			CSI_Player row = m_aPlayerRows[i];
			if (row && row.GetRootWidget())
				row.GetRootWidget().RemoveFromHierarchy();

			m_aPlayerRows.Remove(i);
		}

		while (m_aPlayerRows.Count() < count)
		{
			if (!CreatePlayerRow())
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates the next row from CSI_Player.layout, in the grid cell for its place in the list.
	//! \return the row's handler, or null if the widget couldn't be created
	protected CSI_Player CreatePlayerRow()
	{
		if (!m_wGrid)
			return null;

		Widget rowWidget = GetGame().GetWorkspace().CreateWidgets(CSI_UIHelper.CSI_PLAYER_LAYOUT, m_wGrid);
		if (!rowWidget)
			return null;

		CSI_Player row = CSI_Player.Cast(rowWidget.FindHandler(CSI_Player));
		if (!row)
		{
			rowWidget.RemoveFromHierarchy();
			return null;
		}

		// A column at a time, top to bottom
		int index = m_aPlayerRows.Insert(row);
		GridSlot.SetRow(rowWidget, index % GRID_ROWS);
		GridSlot.SetColumn(rowWidget, index / GRID_ROWS);

		return row;
	}
}
