class COA_StaminaBar : SCR_InfoDisplay
{
	private ProgressBarWidget StamBar = null;
	private bool stamBarVisible = true;
	private bool stamBarEnabled = true;
	private int widgetRefresh = 360;
	
	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleStamina", EActionTrigger.DOWN, ToggleStamina);
		GetGame().GetInputManager().ActivateContext("CoalitionSquadInterfaceContext",3600000);

	}
	
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		
		super.UpdateValues(owner, timeSlice);
		
		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		//Refresh base widgets every 360 frames
		if (widgetRefresh >= 360) {
			widgetRefresh = 0;
			StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
		} else {
			widgetRefresh++;
		};
		
		// Can't run if these dont exist better exit out.
		if (!character || !StamBar) return;
	
		// Get Charachter Controller of the locally played entity.
		SCR_CharacterControllerComponent m_cCharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		
		if (!m_cCharacterController ) return;
		
		// Use local Charachter Controller to get the current players stamina, then use custom function OnStaminaChange() to show current stamina on players screen.
        OnStaminaChange(m_cCharacterController.GetStamina(), StamBar);
		
	}
	
	void FadeBar(ProgressBarWidget widget, float goToOpacity, int time) 
	{
		float currentOpacity = widget.GetOpacity();
		float change = (currentOpacity + goToOpacity) / time; //fix
		
		for(int i = 0; i < time; i++) 
		{
			widget.SetOpacity(change);
			Sleep(1000); //sleep 1s
		}
	}
	
	protected void ToggleStamina()
	{
		stamBarEnabled = !stamBarEnabled;
		if(stamBarEnabled)
		{
			StamBar.SetOpacity(1);
		}
		else
		{
			StamBar.SetOpacity(0);
		}
	}
	
	
	void RevealBar(ProgressBarWidget widget) 
	{
		if(stamBarEnabled)
		{
		stamBarVisible = true;
		widget.SetOpacity(0.5);
		}
		//FadeBar(stamBarRef, 0.5, 2);
	}
	
	protected void HideBar(ProgressBarWidget widget) 
	{
		if(stamBarEnabled)
		{
		stamBarVisible = false;
		widget.SetOpacity(0);
		}

		//FadeBar(stamBarRef, 0, 2);
	}
	
	void OnStaminaChange(float value, ProgressBarWidget stamBarRef)
    {
		if(!stamBarEnabled)
		{
			stamBarRef.SetOpacity(0);
			return;
		}
		stamBarRef.SetCurrent(value);
		//Print("Stamina: " + value);
		//Print("stamBarVisible: " + stamBarVisible);
		
		// Color
		// I couldnt get the switch statement to work. I hate this.
		if (value < 0.7 && value > 0.3)
		{
			stamBarRef.SetColor(new Color(0.091997,0.083009,0.035996,1.000000));
		} else if (value < 0.3) 
		{
			stamBarRef.SetColor(new Color(0.188724,0.046860,0.037476,1.000000));
		} else // Aka above 70%
		{
			stamBarRef.SetColor(new Color(0.035996,0.091997,0.050004,1.000000));
		}
		
		// Opacity
		if (value < 1 && !stamBarVisible) 
		{
			RevealBar(stamBarRef);
		} else if (value == 1 && stamBarVisible) 
		{
			HideBar(stamBarRef);
		}
    }
}