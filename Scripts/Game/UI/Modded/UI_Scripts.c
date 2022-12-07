class COAL_GUI_Compass: SCR_InfoDisplay
{
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	private ProgressBarWidget StamBar = null;
	private SCR_CharacterControllerComponent m_cCharacterController = null;
	private bool stamBarVisible = false;
	
	override void OnStartDraw(IEntity owner)
	{
		IEntity player = GetGame().GetPlayerController();
		super.OnStartDraw(owner);
		if (!player) { Print("no player found"); return; }
		if (!m_wBearing) m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
		if (!m_wCompass) m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
		
		StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
        m_cCharacterController = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
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
	    if (!m_cCharacterController)
        {
            IEntity player = SCR_PlayerController.GetLocalControlledEntity();
            if (!player) {
                return;
            }
        
            m_cCharacterController = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
            if (!m_cCharacterController) return;
        };
        
        OnStaminaChange(m_cCharacterController.GetStamina());
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
	
	// Slowly fades the opacity of the bar
	// TO-DO: Implement this
	void fadeBar(ProgressBarWidget widget, float goToOpacity, int time) 
	{
		float currentOpacity = widget.GetOpacity();
		float change = (currentOpacity + goToOpacity) / time; //fix
		
		for(int i = 0; i < time; i++) 
		{
			widget.SetOpacity(change);
			//Print("Opacity: " + widget.GetOpacity());
			Sleep(1000); //sleep 1s
		}
		
	}
	
	void revealBar(ProgressBarWidget widget) 
	{
		stamBarVisible = true;
		widget.SetOpacity(0.5);
		//fadeBar(StamBar, 0.5, 2);
	}
	
	protected void hideBar(ProgressBarWidget widget) 
	{
		stamBarVisible = false;
		widget.SetOpacity(0);
		//fadeBar(StamBar, 0, 2);
	}
	
	void OnStaminaChange(float value)
    {
		StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
		StamBar.SetCurrent(value);
		//Print("Stamina: " + value);
		//Print("stamBarVisible: " + stamBarVisible);
		
		// Color
		// I couldnt get the switch statement to work. I hate this.
		if (value < 0.7 && value > 0.3)
		{
			StamBar.SetColor(new Color(0.091997,0.083009,0.035996,1.000000));
		} else if (value < 0.3) 
		{
			StamBar.SetColor(new Color(0.188724,0.046860,0.037476,1.000000));
		} else // Aka above 70%
		{
			StamBar.SetColor(new Color(0.035996,0.091997,0.050004,1.000000));
		}
		
		// Opacity
		if (value < 1 && !stamBarVisible) 
		{
			revealBar(StamBar);
		} else if (value == 1 && stamBarVisible) 
		{
			hideBar(StamBar);
		}
    }
};