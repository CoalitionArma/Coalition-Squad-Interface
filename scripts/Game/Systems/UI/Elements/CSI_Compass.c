class CSI_Compass : SCR_ScriptedWidgetComponent
{
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	protected Widget m_wRadar;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wCompass = ImageWidget.Cast(w.FindAnyWidget("Compass"));
		m_wBearing = TextWidget.Cast(w.FindAnyWidget("Bearing"));
		m_wRadar = w.FindAnyWidget("Radar");
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{		
		// Future compass visibility check goes here
		
		CSI_Radar.Cast(m_wRadar.FindHandler(CSI_Radar)).Update();
		
        float yaw = CSI_ChararcterHelper.GetLocalAimingYaw();
		int yawInt = -yaw;
		
		if (yawInt < 0)
			yawInt = 360 - Math.AbsFloat(yawInt);

		m_wCompass.SetRotation(yaw);

		string bearingAdd = "";
		if (yawInt >= 0 & yawInt < 10)
			bearingAdd = "00";
		
		if (yawInt >= 10 & yawInt < 100)
			bearingAdd = "0";

		m_wBearing.SetText(bearingAdd + (yawInt.ToString()));
	}
}
