class CSI_HUD : SCR_InfoDisplay
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDManager m_CSIHUDManager;
	protected bool m_bIsVisible = false;
	
	protected Widget m_wStamina;
	protected Widget m_wCompass;
	protected Widget m_wRadar;
	protected Widget m_wGroup;
	
	protected CSI_Stamina m_Stamina;
	protected CSI_Compass m_Compass;
	protected CSI_Radar m_Radar;
	protected CSI_Group m_Group;
	
	protected override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_CSIHUDManager = CSI_HUDManager.GetInstance();
		
		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.DOWN, ToggleIsVisible);
		GetGame().GetInputManager().AddActionListener("CSI_RevealUI", EActionTrigger.UP, ToggleIsVisible);
		
		m_wStamina = m_wRoot.FindAnyWidget("Stamina");
		m_wCompass = m_wRoot.FindAnyWidget("Compass");
		m_wRadar = m_wRoot.FindAnyWidget("Radar");
		m_wGroup = m_wRoot.FindAnyWidget("Group");
		
		m_Stamina = CSI_Stamina.Cast(m_wStamina.FindHandler(CSI_Stamina));
		m_Compass = CSI_Compass.Cast(m_wCompass.FindHandler(CSI_Compass));
		m_Radar = CSI_Radar.Cast(m_wRadar.FindHandler(CSI_Radar));
		m_Group = CSI_Group.Cast(m_wGroup.FindHandler(CSI_Group));
	}

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