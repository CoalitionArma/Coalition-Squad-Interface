class CSI_Compass : SCR_ScriptedWidgetComponent
{
	protected string m_sStoredCompass;
	
	protected CSI_SettingsManager m_SettingsManager;
	
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	protected Widget m_wRadar;
	
	protected CSI_Radar m_Radar;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		
		m_wCompass = ImageWidget.Cast(w.FindAnyWidget("Compass"));
		m_wBearing = TextWidget.Cast(w.FindAnyWidget("Bearing"));
		m_wRadar = w.FindAnyWidget("Radar");
		
		m_Radar = CSI_Radar.Cast(m_wRadar.FindHandler(CSI_Radar));
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{		
		m_Radar.Update();
		
		int yawInt;
		string bearingAdd = "";
		
		if (m_SettingsManager.GetCSISettingBool(CSI_SettingsManager.COMPASS_VISIBLE))
		{
			CSI_ECompassTheme compassTheme = m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.COMPASS_THEME);
			string compassImage;
			switch (compassTheme)
			{
				case CSI_ECompassTheme.REGULAR : compassImage = CSI_UIHelper.STANDARD_COMPASS_RESOURCE; break;
			};
			
			if (m_sStoredCompass != compassImage)
			{
				m_wCompass.LoadImageTexture(0, compassImage);
				m_sStoredCompass = compassImage;
			};
			
	        float yaw = CSI_ChararcterHelper.GetLocalAimingYaw();
			yawInt = -yaw;
			
			m_wCompass.SetVisible(true);
			m_wCompass.SetRotation(yaw);
		} else 
			m_wCompass.SetVisible(false);

		if (m_SettingsManager.GetCSISettingBool(CSI_SettingsManager.BEARING_VISIBLE))
		{
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
