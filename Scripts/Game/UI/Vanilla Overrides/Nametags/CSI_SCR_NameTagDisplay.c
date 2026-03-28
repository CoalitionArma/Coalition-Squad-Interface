//------------------------------------------------------------------------------------------------
//! Nametag UI
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController
modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected CSI_SettingsManager m_SettingsManager;

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		
		if (!m_SettingsManager) 
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		bool isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || SCR_BinocularsComponent.IsZoomedView()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();
		int baseRange = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_RANGE);
		int nametagsRange;
		if (isZoomed)
			nametagsRange = baseRange * m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION) / 100; // increase max distance if player is using a scope or binoculars
		else
			nametagsRange = baseRange;

		if (!m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_VISIBLE))
			nametagsRange = 1;

		foreach (SCR_NameTagZone nTZone : GetNametagZones()) 
			if (nTZone.GetZoneName() == "Near")
				nTZone.SetZoneEnd(nametagsRange);

		s_NametagCfg.ResetFarthestZone();
	}
}

//------------------------------------------------------------------------------------------------
//! Nametag zone config
[BaseContainerProps(), SCR_NameTagZoneTitle()]
modded class SCR_NameTagZone
{
	void SetZoneEnd(int range)
	{
		m_iZoneEnd = range;
		m_iZoneEndSq = range * range;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
modded class SCR_NameTagConfig
{
	void ResetFarthestZone()
	{
		m_iZoneCount = m_aZones.Count();

		int farthestZone;

		// Max distance
		for (int i = 0; i < m_iZoneCount; i++)
		{
			int zoneEnd = m_aZones[i].m_iZoneEnd;

			if (zoneEnd > farthestZone)
			{
				farthestZone = zoneEnd;
				m_fFarthestZoneRange = zoneEnd;
				m_fFarthestZoneRangeOriginal = zoneEnd;
				m_fFarthestZoneRangePow2 = zoneEnd * zoneEnd;
			}
		}
	}
}