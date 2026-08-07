//------------------------------------------------------------------------------------------------
//! Nametag UI
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController
modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected const float FACTION_RECHECK_TIME = 1.0; // seconds between checks of the local characters faction

	protected CSI_SettingsManager m_SettingsManager;
	protected SCR_NameTagZone m_CachedNearZone;
	protected FactionAffiliationComponent m_LocalFactionAffiliation;
	protected IEntity m_LocalFactionEntity;
	protected float m_fFactionRecheckTime;
	protected int m_iLastNametagsRange = -1;

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);

		UpdateCurrentFaction(timeSlice);

		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		bool isZoomed;
		if (m_CurrentPlayerTag && m_CurrentPlayerTag.m_CharController && SCR_PlayerController.s_pLocalPlayerController)
			isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || SCR_PlayerController.s_pLocalPlayerController.GetIsBinocularsZoomed()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();

		int baseRange = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_RANGE);
		int nametagsRange;
		if (isZoomed)
			nametagsRange = baseRange * m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION) / 100; // increase max distance if player is using a scope or binoculars
		else
			nametagsRange = baseRange;

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

			s_NametagCfg.ResetFarthestZone();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keep m_CurrentFaction in sync with the faction the local character actually has.
	protected void UpdateCurrentFaction(float timeSlice)
	{
		m_fFactionRecheckTime += timeSlice;
		if (m_fFactionRecheckTime < FACTION_RECHECK_TIME)
			return;

		m_fFactionRecheckTime = 0;

		IEntity localEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!localEntity)
			return;

		if (localEntity != m_LocalFactionEntity || !m_LocalFactionAffiliation)
		{
			m_LocalFactionEntity = localEntity;
			m_LocalFactionAffiliation = FactionAffiliationComponent.Cast(localEntity.FindComponent(FactionAffiliationComponent));
		}

		if (!m_LocalFactionAffiliation)
			return;

		Faction faction = m_LocalFactionAffiliation.GetAffiliatedFaction();
		if (!faction || faction == m_CurrentFaction)
			return;

		// The faction arrived late or changed, so every tag has to be re-filtered against it
		m_CurrentFaction = faction;
		RefreshTags(localEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Drop m_CurrentPlayerTag when the tag it points at is wiped.
	override void CleanupAllTags()
	{
		bool currentTagWiped = m_CurrentPlayerTag && m_aNameTags.Find(m_CurrentPlayerTag) != -1;

		super.CleanupAllTags();

		if (currentTagWiped)
			m_CurrentPlayerTag = null;
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