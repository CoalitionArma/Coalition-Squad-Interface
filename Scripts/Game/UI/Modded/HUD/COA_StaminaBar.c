class COA_StaminaBar : SCR_InfoDisplay
{
	protected ProgressBarWidget m_wStamBar = null;
	protected bool m_bStamBarEnabled = true;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleStamina", EActionTrigger.DOWN, ToggleStamina);
		GetGame().GetInputManager().ActivateContext("CoalitionSquadInterfaceContext", 36000000);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);

		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());

		//Refresh base widget
		if (!m_wStamBar) {
			m_wStamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
		};
		
		// Can't run if these dont exist better exit out.
		if (!character || !m_wStamBar) return;

		// Get Charachter Controller of the locally played entity.
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));

		if (!characterController) return;

		// Use local Charachter Controller to get the current players stamina, then use custom function OnStaminaChange() to show current stamina on players screen.
		OnStaminaChange(characterController.GetStamina());
	}
	
	//------------------------------------------------------------------------------------------------

	// Stamina Bar Functions

	//------------------------------------------------------------------------------------------------
	
	protected void ToggleStamina()
	{
		m_bStamBarEnabled = !m_bStamBarEnabled;
	}

	//------------------------------------------------------------------------------------------------
	void RevealBar(float currentOpacity)
	{
		float setOpacity = currentOpacity +0.005;
		m_wStamBar.SetOpacity(setOpacity);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideBar(float currentOpacity)
	{
		float setOpacity = currentOpacity -0.005;
		m_wStamBar.SetOpacity(setOpacity);
	}

	//------------------------------------------------------------------------------------------------
	void OnStaminaChange(float stamina)
	{
		if (!m_bStamBarEnabled)
		{
			m_wStamBar.SetOpacity(0);
			return;
		}
		m_wStamBar.SetCurrent(stamina);

		// Color
		switch (true)
		{
			case (stamina < 0.7 && stamina > 0.3) : { m_wStamBar.SetColor(new Color(0.091997, 0.083009, 0.035996, 1.000000)); break; };
			case (stamina < 0.3)                  : { m_wStamBar.SetColor(new Color(0.188724, 0.046860, 0.037476, 1.000000)); break; };
			default                               : { m_wStamBar.SetColor(new Color(0.035996, 0.091997, 0.050004, 1.000000)); break; };
		};
		
		float currentOpacity = m_wStamBar.GetOpacity();

		// Opacity
		switch (true)
		{
			case (stamina < 1 && currentOpacity < 0.82) : { RevealBar(currentOpacity); break;};
			case (stamina == 1 && currentOpacity > 0)   : { HideBar(currentOpacity);   break;};
		};
	}
}
