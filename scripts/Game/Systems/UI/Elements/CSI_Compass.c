class CSI_Compass : SCR_ScriptedWidgetComponent
{
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wCompass = ImageWidget.Cast(w.FindAnyWidget("Compass"));
		m_wBearing = TextWidget.Cast(w.FindAnyWidget("Bearing"));
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{		
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
