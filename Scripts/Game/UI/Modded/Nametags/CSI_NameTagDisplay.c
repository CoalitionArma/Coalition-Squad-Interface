//------------------------------------------------------------------------------------------------
//! Nametag UI
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController

modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected CSI_ClientComponent m_ClientComponent;

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		
		if (Replication.IsServer()) return;
		
		if (!m_ClientComponent) {
			m_ClientComponent = CSI_ClientComponent.GetInstance();
			return;
		};

		string nametagsRange = m_ClientComponent.ReturnLocalCSISettings()[6];
		string nametagsVisible = m_ClientComponent.ReturnLocalCSISettings()[4];

		if (nametagsRange.IsEmpty()) nametagsRange = "35";
		if (nametagsVisible == "false") nametagsRange = "1";

		foreach (SCR_NameTagZone nTZone : GetNametagZones()) {
			nTZone.SetZoneEnd(nametagsRange.ToInt());
		};

		s_NametagCfg.ResetFarthestZone();
	}
}
