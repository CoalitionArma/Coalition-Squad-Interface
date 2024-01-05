class COA_Compass : SCR_InfoDisplay
{
	private TextWidget Bearing = null;
	private ImageWidget Compass = null;
	private bool CompassVisible = true;

	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleCompass", EActionTrigger.DOWN, ToggleCompass);
	}

	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);

		//Refresh base widgets if we can't get them
		if (!Bearing || !Compass) {
			Bearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			Compass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
		};
		
		if (CompassVisible)
		{
			Compass.SetOpacity(1);
			Bearing.SetOpacity(1);
		}
		else
		{
			Compass.SetOpacity(0);
			Bearing.SetOpacity(0);
			return;
		}

		// Can't run if these dont exist better exit out.
		if (!Bearing || !Compass) return;

		// Sets Bearings text and the Compass direction
		SetBearingSetCompass(Bearing, Compass);
	}

	//------------------------------------------------------------------------------------------------

	// Compass Functions

	//------------------------------------------------------------------------------------------------

	protected void ToggleCompass()
	{
		CompassVisible = !CompassVisible;
	}

	protected void SetBearingSetCompass(TextWidget BearingRef, ImageWidget CompassRef)
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

		// Rotate Compass texture to current bearing direction (yaw)
		CompassRef.SetRotation(-yaw);

		// Set Bearing so if yaw outputs 6 it'll read 006
		string bearingAdd = "";
		// I love this.
		if (yaw >= 0 & yaw < 10) { bearingAdd = "00"; }
		if (yaw >= 10 & yaw < 100) { bearingAdd = "0"; }
		string bearingSTR = bearingAdd + (yaw.ToString());

		// Set Bearing Text
		BearingRef.SetText(bearingSTR);
	}
}
