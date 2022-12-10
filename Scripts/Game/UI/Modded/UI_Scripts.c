class COAL_GUI: SCR_InfoDisplay
{
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Variables used on/for UI Components
	
	//---------------------------------------------------------------------------------------------------------------------------------
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	private ProgressBarWidget StamBar = null;
	private SCR_CharacterControllerComponent m_cCharacterController = null;
	private bool stamBarVisible = false;
	static Widget s_wDebugLayout;
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// When player is created
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		// Re-aquire UI widgets if they dont exist at the moment.
		if (!m_wBearing) m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
		if (!m_wCompass) m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
		if (!StamBar) StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
    }
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Main loop that repeats every frame
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		// Can't run shit if these dont exist better exit out quick.
		if (!character || !m_wBearing || !m_wCompass) return;
	
		// Passes m_wBearing & m_wCompass onto the custom function SetBearingSetCompass().
		SetBearingSetCompass(m_wBearing, m_wCompass);
		
		// Get Charachter Controller for locally played entity.
		 m_cCharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		
		if (!m_cCharacterController ) return;
		// Use local Charachter Controller to get the current entity stamina, then use custom function OnStaminaChange() to show current stamina on players screen.
        OnStaminaChange(m_cCharacterController.GetStamina());
		
		// Get local played entitys PlayerID.
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		
		// Groups may not initilize until a second after players locally controlled entity spawns, CallLater allows us to call it in exactly 1000 miliseconds after players locally controlled entity spawns.
		//GetGame().GetCallqueue().CallLater(GroupDisplay, 1000, false, playerId);
	}
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Functions used for the UI
	
	//---------------------------------------------------------------------------------------------------------------------------------
	protected void GroupDisplay(int playerId)
	{
		// Method returns null, tested in coop world
		//SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByID(playerId));
		//SCR_AIGroup group = GetGroup(playerId);

     	//Print("Debug: " + group);
    	//foreach (AIAgent agent : agents)
    	//{    
    	//    Print("Group Member: " + agent);
    	//}
	}
	
	protected void SetBearingSetCompass(TextWidget Bearing, ImageWidget Compass)
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
		Compass.SetRotation(-yaw);
		
		// Set Bearing so if yaw outputs 6 it'll read 006
		string Bearing_add = "";
		// I love this.
		if (yaw >= 0 & yaw < 10 ) { Bearing_add = "00"; }
		if (yaw >= 10 & yaw < 100) { Bearing_add = "0"; }
		string Bearing_STR = Bearing_add + (yaw.ToString());
		
		// Set Bearing Text
		Bearing.SetText(Bearing_STR);
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