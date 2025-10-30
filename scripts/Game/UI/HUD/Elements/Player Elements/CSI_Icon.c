class CSI_Icon : SCR_ScriptedWidgetComponent
{	
	protected CSI_AuthorityManager m_AuthorityManager;
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerData m_PlayerData;
	protected int m_iPlayerId;
	
	protected ImageWidget m_wArrow;
	protected ImageWidget m_wIcon;
	protected ImageWidget m_wOutline;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_AuthorityManager = CSI_AuthorityManager.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_SettingsManager.GetOnSettingsUpdate().Insert(DataUpdate);
		
		m_wArrow = ImageWidget.Cast(w.FindAnyWidget("Arrow"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wOutline = ImageWidget.Cast(w.FindAnyWidget("Outline"));
	}
	
	//------------------------------------------------------------------------------------------------
	void IconUpdate(int playerId)
	{
		if (m_iPlayerId != playerId)
		{
			m_iPlayerId = playerId;
			
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);
			
			if (playerId <= 0)
			{
				m_wRoot.SetVisible(false);
				return;
			};
			
			m_wRoot.SetVisible(true);
			m_PlayerData = m_AuthorityManager.GetPlayerData(playerId);
			
			if (!m_PlayerData)
				return;
			
			m_PlayerData.GetOnDataUpdate().Insert(DataUpdate);
			
			DataUpdate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRotation(float yaw)
	{
		m_wArrow.SetRotation(yaw);

		if (m_SettingsManager.GetCSISettingBool(CSI_SettingsManager.ONLY_RADAR_ICON_ARROWS_ROTATE))
			yaw = 0;

		m_wIcon.SetRotation(yaw);
		m_wOutline.SetRotation(yaw);
	};
	
	//------------------------------------------------------------------------------------------------
	protected void DataUpdate()
	{	
		if (!m_PlayerData)
			return;
		
		SetIconWidget(m_PlayerData.GetDisplayIcon());
		SetArrowWidget();
		
		Color colorTeam = CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam());
		
		m_wArrow.SetColor(colorTeam); 
		m_wIcon.SetColor(colorTeam);
		m_wOutline.SetColor(colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetIconWidget(CSI_EIcon icon)
	{
		string iconString = CSI_UIHelper.GetIconString(icon);
		
		if (m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.ICON_TYPE) == CSI_EIconType.REGULAR)
			m_wOutline.SetVisible(false);
		else {
			m_wOutline.SetVisible(true);
			iconString = iconString + "_ICON";
		};
		
		m_wIcon.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, iconString);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetArrowWidget()
	{
		// do settings check here
		CSI_EArrowTheme arrow = m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.ARROW_THEME);
		
		string arrowString = string.Format("ARROW %1", SCR_Enum.GetEnumName(CSI_EArrowTheme, arrow));
		
		m_wArrow.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, arrowString);
	}
}