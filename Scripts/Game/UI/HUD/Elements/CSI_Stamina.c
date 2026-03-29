class CSI_Stamina : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	const static ref Color STAM_GREEN = new Color(0.035996, 0.091997, 0.050004, 1.000000);
	const static ref Color STAM_YELLOW = new Color(0.091997, 0.083009, 0.035996, 1.000000);
	const static ref Color STAM_RED = new Color(0.188724, 0.046860, 0.037476, 1.000000);
	
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;
	
	protected ProgressBarWidget m_wStamBar;
		
	protected Color m_CurrentColor;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();
		
		m_wStamBar = ProgressBarWidget.Cast(w.FindWidget("StamBar"));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	//! Update the stamina UI element each frame.
	void Update()
	{
		if (!m_SettingsManager.GetSettingBool(CSI_GameSettings.STAMINA_VISIBLE))
		{
			HideBar(0);
			return;
		}
		
		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = m_HUDSystem.GetLocalPlayerCharacter();

		// Can't run if these dont exist better exit out.
		if (!character) 
			return;

		// Get Charachter Controller of the locally played entity.
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));

		if (!characterController) 
			return;

		// Use local Charachter Controller to get the current players stamina, then use custom function OnStaminaChange() to show current stamina on players screen.
		OnStaminaChange(characterController.GetStamina());
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	void RevealBar(float currentOpacity)
	{
		float setOpacity = Math.Clamp(currentOpacity + 0.005, 0.0, 1.0);
		m_wStamBar.SetOpacity(setOpacity);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideBar(float currentOpacity)
	{
		float setOpacity = Math.Clamp(currentOpacity - 0.005, 0.0, 1.0);
		m_wStamBar.SetOpacity(setOpacity);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the color and opacity of the samina bar widget.
	//! \param[in] stamina stamina value to set the bar to
	void OnStaminaChange(float stamina)
	{
		m_wStamBar.SetCurrent(stamina); 

		// Color
		switch (true)
		{
			case (stamina > 0.7 && m_CurrentColor != STAM_GREEN)					: { m_wStamBar.SetColor(STAM_GREEN); m_CurrentColor = STAM_GREEN;		break;};
			case (stamina < 0.7 && stamina > 0.3 && m_CurrentColor != STAM_YELLOW)	: { m_wStamBar.SetColor(STAM_YELLOW); m_CurrentColor = STAM_YELLOW; 	break;};
			case (stamina < 0.3 && m_CurrentColor != STAM_RED)						: { m_wStamBar.SetColor(STAM_RED); m_CurrentColor = STAM_RED;			break;};
		};

		float currentOpacity = m_wStamBar.GetOpacity();

		// Opacity
		switch (true)
		{
			case (stamina < 1 && currentOpacity < 1)  : { RevealBar(currentOpacity); break;};
			case (stamina == 1 && currentOpacity > 0) : { HideBar(currentOpacity); break;};
		};
	}
}
