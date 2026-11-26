//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD stamina bar: fades in while stamina is below full; its color shows the stamina level.
//! 				CSI_HUD has it read its setting (ApplySettings).
//! 
//------------------------------------------------------------------------------------------------
class CSI_Stamina : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;

	protected ProgressBarWidget m_wStamBar;
	protected Color m_StamBarColor;	//!< color applied to the bar
	protected bool m_bVisible;		//!< STAMINA_VISIBLE

	protected static const float FADE_SPEED = 0.3;	//!< opacity per second (full fade about 3 s)
	protected static const ref Color HIGH_STAMINA_COLOR = new Color(0.035996, 0.091997, 0.050004, 1.000000);		//!< green, above 70%
	protected static const ref Color MEDIUM_STAMINA_COLOR = new Color(0.091997, 0.083009, 0.035996, 1.000000);	//!< amber, 30-70%
	protected static const ref Color LOW_STAMINA_COLOR = new Color(0.188724, 0.046860, 0.037476, 1.000000);		//!< red, below 30%

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
		if (!m_wStamBar)
			Print("CSI_Stamina: progress bar 'StamBar' not found, the stamina bar won't show", LogLevel.WARNING);

		// Opaque in the layout for editing; starts hidden in game
		if (m_wStamBar && !SCR_Global.IsEditMode())
			m_wStamBar.SetOpacity(0);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Applies STAMINA_VISIBLE. Called by CSI_HUD on a settings update.
	void ApplySettings()
	{
		if (!m_wStamBar || !m_SettingsManager)
			return;

		m_bVisible = m_SettingsManager.GetSettingBool(CSI_GameSettings.STAMINA_VISIBLE);
		if (!m_bVisible)
			SetBarOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD.
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		if (!m_wStamBar || !m_bVisible)
			return;

		// Full without a character or stamina component (-1), so the bar fades out
		float stamina = 1;
		SCR_ChimeraCharacter character = m_HUDSystem.GetLocalPlayerCharacter();
		if (character)
		{
			SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (characterController)
				stamina = characterController.GetStamina();
		}

		if (stamina < 0)
			stamina = 1;

		// Full and faded out: nothing to do until stamina drops
		float opacity = m_wStamBar.GetOpacity();
		if (stamina >= 1 && opacity <= 0)
			return;

		OnStaminaChange(stamina);

		float fadeStep = FADE_SPEED * timeSlice;
		if (stamina < 1)
			SetBarOpacity(Math.Min(opacity + fadeStep, 1));
		else
			SetBarOpacity(Math.Max(opacity - fadeStep, 0));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] opacity 0 - 1
	protected void SetBarOpacity(float opacity)
	{
		if (opacity != m_wStamBar.GetOpacity())
			m_wStamBar.SetOpacity(opacity);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Sets the bar's value and color; also called by the Setting Info preview.
	//! \param[in] stamina 0 - 1
	void OnStaminaChange(float stamina)
	{
		if (!m_wStamBar)
			return;

		m_wStamBar.SetCurrent(stamina);

		// Color: green above 70%, amber 30-70%, red below 30%
		Color color;
		switch (true)
		{
			case (stamina < 0.7 && stamina > 0.3) : { color = MEDIUM_STAMINA_COLOR; break;};
			case (stamina < 0.3)                  : { color = LOW_STAMINA_COLOR; break;};
			default                               : { color = HIGH_STAMINA_COLOR; break;};
		};

		// Only set when the band changes
		if (m_StamBarColor == color)
			return;

		m_wStamBar.SetColor(color);
		m_StamBarColor = color;
	}
}
