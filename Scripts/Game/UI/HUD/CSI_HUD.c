//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-21
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD InfoDisplay: updates the stamina bar, compass, vehicle outline, radar and group list every frame, and has them
//! 				read their settings again on a settings update. Hidden without a controlled character, in the
//! 				pause menu and editor, by Auto-hide HUD until revealed, and in mortars.
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
	protected bool m_bAutoHide;				//!< AUTO_HIDE_HUD

	protected CSI_Stamina m_Stamina;
	protected CSI_Compass m_Compass;
	protected CSI_VehicleOutlineElement m_VehicleOutline;
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

		Widget vehicleOutline = m_wRoot.FindAnyWidget("VehicleOutlineElement");
		if (vehicleOutline)
			m_VehicleOutline = CSI_VehicleOutlineElement.Cast(vehicleOutline.FindHandler(CSI_VehicleOutlineElement));

		Widget radar = m_wRoot.FindAnyWidget("Radar");
		if (radar)
			m_Radar = CSI_Radar.Cast(radar.FindHandler(CSI_Radar));

		if (m_Radar)
			m_Radar.SetVehicleOutline(m_VehicleOutline);

		Widget group = m_wRoot.FindAnyWidget("Group");
		if (group)
			m_Group = CSI_Group.Cast(group.FindHandler(CSI_Group));

		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Insert(OnSettingsUpdate);

		OnSettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayStopDraw(IEntity owner)
	{
		GetGame().GetInputManager().RemoveActionListener("CSI_RevealUI", EActionTrigger.DOWN, OnRevealUIPressed);
		GetGame().GetInputManager().RemoveActionListener("CSI_RevealUI", EActionTrigger.UP, OnRevealUIReleased);

		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Remove(OnSettingsUpdate);

		super.DisplayStopDraw(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the settings used every frame again, here and in the elements. The elements don't follow settings
	//! updates themselves: the settings tab's HUD example embeds them too and sets them up from the tab's values.
	protected void OnSettingsUpdate()
	{
		if (!m_SettingsManager)
			return;

		m_bAutoHide = m_SettingsManager.GetSettingBool(CSI_GameSettings.AUTO_HIDE_HUD);

		if (m_Stamina)
			m_Stamina.ApplySettings();

		if (m_Compass)
			m_Compass.ApplySettings();

		if (m_VehicleOutline)
			m_VehicleOutline.ApplySettings();

		if (m_Radar)
			m_Radar.ApplySettings();
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
		bool show = (m_bIsVisible || !m_bAutoHide) && !m_CSIHUDSystem.GetIsLocalPlayerInMortar();
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

		// Before the radar, which centers on the outline
		if (m_VehicleOutline)
			m_VehicleOutline.Update(timeSlice);

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