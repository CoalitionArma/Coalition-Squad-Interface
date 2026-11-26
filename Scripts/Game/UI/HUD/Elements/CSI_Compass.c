//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD compass: themed texture rotated to the local yaw, with the bearing above it.
//! 				Both use the Compass color setting. CSI_HUD has it read its settings (ApplySettings).
//! 
//------------------------------------------------------------------------------------------------
class CSI_Compass : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected float m_fStoredYaw;
	protected int m_iStoredBearing = -2;	//!< bearing shown; -1 hidden, -2 not set yet
	protected bool m_bCompassVisible;		//!< COMPASS_VISIBLE
	protected bool m_bBearingVisible;		//!< BEARING_VISIBLE

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;

	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();

		m_wCompass = ImageWidget.Cast(w.FindAnyWidget("Compass"));
		if (!m_wCompass)
			Print("CSI_Compass: image 'Compass' not found, the compass and bearing won't show", LogLevel.WARNING);

		m_wBearing = TextWidget.Cast(w.FindAnyWidget("Bearing"));
		if (!m_wBearing)
			Print("CSI_Compass: text 'Bearing' not found, the compass and bearing won't show", LogLevel.WARNING);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Applies the compass settings: color, theme, and whether the compass and bearing show. Called by CSI_HUD on
	//! a settings update.
	void ApplySettings()
	{
		if (!m_wCompass || !m_wBearing || !m_SettingsManager)
			return;

		Color color = CSI_ColorPresets.GetColor(m_SettingsManager.GetSettingInt(CSI_GameSettings.COMPASS_COLOR));
		m_wCompass.SetColor(color);
		m_wBearing.SetColor(color);

		m_bCompassVisible = m_SettingsManager.GetSettingBool(CSI_GameSettings.COMPASS_VISIBLE);
		if (m_bCompassVisible)
		{
			CSI_ECompassTheme compassTheme = m_SettingsManager.GetSettingInt(CSI_GameSettings.COMPASS_THEME);
			m_wCompass.LoadImageTexture(0, CSI_UIHelper.GetCompassTexture(compassTheme));
		}

		m_wCompass.SetVisible(m_bCompassVisible);

		// Update sets the text again when the bearing is shown
		m_bBearingVisible = m_SettingsManager.GetSettingBool(CSI_GameSettings.BEARING_VISIBLE);
		if (!m_bBearingVisible)
		{
			m_wBearing.SetText("");
			m_iStoredBearing = -1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD.
	void Update()
	{
		if (!m_wCompass || !m_wBearing)
			return;

		float yaw = m_HUDSystem.GetLocalYaw();

		if (m_bCompassVisible && m_fStoredYaw != yaw)
		{
			m_wCompass.SetRotation(yaw);
			m_fStoredYaw = yaw;
		};

		if (!m_bBearingVisible)
			return;

		// ToString(3) zero-pads
		int yawInt = Math.Repeat(Math.Floor(-yaw), 360);	// bearing 0-359, rounded down
		if (m_iStoredBearing != yawInt)
		{
			m_wBearing.SetText(yawInt.ToString(3));
			m_iStoredBearing = yawInt;
		}
	}
}
