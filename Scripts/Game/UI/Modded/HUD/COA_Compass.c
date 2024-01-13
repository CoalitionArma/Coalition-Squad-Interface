class COA_Compass : SCR_InfoDisplay
{
	private TextWidget m_wBearing = null;
	private ImageWidget m_wCompass = null;
	private bool m_bCompassVisible = true;

	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("Togglem_wCompass", EActionTrigger.DOWN, Togglem_wCompass);
	}

	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);

		//Refresh base widgets if we can't get them
		if (!m_wBearing || !m_wCompass) {
			m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
		};
		
		if (m_bCompassVisible)
		{
			m_wCompass.SetOpacity(1);
			m_wBearing.SetOpacity(1);
		}
		else
		{
			m_wCompass.SetOpacity(0);
			m_wBearing.SetOpacity(0);
			return;
		}

		// Can't run if these dont exist better exit out.
		if (!m_wBearing || !m_wCompass) return;

		// Sets m_wBearings text and the m_wCompass direction
		Setm_wBearingAndm_wCompass(m_wBearing, m_wCompass);
	}

	//------------------------------------------------------------------------------------------------

	// m_wCompass Functions

	//------------------------------------------------------------------------------------------------

	protected void Togglem_wCompass()
	{
		m_bCompassVisible = !m_bCompassVisible;
	}

	protected void Setm_wBearingAndm_wCompass(TextWidget m_wBearingRef, ImageWidget m_wCompassRef)
	{
		// Get current world
		World world = GetGame().GetWorld();
		if (!world) return;

		// Settup transform
		vector transform[4];
		// Get current vector camera is looking at
		world.GetCurrentCamera(transform);

		// Creates rotation matrix from angles (yaw, pitch, roll in degrees)
		int yaw = Math3D.MatrixToAngles(transform)[0];
		// Returns absolute value of yaw
		if (yaw < 0) { yaw = 360 - Math.AbsFloat(yaw); }

		// Rotate m_wCompass texture to current m_wBearing direction (yaw)
		m_wCompassRef.SetRotation(-yaw);

		// Set m_wBearing so if yaw outputs 6 it'll read 006
		string m_wBearingAdd = "";
		// I love this.
		if (yaw >= 0 & yaw < 10) { m_wBearingAdd = "00"; }
		if (yaw >= 10 & yaw < 100) { m_wBearingAdd = "0"; }
		string m_wBearingSTR = m_wBearingAdd + (yaw.ToString());

		// Set m_wBearing Text
		m_wBearingRef.SetText(m_wBearingSTR);
	}
}
