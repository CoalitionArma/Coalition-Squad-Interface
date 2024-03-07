//------------------------------------------------------------------------------------------------
//! Nametag UI
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController

modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected CSI_GameModeComponent m_GameModeComponent;

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		
		if (!m_GameModeComponent) {
			m_GameModeComponent = CSI_GameModeComponent.GetInstance();
			return;
		};

		string nametagsRange = m_GameModeComponent.ReturnLocalCSISettings()[6];
		string nametagsVisible = m_GameModeComponent.ReturnLocalCSISettings()[4];

		if (nametagsRange.IsEmpty()) nametagsRange = "35";
		if (nametagsVisible == "false") nametagsRange = "1";

		foreach (SCR_NameTagZone nTZone : GetNametagZones()) {
			nTZone.SetZoneEnd(nametagsRange.ToInt());
		};

		s_NametagCfg.ResetFarthestZone();
	}
}
