//------------------------------------------------------------------------------------------------
// Ruleset for dosplay of all frindlies
[BaseContainerProps()]
class SCR_NameTagRulesetFriendlyPlayers : SCR_NameTagRulesetBase
{
	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		if (data.m_Flags & ENameTagFlags.FADE_TIMER)
			data.m_fTimeSliceFade += timeSlice;
		
		if (data.m_Flags & ENameTagFlags.OBSTRUCTED) 	// obstructed, this is checked here in order for the LOS checks to have run for the smaller subset of tag data 
			return false;
		
		float distLerp = Math.InverseLerp(m_ZoneCfg.m_fFarthestZoneRangePow2, 0, data.m_fDistance); // reduce the angle required to show with distance -> the further is the entity, angle required to focus it gets smaller
		distLerp *= m_iMaxAngle/2;  // adjust for more standard ish FOV of 90 (45 radius) TODO: this should be taken from real FOV
		
		data.m_fAngleToScreenCenter = GetCameraToEntityAngle(data.m_vEntWorldPos, VERT_ANGLE_ADJUST);
		if (data.m_fAngleToScreenCenter < distLerp)		// pass if within visibility angle							
			return true;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void DetermineVisibility(float timeSlice)
	{
		super.DetermineVisibility(timeSlice);

		int count = m_aCandidateTags.Count();
		for (int i = count - 1; i > -1; i--)
		{		
			SCR_NameTagData data = m_aCandidateTags.Get(i);

			UpdateVisibleTag(data, timeSlice);
		}
	}
	
		//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	override protected bool TraceLOS(SCR_NameTagData data)
	{
		if (data.m_Flags & ENameTagFlags.VEHICLE || data.m_Flags & ENameTagFlags.TURRET)
			return true;
		
		TraceParam param = new TraceParam;
		param.Start = m_CurrentPlayerTag.m_vEntHeadPos;
		param.End = data.m_vEntHeadPos + HEAD_LOS_OFFSET;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ANY_CONTACT | TraceFlags.WORLD | TraceFlags.ENTS; 
		array<IEntity> excludes = { m_CurrentPlayerTag.m_Entity, data.m_Entity };
		param.ExcludeArray = excludes;
			
		float percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)	// If trace travels the entire path, return true
			return true;
				
		return false;
	}
};
