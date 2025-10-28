class CSI_HUD : SCR_InfoDisplay
{
	protected bool m_bIsVisible = true;
	
	protected Widget m_wStamina;
	protected Widget m_wCompass;
	protected Widget m_wGroup;
	
	protected override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		GetGame().GetInputManager().AddActionListener("RevealCSIUI", EActionTrigger.DOWN, ToggleIsVisible);
		GetGame().GetInputManager().AddActionListener("RevealCSIUI", EActionTrigger.UP, ToggleIsVisible);
		
		m_wStamina = m_wRoot.FindAnyWidget("Stamina");
		m_wCompass = m_wRoot.FindAnyWidget("Compass");
		m_wGroup = m_wRoot.FindAnyWidget("Group");
	}

	//------------------------------------------------------------------------------------------------
	protected override event void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		if (!m_bIsVisible)
		{
			m_wRoot.SetVisible(false);
			return;		
		} else
			m_wRoot.SetVisible(true);
		
		CSI_Stamina.Cast(m_wStamina.FindHandler(CSI_Stamina)).Update();
		CSI_Compass.Cast(m_wCompass.FindHandler(CSI_Compass)).Update();
		CSI_Group.Cast(m_wGroup.FindHandler(CSI_Group)).Update();
	};
	
	//------------------------------------------------------------------------------------------------
	protected void ToggleIsVisible()
	{
		m_bIsVisible = !m_bIsVisible;
	};
};