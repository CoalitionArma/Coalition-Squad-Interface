//------------------------------------------------------------------------------------------------
// Ruleset for display of all frindlies
[BaseContainerProps()]
modded class SCR_NameTagRulesetFriendlies : SCR_NameTagRulesetBase
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDManager m_HUDManager;
	
	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		if (data.m_Flags & ENameTagFlags.FADE_TIMER)
			data.m_fTimeSliceFade += timeSlice;
		
		if (data.m_Flags & ENameTagFlags.OBSTRUCTED) 	// obstructed, this is checked here in order for the LOS checks to have run for the smaller subset of tag data 
			return false;
		
		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();
		
		bool isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || SCR_BinocularsComponent.IsZoomedView()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();
		
		// increase max distance if player is using a scope or binoculars
		int distMax = m_ZoneCfg.m_fFarthestZoneRangePow2 + ((int)(isZoomed) * (m_ZoneCfg.m_fFarthestZoneRangePow2 * (m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.NAMETAG_MAGNIFICATION_MULTIPLICATION) - 1)));
		
		float distLerp = Math.InverseLerp(distMax, 0, data.m_fDistance); // reduce the angle required to show with distance -> the further is the entity, angle required to focus it gets smaller
		distLerp *= m_iMaxAngle/2;  // adjust for more standard ish FOV of 90 (45 radius) TODO: this should be taken from real FOV
		
		data.m_fAngleToScreenCenter = GetCameraToEntityAngle(data.m_vEntWorldPos, VERT_ANGLE_ADJUST);
		if (data.m_fAngleToScreenCenter < distLerp)		// pass if within visibility angle							
			return true;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	override protected bool TraceLOS(SCR_NameTagData data)
	{
		if (!m_SettingsManager || !m_HUDManager)
		{
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_HUDManager = CSI_HUDManager.GetInstance();
		};
		
		TraceParam param = new TraceParam;
		param.Start = m_CameraPosition;
		param.End = data.m_vEntHeadPos + HEAD_LOS_OFFSET;
		param.Flags = TraceFlags.ANY_CONTACT | TraceFlags.WORLD | TraceFlags.ENTS; 
		IEntity targetEntity = data.m_Entity;
		
		if (data.GetVehicleCompartment())
			targetEntity = data.GetVehicleCompartment().GetVehicle();
		
		array<IEntity> ExcludeArray = {};
		ExcludeArray.Insert(targetEntity);
		ExcludeArray.Insert(m_CurrentPlayerTag.m_Entity);
		param.ExcludeArray = ExcludeArray;
		
		param.LayerMask = TRACE_LAYER_CAMERA;
		float percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)
			return true;
		
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)
			return true;
		
		return false;
	}
};
