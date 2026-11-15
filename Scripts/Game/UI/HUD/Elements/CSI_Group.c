//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD group list: a CSI_Player row per group member (up to 25), filling a 5 x 5 grid a column at a time.
//! 				Hidden in groups of one.
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
	protected ref array<CSI_Player> m_aPlayerRows = {};	//!< rows in grid order

	protected static const int MAX_PLAYER_ROWS = 25;	//!< grid cells; extra members aren't listed
	protected static const int GRID_ROWS = 5;			//!< rows per grid column (the layout's RowFill)

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();

		// Settings changes (group visibility) also need a refresh
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Insert(OnSettingsUpdate);

		m_wGrid = w.FindAnyWidget("GroupGridLayout");
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Remove(OnSettingsUpdate);

		// Rows are child widgets, destroyed with this layout
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
	//! Called every frame by CSI_HUD; acts only on a new group array or settings change.
	void Update()
	{
		array<int> groupArray = m_HUDSystem.GetLocalGroupPlayerIds();

		// Same array reference: no group refresh since the last update
		if (m_aStoredGroupPlayerIDs == groupArray)
			return;

		m_aStoredGroupPlayerIDs = groupArray;

		int rowCount;
		if (m_HUDSystem.GetLocalGroupCount() > 1 && m_SettingsManager.GetSettingBool(CSI_GameSettings.GROUP_VISIBLE))
			rowCount = Math.ClampInt(groupArray.Count(), 0, MAX_PLAYER_ROWS);

		SetPlayerRowCount(rowCount);

		// Rows only resubscribe when their member changed
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
	protected void SetPlayerRowCount(int count)
	{
		// Removing a row unsubscribes it (HandlerDeattached)
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
	//! \return the row, or null if it couldn't be created
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
