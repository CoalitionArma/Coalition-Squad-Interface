//------------------------------------------------------------------------------------------------
//! Nametag zone config root
[BaseContainerProps(), SCR_NameTagZoneTitle()]
modded class SCR_NameTagZone
{		
	void SetZoneEnd(int range)
	{
		m_iZoneEnd = range;
		m_iZoneEndPow2 = range * range;
	}
};


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
};
