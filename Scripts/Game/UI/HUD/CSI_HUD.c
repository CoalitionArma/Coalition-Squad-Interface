class CSI_HUD : SCR_InfoDisplay
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDManager m_CSIHUDManager;
	protected bool m_bIsVisible = false;
	
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
		m_CSIHUDManager = CSI_HUDManager.GetInstance();
		
		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.DOWN, ToggleIsVisible);
		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.UP, ToggleIsVisible);
		
		Widget stamina = m_wRoot.FindAnyWidget("Stamina");
		Widget compass = m_wRoot.FindAnyWidget("Compass");
		Widget radar = m_wRoot.FindAnyWidget("Radar");
		Widget group = m_wRoot.FindAnyWidget("Group");
		
		m_Stamina = CSI_Stamina.Cast(stamina.FindHandler(CSI_Stamina));
		m_Compass = CSI_Compass.Cast(compass.FindHandler(CSI_Compass));
		m_Radar = CSI_Radar.Cast(radar.FindHandler(CSI_Radar));
		m_Group = CSI_Group.Cast(group.FindHandler(CSI_Group));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected override event void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		if ((!m_bIsVisible && m_SettingsManager.GetSettingBool(CSI_GameSettings.AUTO_HIDE_HUD)) || m_CSIHUDManager.GetIsLocalPlayerInMortar())
		{
			m_wRoot.SetVisible(false);
			return;
		} else
			m_wRoot.SetVisible(true);
		
		m_Stamina.Update();
		m_Compass.Update();
		m_Group.Update();
		m_Radar.Update();
	};
	
	//------------------------------------------------------------------------------------------------
	protected void ToggleIsVisible()
	{
		m_bIsVisible = !m_bIsVisible;
	};
};