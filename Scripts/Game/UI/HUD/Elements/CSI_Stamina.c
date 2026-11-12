//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD stamina bar: fades in while stamina is below full and out when full; the color shows the stamina band.
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
	//! Called every frame by CSI_HUD.
	void Update()
	{
		if (!m_SettingsManager.GetSettingBool(CSI_GameSettings.STAMINA_VISIBLE))
		{
			HideBar(0);
			return;
		}

		SCR_ChimeraCharacter character = m_HUDSystem.GetLocalPlayerCharacter();

		if (!character)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));

		if (!characterController)
			return;

		OnStaminaChange(characterController.GetStamina());
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Raises the bar's opacity one step (0.005 per frame).
	//! \param[in] currentOpacity
	void RevealBar(float currentOpacity)
	{
		float setOpacity = Math.Clamp(currentOpacity + 0.005, 0.0, 1.0);
		m_wStamBar.SetOpacity(setOpacity);
	}

	//------------------------------------------------------------------------------------------------
	//! Lowers the bar's opacity one step (0.005 per frame).
	//! \param[in] currentOpacity
	protected void HideBar(float currentOpacity)
	{
		float setOpacity = Math.Clamp(currentOpacity - 0.005, 0.0, 1.0);
		m_wStamBar.SetOpacity(setOpacity);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the bar's value, color and fade direction.
	//! \param[in] stamina 0 - 1
	void OnStaminaChange(float stamina)
	{
		m_wStamBar.SetCurrent(stamina);

		// Color: green above 70%, amber 30-70%, red below 30%
		switch (true)
		{
			case (stamina < 0.7 && stamina > 0.3) : { m_wStamBar.SetColor(new Color(0.091997, 0.083009, 0.035996, 1.000000)); break;};
			case (stamina < 0.3)                  : { m_wStamBar.SetColor(new Color(0.188724, 0.046860, 0.037476, 1.000000)); break;};
			default                               : { m_wStamBar.SetColor(new Color(0.035996, 0.091997, 0.050004, 1.000000)); break;};
		};

		float currentOpacity = m_wStamBar.GetOpacity();

		// Opacity: fade in while below full, fade out once full
		switch (true)
		{
			case (stamina < 1 && currentOpacity < 1)  : { RevealBar(currentOpacity); break;};
			case (stamina == 1 && currentOpacity > 0) : { HideBar(currentOpacity); break;};
		};
	}
}
