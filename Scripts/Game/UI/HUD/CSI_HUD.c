//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD InfoDisplay: updates the stamina bar, compass, radar and group list every frame. Hidden without
//! 				a controlled character, in the pause menu and editor, by Auto-hide HUD until revealed, and in mortars.
//! 
//------------------------------------------------------------------------------------------------
class CSI_HUD : SCR_InfoDisplayExtended
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_CSIHUDSystem;
	protected bool m_bIsVisible = false;	//!< revealed with CSI_RevealUI; only used with AUTO_HIDE_HUD

	protected CSI_Stamina m_Stamina;
	protected CSI_Compass m_Compass;
	protected CSI_Radar m_Radar;
	protected CSI_Group m_Group;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 HUD INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_CSIHUDSystem = CSI_HUDSystem.GetInstance();

		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.DOWN, OnRevealUIPressed);
		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.UP, OnRevealUIReleased);

		if (!m_wRoot)
			return;

		// Each element is optional
		Widget stamina = m_wRoot.FindAnyWidget("Stamina");
		if (stamina)
			m_Stamina = CSI_Stamina.Cast(stamina.FindHandler(CSI_Stamina));

		Widget compass = m_wRoot.FindAnyWidget("Compass");
		if (compass)
			m_Compass = CSI_Compass.Cast(compass.FindHandler(CSI_Compass));

		Widget radar = m_wRoot.FindAnyWidget("Radar");
		if (radar)
			m_Radar = CSI_Radar.Cast(radar.FindHandler(CSI_Radar));

		Widget group = m_wRoot.FindAnyWidget("Group");
		if (group)
			m_Group = CSI_Group.Cast(group.FindHandler(CSI_Group));
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayStopDraw(IEntity owner)
	{
		GetGame().GetInputManager().RemoveActionListener("CSI_RevealUI", EActionTrigger.DOWN, OnRevealUIPressed);
		GetGame().GetInputManager().RemoveActionListener("CSI_RevealUI", EActionTrigger.UP, OnRevealUIReleased);

		super.DisplayStopDraw(owner);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Not called without a controlled character or while the pause menu or editor is open.
	protected override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);

		if (!m_wRoot || !m_SettingsManager || !m_CSIHUDSystem)
			return;

		// Hidden by Auto-hide HUD until revealed, and in mortars
		bool show = (m_bIsVisible || !m_SettingsManager.GetSettingBool(CSI_GameSettings.AUTO_HIDE_HUD)) && !m_CSIHUDSystem.GetIsLocalPlayerInMortar();
		if (m_bShown != show)
			Show(show);

		if (!show)
			return;

		if (m_Stamina)
			m_Stamina.Update(timeSlice);

		if (m_Compass)
			m_Compass.Update();

		if (m_Group)
			m_Group.Update();

		if (m_Radar)
			m_Radar.Update(timeSlice);
	};

	//------------------------------------------------------------------------------------------------
	//! Shows the HUD (HOLD) or toggles it (TOGGLE).
	protected void OnRevealUIPressed()
	{
		if (IsRevealToggled())
			m_bIsVisible = !m_bIsVisible;
		else
			m_bIsVisible = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Hides the HUD again (HOLD only).
	protected void OnRevealUIReleased()
	{
		if (!IsRevealToggled())
			m_bIsVisible = false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsRevealToggled()
	{
		return m_SettingsManager && m_SettingsManager.GetSettingInt(CSI_GameSettings.REVEAL_UI_MODE) == CSI_ERevealUIMode.TOGGLE;
	}
};