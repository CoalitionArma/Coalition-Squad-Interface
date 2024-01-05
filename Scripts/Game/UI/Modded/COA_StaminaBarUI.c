class COA_StaminaBar : SCR_InfoDisplay
{
	private ProgressBarWidget StamBar = null;
	private bool stamBarEnabled = true;

	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleStamina", EActionTrigger.DOWN, ToggleStamina);
		GetGame().GetInputManager().ActivateContext("CoalitionSquadInterfaceContext", 36000000);
	}

	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);

		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());

		//Refresh base widget
		if (!StamBar) {
			StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
		};
		
		// Can't run if these dont exist better exit out.
		if (!character || !StamBar) return;

		// Get Charachter Controller of the locally played entity.
		SCR_CharacterControllerComponent m_cCharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));

		if (!m_cCharacterController) return;

		// Use local Charachter Controller to get the current players stamina, then use custom function OnStaminaChange() to show current stamina on players screen.
		OnStaminaChange(m_cCharacterController.GetStamina());
	}
	
	protected void ToggleStamina()
	{
		stamBarEnabled = !stamBarEnabled;
	}

	void RevealBar(float currentOpacity)
	{
		float setOpacity = currentOpacity +0.005;
		StamBar.SetOpacity(setOpacity);
	}

	protected void HideBar(float currentOpacity)
	{
		float setOpacity = currentOpacity -0.005;
		StamBar.SetOpacity(setOpacity);
	}

	void OnStaminaChange(float stamina)
	{
		if (!stamBarEnabled)
		{
			StamBar.SetOpacity(0);
			return;
		}
		StamBar.SetCurrent(stamina);

		// Color
		switch (true)
		{
			case (stamina < 0.7 && stamina > 0.3) : { StamBar.SetColor(new Color(0.091997, 0.083009, 0.035996, 1.000000)); break; };
			case (stamina < 0.3)                  : { StamBar.SetColor(new Color(0.188724, 0.046860, 0.037476, 1.000000)); break; };
			default                               : { StamBar.SetColor(new Color(0.035996, 0.091997, 0.050004, 1.000000)); break; };
		};
		
		float currentOpacity = StamBar.GetOpacity();

		// Opacity
		switch (true)
		{
			case (stamina < 1 && currentOpacity < 0.82) : { RevealBar(currentOpacity); break;};
			case (stamina == 1 && currentOpacity > 0)   : { HideBar(currentOpacity);   break;};
		};
	}
}
