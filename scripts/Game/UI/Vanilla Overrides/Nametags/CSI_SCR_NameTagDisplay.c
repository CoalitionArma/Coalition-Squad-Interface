//------------------------------------------------------------------------------------------------
//! Nametag UI
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController
modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected CSI_SettingsManager m_SettingsManager;

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		
		if (!m_SettingsManager) 
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		int nametagsRange = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_RANGE);

		if (!m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_VISIBLE)) 
			nametagsRange = 1;

		foreach (SCR_NameTagZone nTZone : GetNametagZones()) 
			nTZone.SetZoneEnd(nametagsRange);

		s_NametagCfg.ResetFarthestZone();
	}
}
