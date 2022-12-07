class TNK_StamBar: SCR_InfoDisplay
{
	private ProgressBarWidget StamBar = null;
	private SCR_CharacterControllerComponent m_cCharacterController = null;
	private bool stamBarVisible = false;
	
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
		Print("Stamina: " + value);
		Print("stamBarVisible: " + stamBarVisible);
		
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
    
    override event void OnStartDraw(IEntity owner)
    {
        super.OnStartDraw(owner);
		Print("Stamina script running");

        StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
		
        IEntity player = GetGame().GetPlayerController();
        if (!player) {
            Print("no player found");
            return;
        }
        
        m_cCharacterController = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
    }
	
	override event void UpdateValues(IEntity owner, float timeSlice)
    {
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
}