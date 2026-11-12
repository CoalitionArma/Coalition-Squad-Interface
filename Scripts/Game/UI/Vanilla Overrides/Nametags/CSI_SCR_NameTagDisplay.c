//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the "Near" zone's range comes from NAMETAG_RANGE (multiplied while zoomed through sights or
//! 				binoculars), and nametags are hidden when NAMETAG_VISIBLE is off.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected CSI_SettingsManager m_SettingsManager;
	protected SCR_NameTagZone m_CachedNearZone;
	protected int m_iLastNametagsRange = -1;	//!< last range applied, so zones are only reset on change
	protected bool m_b2DSightsActive = false;

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);

		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		// Can be missing while dead/respawning or before the game mode streams in
		if (!m_SettingsManager || !m_CurrentPlayerTag || !m_CurrentPlayerTag.m_CharController || !SCR_PlayerController.s_pLocalPlayerController)
			return;

		bool isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || m_b2DSightsActive || SCR_PlayerController.s_pLocalPlayerController.GetIsBinocularsZoomed()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();
		int baseRange = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_RANGE);
		int nametagsRange;
		if (isZoomed)
			nametagsRange = baseRange * m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION) / 100; // increase max distance if player is using a scope or binoculars
		else
			nametagsRange = baseRange;

		// Disabled: shrink the zone to 1 m
		if (!m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_VISIBLE))
			nametagsRange = 1;

		if (nametagsRange != m_iLastNametagsRange)
		{
			m_iLastNametagsRange = nametagsRange;

			if (!m_CachedNearZone)
				foreach (SCR_NameTagZone nTZone : GetNametagZones())
					if (nTZone.GetZoneName() == "Near")
						m_CachedNearZone = nTZone;

			if (m_CachedNearZone)
				m_CachedNearZone.SetZoneEnd(nametagsRange);

			// The config caches the farthest range, recompute it
			s_NametagCfg.ResetFarthestZone();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adjust zone range based on zoom. CSI only records whether 2D sights are zoomed; DisplayUpdate applies the range.
	//! \param[in] adjust determines if we are adjusting the ranges or restoring them to default
	//! \param[in] FOVZoomed is the new FOV we are adjusting to
	protected override void AdjustRange(bool adjust, float FOVZoomed)
	{
		if (adjust)
			m_b2DSightsActive = true;
		else
			m_b2DSightsActive = false;
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: adds SetZoneEnd so a zone's range can change at runtime.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagZoneTitle()]
modded class SCR_NameTagZone
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] range zone end in metres
	void SetZoneEnd(int range)
	{
		m_iZoneEnd = range;
		m_iZoneEndSq = range * range;
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: adds ResetFarthestZone to recompute the farthest zone range after a zone is resized.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
modded class SCR_NameTagConfig
{
	//------------------------------------------------------------------------------------------------
	//! Recomputes m_fFarthestZoneRange (and its original and squared values) from the current zones.
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