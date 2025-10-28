class CSI_HUD : SCR_InfoDisplay
{
	protected bool m_bIsVisible = true;
	
	protected override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		GetGame().GetInputManager().AddActionListener("RevealCSIUI", EActionTrigger.DOWN, ToggleIsVisible);
		GetGame().GetInputManager().AddActionListener("RevealCSIUI", EActionTrigger.UP, ToggleIsVisible);
	}

	//------------------------------------------------------------------------------------------------
	protected override event void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
	};
	
	//------------------------------------------------------------------------------------------------
	protected void ToggleIsVisible()
	{
		m_bIsVisible = !m_bIsVisible;
	};
};