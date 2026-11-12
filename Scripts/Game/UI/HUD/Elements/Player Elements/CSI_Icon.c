//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	One player icon (HUD group list and radar): role icon, facing arrow and color team, refreshed from the
//! 				player's CSI_PlayerData and the settings invokers.
//! 
//------------------------------------------------------------------------------------------------
class CSI_Icon : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerData m_PlayerData;
	protected int m_iPlayerId;

	protected ImageWidget m_wArrow;
	protected ImageWidget m_wIcon;
	protected ImageWidget m_wOutline;
	protected ImageWidget m_wIconLarge;		//!< used for vehicle seat icons
	protected ImageWidget m_wOutlineLarge;
	protected FrameWidget m_wIconFrame;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox)]
	bool m_bIsRadarWidget;	//!< radar icons show the arrow on turreted seats

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

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

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Points the icon at a player and subscribes to their data. Does nothing if it already shows that player.
	//! \param[in] playerID <= 0 hides the icon
	void IconUpdate(int playerID)
	{
		if (m_iPlayerId != playerID)
		{
			m_iPlayerId = playerID;

			// Stop listening to the previous player's data
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
	//! Applies the player's display icon and color team. Called on player data and settings changes.
	protected void DataUpdate()
	{
		if (!m_PlayerData)
			return;

		SetIconWidget(m_PlayerData.GetDisplayIcon());

		m_wIconFrame.SetColor(CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam()));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Rotates the arrow, and the icon too unless RADAR_ICON_ARROWS_ROTATE is on.
	//! \param[in] yaw degrees
	void SetRotation(float yaw)
	{
		m_wArrow.SetRotation(yaw);

		if (m_SettingsManager.GetSettingBool(CSI_GameSettings.ONLY_RADAR_ICON_ARROWS_ROTATE))
			yaw = 0;

		m_wIcon.SetRotation(yaw);
		m_wOutline.SetRotation(yaw);
		m_wIconLarge.SetRotation(yaw);
		m_wOutlineLarge.SetRotation(yaw);
	};

	//------------------------------------------------------------------------------------------------
	//! Shows the icon in the right widget set: large widgets for vehicle seats, regular ones otherwise.
	//! \param[in] icon
	protected void SetIconWidget(CSI_EIcon icon)
	{
		// Simple icons for the INVERTED type and for all vehicle seats
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

			// Turreted seats on the radar show the arrow (turret direction) instead of the outline
			bool isTurreted = (icon == CSI_EIcon.GUNNER || icon == CSI_EIcon.COMMANDER);

			if (isTurreted && m_bIsRadarWidget)
			{
				m_wArrow.SetVisible(true);
				m_wOutlineLarge.SetVisible(false);
			} else {
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

		iconToUse.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, iconString);

		SetArrowWidget();
	}

	//------------------------------------------------------------------------------------------------
	//! Loads the arrow image for the ARROW_THEME setting.
	protected void SetArrowWidget()
	{
		CSI_EArrowTheme arrow = m_SettingsManager.GetSettingInt(CSI_GameSettings.ARROW_THEME);

		string arrowString = string.Format("ARROW %1", SCR_Enum.GetEnumName(CSI_EArrowTheme, arrow));

		m_wArrow.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, arrowString);
	}
}