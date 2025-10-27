class CSI_HUD : SCR_InfoDisplay
{
	//------------------------------------------------------------------------------------------------
	protected override event void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
	
		string stamBarVisible = m_ClientComponent.ReturnLocalCSISettings()[3];

		if (stamBarVisible == "false") 
		{
			if (m_wStamBar.GetOpacity() > 0) m_wStamBar.SetOpacity(0);
			return;
		}
			
	};
};