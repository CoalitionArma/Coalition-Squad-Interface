//------------------------------------------------------------------------------------------------
//! Nametag UI
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController

modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected CSI_AuthorityComponent m_AuthorityComponent;

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		
		if (!m_AuthorityComponent) {
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			return;
		};

		string nametagsRange = m_AuthorityComponent.ReturnLocalCSISettings()[6];
		string nametagsVisible = m_AuthorityComponent.ReturnLocalCSISettings()[4];

		if (nametagsRange.IsEmpty()) nametagsRange = "35";
		if (nametagsVisible == "false") nametagsRange = "1";

		foreach (SCR_NameTagZone nTZone : GetNametagZones()) {
			nTZone.SetZoneEnd(nametagsRange.ToInt());
		};

		s_NametagCfg.ResetFarthestZone();
	}
}
