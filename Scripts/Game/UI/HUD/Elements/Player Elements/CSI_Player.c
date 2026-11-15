//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	One row of the HUD group list: the player's name in their color team plus a CSI_Icon,
//! 				refreshed on player data and settings changes.
//! 
//------------------------------------------------------------------------------------------------
class CSI_Player : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerData m_PlayerData;

	protected RichTextWidget m_wPlayerName;
	protected Widget m_wIcon;

	protected int m_iPlayerId;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();

		// Settings changes (rank) also need a refresh
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Insert(DataUpdate);

		m_wPlayerName = RichTextWidget.Cast(w.FindAnyWidget("PlayerName"));
		if (!m_wPlayerName)
			Print("CSI_Player: rich text 'PlayerName' not found, group list names won't show", LogLevel.WARNING);

		m_wIcon = w.FindAnyWidget("Icon");
		if (!m_wIcon)
			Print("CSI_Player: 'Icon' (CSI_Icon.layout) not found, group list icons won't show", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Remove(DataUpdate);

		if (m_PlayerData)
			m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);

		super.HandlerDeattached(w);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Points the row at a player and subscribes to their data.
	//! \param[in] playerID <= 0 hides the row
	void PlayerUpdate(PlayerId playerID)
	{
		if (m_iPlayerId != playerID)
		{
			m_iPlayerId = playerID;

			CSI_Icon icon;
			if (m_wIcon)
				icon = CSI_Icon.Cast(m_wIcon.FindHandler(CSI_Icon));

			if (icon)
				icon.IconUpdate(playerID);

			// Stop listening to the previous player's data
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);

			m_PlayerData = null;

			if (playerID <= 0)
			{
				m_wRoot.SetVisible(false);

				if (m_wPlayerName)
					m_wPlayerName.SetText("");

				return;
			};

			m_wRoot.SetVisible(true);

			m_PlayerData = m_PlayerDataManager.GetPlayerData(playerID);

			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Insert(DataUpdate);

			DataUpdate();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Applies the player's name (with rank, if enabled) and color team.
	protected void DataUpdate()
	{
		if (!m_PlayerData || !m_wPlayerName)
			return;

		Color colorTeam = CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam());

		m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_PlayerData.GetPlayerID()));
		m_wPlayerName.SetColor(colorTeam);
	}
}