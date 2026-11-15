//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD compass: the COMPASS_THEME setting's texture rotated to the local yaw, and a three-digit bearing
//! 				above it, both in the COMPASS_COLOR setting's colour.
//! 
//------------------------------------------------------------------------------------------------
class CSI_Compass : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected string m_sStoredCompass;	//!< currently loaded texture, to avoid reloading it every frame
	protected int m_iStoredCompassColor = -1;	//!< COMPASS_COLOR value applied, to avoid recolouring every frame
	protected float m_fStoredYaw;
	protected int m_iStoredBearing = -2;	//!< bearing text shown (-1 hidden, -2 not set yet), to avoid setting it every frame

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
		m_wBearing = TextWidget.Cast(w.FindAnyWidget("Bearing"));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD.
	void Update()
	{
		if (!m_wCompass || !m_wBearing)
			return;

		int yawInt;
        float yaw = m_HUDSystem.GetLocalYaw();
		yawInt = -yaw;

		// The compass and its bearing take the COMPASS_COLOR setting's colour
		int compassColor = m_SettingsManager.GetSettingInt(CSI_GameSettings.COMPASS_COLOR);
		if (m_iStoredCompassColor != compassColor)
		{
			Color color = CSI_ColorPresets.GetColor(compassColor);
			m_wCompass.SetColor(color);
			m_wBearing.SetColor(color);
			m_iStoredCompassColor = compassColor;
		}

		if (m_SettingsManager.GetSettingBool(CSI_GameSettings.COMPASS_VISIBLE))
		{
			CSI_ECompassTheme compassTheme = m_SettingsManager.GetSettingInt(CSI_GameSettings.COMPASS_THEME);
			string compassImage = CSI_UIHelper.GetCompassTexture(compassTheme);

			if (m_sStoredCompass != compassImage)
			{
				m_wCompass.LoadImageTexture(0, compassImage);
				m_sStoredCompass = compassImage;
			};

			m_wCompass.SetVisible(true);

			if (m_fStoredYaw != yaw)
			{
				m_wCompass.SetRotation(yaw);
				m_fStoredYaw = yaw;
			};
		} else
			m_wCompass.SetVisible(false);

		if (m_SettingsManager.GetSettingBool(CSI_GameSettings.BEARING_VISIBLE))
		{
			// Wrap to 0-359; ToString(3) zero-pads to three digits
			if (yawInt < 0)
				yawInt += 360;

			if (m_iStoredBearing != yawInt)
			{
				m_wBearing.SetText(yawInt.ToString(3));
				m_iStoredBearing = yawInt;
			}
		} else if (m_iStoredBearing != -1)
		{
			m_wBearing.SetText("");
			m_iStoredBearing = -1;
		}
	}
}
