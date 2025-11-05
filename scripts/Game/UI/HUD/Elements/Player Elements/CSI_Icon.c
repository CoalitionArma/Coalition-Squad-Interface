class CSI_Icon : SCR_ScriptedWidgetComponent
{	
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerData m_PlayerData;
	protected int m_iPlayerId;
	
	protected ImageWidget m_wArrow;
	protected ImageWidget m_wIcon;
	protected ImageWidget m_wOutline;
	protected ImageWidget m_wIconLarge;
	protected ImageWidget m_wOutlineLarge;
	protected FrameWidget m_wIconFrame;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Insert(DataUpdate);
		
		m_wArrow = ImageWidget.Cast(w.FindAnyWidget("Arrow"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wOutline = ImageWidget.Cast(w.FindAnyWidget("Outline"));
		m_wIconLarge = ImageWidget.Cast(w.FindAnyWidget("Icon_Large"));
		m_wOutlineLarge = ImageWidget.Cast(w.FindAnyWidget("Outline_Large"));
		m_wIconFrame = FrameWidget.Cast(w.FindAnyWidget("IconFrame"));
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates the icon state for a specific player
	 * @param playerID The ID of the player to update the icon for
	 */
	void IconUpdate(int playerID)
	{
		if (m_iPlayerId != playerID)
		{
			m_iPlayerId = playerID;
			
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);
			
			if (playerID <= 0)
			{
				m_wRoot.SetVisible(false);
				return;
			};
			
			m_wRoot.SetVisible(true);
			m_PlayerData = m_PlayerDataManager.GetPlayerData(playerID);
			
			if (!m_PlayerData)
				return;
			
			m_PlayerData.GetOnDataUpdate().Insert(DataUpdate);
			
			DataUpdate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the rotation angle of the icon.
	 * @param yaw The rotation angle in degrees
	 */
	void SetRotation(float yaw)
	{
		m_wArrow.SetRotation(yaw);

		if (m_SettingsManager.GetSettingBool(CSI_SettingsManager.ONLY_RADAR_ICON_ARROWS_ROTATE))
			yaw = 0;

		m_wIcon.SetRotation(yaw);
		m_wOutline.SetRotation(yaw);
		m_wIconLarge.SetRotation(yaw);
		m_wOutlineLarge.SetRotation(yaw);
	};
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates the visual data representation of the player's icon.
	 */
	protected void DataUpdate()
	{	
		if (!m_PlayerData)
			return;
		
		SetIconWidget(m_PlayerData.GetDisplayIcon());
		
		m_wIconFrame.SetColor(CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam())); 
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the icon type for the widget.
	 * @param icon The enum icon type to be set
	 */
	protected void SetIconWidget(CSI_EIcon icon)
	{		
		bool simpleIcon = true;
		if (m_SettingsManager.GetSettingInt(CSI_GameSettings.ICON_TYPE) == CSI_EIconType.REGULAR && !CSI_UIHelper.m_aVehicleIcons.Contains(icon))
			simpleIcon = false;
		
		ImageWidget iconToUse;
		string iconString = CSI_UIHelper.GetIconString(icon, simpleIcon);
		
		if (CSI_UIHelper.m_aVehicleIcons.Contains(icon))
		{
			m_wOutline.SetVisible(false);
			m_wIcon.SetVisible(false);
			
			iconToUse = m_wIconLarge;
			
			m_wIconLarge.SetVisible(true);
			
			if (icon != CSI_EIcon.GUNNER)
			{
				m_wArrow.SetVisible(false);
				m_wOutlineLarge.SetVisible(true);
			};
		} else {
			m_wIconLarge.SetVisible(false);
			m_wOutlineLarge.SetVisible(false);
			
			iconToUse = m_wIcon;
			
			m_wArrow.SetVisible(true);
			m_wIcon.SetVisible(true);
			
			if (simpleIcon)
				m_wOutline.SetVisible(true);
			else
				m_wOutline.SetVisible(false);
		};
		
		iconToUse.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, iconString);
		
		SetArrowWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets up the arrow imagewidget within the icon.
	 */
	protected void SetArrowWidget()
	{
		// do settings check here
		CSI_EArrowTheme arrow = m_SettingsManager.GetSettingInt(CSI_GameSettings.ARROW_THEME);
		
		string arrowString = string.Format("ARROW %1", SCR_Enum.GetEnumName(CSI_EArrowTheme, arrow));
		
		m_wArrow.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, arrowString);
	}
}