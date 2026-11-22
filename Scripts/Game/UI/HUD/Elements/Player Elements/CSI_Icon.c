//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-22
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Player icon for the group list and radar: role icon, facing arrow and color team,
//! 				refreshed on player data and settings changes.
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
	protected ImageWidget m_wIconLarge;		//!< vehicle seat icons
	protected ImageWidget m_wOutlineLarge;
	protected FrameWidget m_wIconFrame;
	protected bool m_bHasAllParts;	//!< all widgets above were found
	protected float m_fArrowRotation;	//!< degrees last set (SetRotation)
	protected float m_fIconRotation;
	protected bool m_bOutsider;		//!< outside the local group: nametag color, no seat ring unless driving
	protected bool m_bInOutlinedVehicle;	//!< in the radar's outlined vehicle: turret seats show no arrow

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
		m_bHasAllParts = m_wArrow && m_wIcon && m_wOutline && m_wIconLarge && m_wOutlineLarge && m_wIconFrame;
		if (!m_bHasAllParts)
			Print("CSI_Icon: 'Arrow', 'Icon', 'Outline', 'Icon_Large', 'Outline_Large' or 'IconFrame' not found, the player icon won't show", LogLevel.WARNING);

		// Visible in the layout for editing; SetIconWidget shows the ones in use
		if (SCR_Global.IsEditMode())
			return;

		array<Widget> parts = {m_wArrow, m_wIcon, m_wOutline, m_wIconLarge, m_wOutlineLarge};
		foreach (Widget part : parts)
		{
			if (part)
				part.SetVisible(false);
		}
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

	//------------------------------------------------------------------------------------------------
	void SetIsRadarWidget(bool isRadarWidget)
	{
		m_bIsRadarWidget = isRadarWidget;
	}

	//------------------------------------------------------------------------------------------------
	void SetInOutlinedVehicle(bool inside)
	{
		if (inside == m_bInOutlinedVehicle)
			return;

		m_bInOutlinedVehicle = inside;
		DataUpdate();
	}

	//------------------------------------------------------------------------------------------------
	void SetOutsider(bool outsider)
	{
		if (outsider == m_bOutsider)
			return;

		m_bOutsider = outsider;
		DataUpdate();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Points the icon at a player and subscribes to their data.
	//! \param[in] playerID <= 0 hides the icon
	void IconUpdate(PlayerId playerID)
	{
		if (m_iPlayerId != playerID)
		{
			m_iPlayerId = playerID;

			// Stop listening to the previous player's data
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);

			m_PlayerData = null;

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
	//! Applies the player's display icon and color team.
	protected void DataUpdate()
	{
		if (!m_PlayerData || !m_bHasAllParts)
			return;

		SetIconWidget(m_PlayerData.GetDisplayIcon());

		// Outsiders take the nametag color, as their nametags do
		if (m_bOutsider)
		{
			int teamSet = m_SettingsManager.GetSettingInt(CSI_GameSettings.COLOR_TEAM_SET);
			int nametagPreset = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_COLOR);
			m_wIconFrame.SetColor(CSI_ColorPresets.GetColor(CSI_ColorPresets.GetNonClashingPreset(nametagPreset, teamSet)));
			return;
		}

		m_wIconFrame.SetColor(CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam()));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Rotates the arrow, and the icon too unless upright. Called every frame by the radar, so only changes are set.
	//! \param[in] yaw degrees
	//! \param[in] iconUpright RADAR_ICON_ARROWS_ROTATE: only the arrow turns, if it shows
	void SetRotation(float yaw, bool iconUpright)
	{
		if (!m_bHasAllParts)
			return;

		if (yaw != m_fArrowRotation)
		{
			m_fArrowRotation = yaw;
			m_wArrow.SetRotation(yaw);
		}

		// Without the arrow, the icon shows the facing
		if (iconUpright && m_wArrow.IsVisible())
			yaw = 0;

		if (yaw == m_fIconRotation)
			return;

		m_fIconRotation = yaw;
		m_wIcon.SetRotation(yaw);
		m_wOutline.SetRotation(yaw);
		m_wIconLarge.SetRotation(yaw);
		m_wOutlineLarge.SetRotation(yaw);
	};

	//------------------------------------------------------------------------------------------------
	//! Shows the icon: large widgets for vehicle seats, regular ones otherwise.
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

			// Radar: turreted seats show the turret arrow instead of the outline, except in the outlined vehicle
			bool isTurreted = (icon == CSI_EIcon.GUNNER || icon == CSI_EIcon.COMMANDER);

			if (isTurreted && m_bIsRadarWidget && !m_bInOutlinedVehicle)
			{
				m_wArrow.SetVisible(true);
				m_wOutlineLarge.SetVisible(false);
			} else {
				// Outsiders' seats have no ring, except the driver's
				m_wArrow.SetVisible(false);
				m_wOutlineLarge.SetVisible(!m_bOutsider || icon == CSI_EIcon.DRIVER || icon == CSI_EIcon.HELIPILOT);
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