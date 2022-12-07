class COAL_GUI_Compass: SCR_InfoDisplay
{
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	
	override void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		if (!m_wBearing) m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
		if (!m_wCompass) m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
	}
	
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		if (!m_wBearing ||!m_wCompass ) return;
		private float yaw = GetBearing();
		string Bearing_add = "";
		if (yaw >= 0 & yaw < 10 ) { Bearing_add = "00"; }
		if (yaw >= 10 & yaw < 100) { Bearing_add = "0"; }
		m_wBearing.SetText(Bearing_add + yaw.ToString());
		m_wCompass.SetRotation(-(GetBearing()));
	}
	
	protected int GetBearing()
	{
		World world = GetGame().GetWorld();
		if (!world) return 0;
		vector transform[4];
		world.GetCurrentCamera(transform);
		float yaw = Math3D.MatrixToAngles(transform)[0];
		if (yaw < 0) { yaw = 360 - Math.AbsFloat(yaw); }
		return yaw;
	}
};