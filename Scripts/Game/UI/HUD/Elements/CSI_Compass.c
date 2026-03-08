class CSI_Compass : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected string m_sStoredCompass;
	protected float m_fStoredYaw;
	
	protected CSI_SettingsSystem m_SettingsSystem;
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
		
		m_SettingsSystem = CSI_SettingsSystem.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();
		
		m_wCompass = ImageWidget.Cast(w.FindAnyWidget("Compass"));
		m_wBearing = TextWidget.Cast(w.FindAnyWidget("Bearing"));
	}
	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Updates the compass HUD element state and position each frame
	void Update()
	{		
		int yawInt;
        float yaw = m_HUDSystem.GetLocalYaw();
		yawInt = -yaw;
		
		if (m_SettingsSystem.GetSettingBool(CSI_GameSettings.COMPASS_VISIBLE))
		{
			CSI_ECompassTheme compassTheme = m_SettingsSystem.GetSettingInt(CSI_GameSettings.COMPASS_THEME);
			string compassImage;
			switch (compassTheme)
			{
				default : compassImage = CSI_UIHelper.CSI_STANDARD_COMPASS; break;
			};
			
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

		if (m_SettingsSystem.GetSettingBool(CSI_GameSettings.BEARING_VISIBLE))
		{
			string bearingAdd = "";
			
			if (yawInt < 0)
				yawInt = 360 - Math.AbsInt(yawInt);
			
			if (yawInt >= 0 & yawInt < 10)
				bearingAdd = "00";
			
			if (yawInt >= 10 & yawInt < 100)
				bearingAdd = "0";
			
			m_wBearing.SetText(bearingAdd + (yawInt.ToString()));
		} else
			m_wBearing.SetText("");
	}
}
