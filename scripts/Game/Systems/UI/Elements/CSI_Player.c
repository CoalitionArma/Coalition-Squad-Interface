class CSI_Player : SCR_ScriptedWidgetComponent
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_AuthorityManager m_AuthorityManager;
	protected CSI_PlayerData m_PlayerData;
	
	protected RichTextWidget m_wPlayerName;
	protected Widget m_wIcon;

	protected int m_iPlayerId;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_AuthorityManager = CSI_AuthorityManager.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		
		m_wPlayerName = RichTextWidget.Cast(w.FindAnyWidget("PlayerName"));
		m_wIcon = w.FindAnyWidget("Icon");
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayerUpdate(int playerId)
	{
		CSI_Icon icon = CSI_Icon.Cast(m_wIcon.FindHandler(CSI_Icon));
		
		icon.IconUpdate(playerId);
		
		if (m_iPlayerId != playerId)
		{
			m_iPlayerId = playerId;
			
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);
			
			if (playerId == 0)
			{
				m_wRoot.SetVisible(false);
				m_wPlayerName.SetText("");
				return;
			};
			
			m_wRoot.SetVisible(true);
			m_wPlayerName.SetText(CSI_UIHelper.CheckEllipsis(m_wPlayerName, 102, GetGame().GetPlayerManager().GetPlayerName(playerId)));
			
			m_PlayerData = m_AuthorityManager.GetPlayerData(playerId);
			
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Insert(DataUpdate);
			
			DataUpdate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DataUpdate()
	{	
		if (!m_PlayerData)
			return;
		
		Color colorTeam = CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam());
		
		m_wPlayerName.SetColor(colorTeam); 
	}
}