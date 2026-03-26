class CSI_Compass : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_ECompassTheme m_StoredCompassTheme;
	protected float m_fStoredYaw;
	
	protected CSI_HUDSystem m_HUDSystem;
	
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	
	protected bool m_bCompassVisible;
	protected bool m_bBearingVisible;
	protected CSI_ECompassTheme m_CompassTheme;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Insert(OnSettingsUpdate);
		OnSettingsUpdate();
		
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
		float yaw = m_HUDSystem.GetLocalYaw();
		int negYaw = -yaw;
		
		if (m_bCompassVisible)
		{
			if (m_StoredCompassTheme != m_CompassTheme)
			{
				m_wCompass.LoadImageTexture(0, CSI_UIHelper.GetCompassThemeResource(m_CompassTheme));
				m_StoredCompassTheme = m_CompassTheme;
			};
			
			m_wCompass.SetVisible(true);
		} else 
			m_wCompass.SetVisible(false);

		if (m_bBearingVisible)
		{
			if (m_fStoredYaw != yaw)
			{
				string bearingAdd = "";
				
				if (negYaw < 0)
					negYaw = 360 - Math.AbsInt(negYaw);
				
				if (negYaw >= 0 & negYaw < 10)
					bearingAdd = "00";
				
				if (negYaw >= 10 & negYaw < 100)
					bearingAdd = "0";
				
				m_wBearing.SetText(bearingAdd + (negYaw.ToString()));
			};
		} else
			m_wBearing.SetText("");
		
		if (m_fStoredYaw != yaw)
		{
			m_wCompass.SetRotation(yaw);
			m_fStoredYaw = yaw;
		};
	}
	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTINGS UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	protected void OnSettingsUpdate()
	{
		CSI_SettingsManager settingsManager = CSI_SettingsManager.GetInstance();
		
		m_bCompassVisible = settingsManager.GetSettingBool(CSI_GameSettings.COMPASS_VISIBLE);
		m_bBearingVisible = settingsManager.GetSettingBool(CSI_GameSettings.BEARING_VISIBLE);
		m_CompassTheme = settingsManager.GetSettingInt(CSI_GameSettings.COMPASS_THEME);
	}
}
