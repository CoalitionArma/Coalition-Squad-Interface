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
	//! Updates the player widget state for a specific player
	//! \param[in] playerID  Player ID to update
	void PlayerUpdate(int playerID)
	{	
		if (m_iPlayerId != playerID)
		{
			m_iPlayerId = playerID;
			CSI_Icon icon = CSI_Icon.Cast(m_wIcon.FindHandler(CSI_Icon));
		
			icon.IconUpdate(playerID);
			
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
	//! Updates player related data within the HUD element.
	protected void DataUpdate()
	{	
		if (!m_PlayerData)
			return;
		
		Color colorTeam = CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam());
		
		m_wPlayerName.SetText(CSI_UIHelper.GetPlayersName(m_PlayerData.GetPlayerID()));
		m_wPlayerName.SetColor(colorTeam); 
	}
}