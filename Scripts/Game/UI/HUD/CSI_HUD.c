//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-13
//! \Source: 		Coalition Squad Interface
//!	\Description: 	CSI's HUD InfoDisplay: finds the stamina bar, compass, radar and group list in its layout and updates them every frame.
//! 				Public API: CRF mods UpdateValues and uses m_wRoot.
//! 
//------------------------------------------------------------------------------------------------
class CSI_HUD : SCR_InfoDisplay
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_CSIHUDSystem;
	protected bool m_bIsVisible = false;	//!< revealed with CSI_RevealUI (held or toggled, REVEAL_UI_MODE), only used with AUTO_HIDE_HUD

	protected CSI_Stamina m_Stamina;
	protected CSI_Compass m_Compass;
	protected CSI_Radar m_Radar;
	protected CSI_Group m_Group;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 HUD INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_CSIHUDSystem = CSI_HUDSystem.GetInstance();

		// Press and release, so HOLD shows the HUD while the key is held and TOGGLE flips it on each press
		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.DOWN, OnRevealUIPressed);
		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.UP, OnRevealUIReleased);

		if (!m_wRoot)
			return;

		// Each element is optional, so an edited layout without one doesn't break the others
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
	//! Removes the CSI_RevealUI listeners added in OnStartDraw, so a restarted display doesn't stack another pair.
	//! \param[in] owner
	protected override event void OnStopDraw(IEntity owner)
	{
		GetGame().GetInputManager().RemoveActionListener("CSI_RevealUI", EActionTrigger.DOWN, OnRevealUIPressed);
		GetGame().GetInputManager().RemoveActionListener("CSI_RevealUI", EActionTrigger.UP, OnRevealUIReleased);

		super.OnStopDraw(owner);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected override event void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);

		if (!m_wRoot || !m_SettingsManager || !m_CSIHUDSystem)
			return;

		// Hidden while auto-hide is on and the HUD isn't revealed (CSI_RevealUI), and while using a mortar
		if ((!m_bIsVisible && m_SettingsManager.GetSettingBool(CSI_GameSettings.AUTO_HIDE_HUD)) || m_CSIHUDSystem.GetIsLocalPlayerInMortar())
		{
			m_wRoot.SetVisible(false);
			return;
		}

		m_wRoot.SetVisible(true);

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
	//! CSI_RevealUI pressed: shows the HUD (HOLD), or shows or hides it (TOGGLE).
	protected void OnRevealUIPressed()
	{
		if (IsRevealToggled())
			m_bIsVisible = !m_bIsVisible;
		else
			m_bIsVisible = true;
	}

	//------------------------------------------------------------------------------------------------
	//! CSI_RevealUI released: hides the HUD again (HOLD only).
	protected void OnRevealUIReleased()
	{
		if (!IsRevealToggled())
			m_bIsVisible = false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if REVEAL_UI_MODE is TOGGLE
	protected bool IsRevealToggled()
	{
		return m_SettingsManager && m_SettingsManager.GetSettingInt(CSI_GameSettings.REVEAL_UI_MODE) == CSI_ERevealUIMode.TOGGLE;
	}
};