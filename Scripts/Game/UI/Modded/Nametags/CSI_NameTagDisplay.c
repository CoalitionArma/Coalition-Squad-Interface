//------------------------------------------------------------------------------------------------
//! Nametag UI 
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController

modded class SCR_NameTagDisplay
{												
	protected CSI_AuthorityComponent m_AuthorityComponent;		
	protected string m_sNametagsVisible;	
	protected int m_iNametagsRange;	
	
	protected int m_iCheckSettingsFrame = 65;

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{				
		super.DisplayUpdate(owner, timeSlice);
	
		if (!m_AuthorityComponent) {
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			return;
		};
		
		if (m_iCheckSettingsFrame >= 65) 
		{
			m_iCheckSettingsFrame = 0;
			string nametagsVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[4];
			switch (true)
			{
				case(nametagsVisibleSO == "true" || nametagsVisibleSO == "false") : { m_sNametagsVisible = nametagsVisibleSO; break; };
				default : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsVisible", m_sNametagsVisible); break; };
			};
			
			string nametagsRangeSOString = m_AuthorityComponent.ReturnAuthoritySettings()[6];
			int nametagsRangeSO = 0;
			if (nametagsRangeSOString != "") {
				nametagsRangeSO = nametagsRangeSOString.ToInt();
			} else {
				nametagsRangeSO = 0;
			};
			switch (true)
			{
				case(nametagsRangeSO > 0) : { m_iNametagsRange = nametagsRangeSO; break; };
				default : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsRange", m_iNametagsRange); break; };
			};
		
			if(m_iNametagsRange == 0) m_iNametagsRange = 35;
			if (m_sNametagsVisible == "false") m_iNametagsRange = 1;

			foreach(SCR_NameTagZone nTZone : GetNametagZones()) {
				nTZone.SetZoneEnd(m_iNametagsRange);
			};
			
			s_NametagCfg.ResetFarthestZone();
		} else { m_iCheckSettingsFrame++; };
	}
};