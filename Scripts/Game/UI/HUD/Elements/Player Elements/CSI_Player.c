//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	One row of the HUD group list: the player's name in their color team plus a CSI_Icon.
//! 
//------------------------------------------------------------------------------------------------
class CSI_Player : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_PlayerDataManager m_PlayerDataManager;
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

		m_wPlayerName = RichTextWidget.Cast(w.FindAnyWidget("PlayerName"));
		m_wIcon = w.FindAnyWidget("Icon");
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Points the row at a player and subscribes to their data. Does nothing if it already shows that player.
	//! \param[in] playerID <= 0 hides the row
	void PlayerUpdate(int playerID)
	{
		if (m_iPlayerId != playerID)
		{
			m_iPlayerId = playerID;
			CSI_Icon icon = CSI_Icon.Cast(m_wIcon.FindHandler(CSI_Icon));

			icon.IconUpdate(playerID);

			// Stop listening to the previous player's data
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);

			if (playerID <= 0)
			{
				m_wRoot.SetVisible(false);
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
		if (!m_PlayerData)
			return;

		Color colorTeam = CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam());

		m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_PlayerData.GetPlayerID()));
		m_wPlayerName.SetColor(colorTeam);
	}
}