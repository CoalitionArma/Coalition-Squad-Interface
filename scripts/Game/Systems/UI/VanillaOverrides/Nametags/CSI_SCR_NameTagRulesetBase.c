[BaseContainerProps()]
modded class SCR_NameTagRulesetBase : Managed
{
	protected CSI_SettingsManager m_SettingsManager;
	
	//------------------------------------------------------------------------------------------------
	//! Determine whether the tag passes basic conditions for visibility
	//! \param data is the subject nametag
	//! \param timeSlice is frame time slice
	//! \return Returns true if tag passed conditions for visibility
	override protected bool TestVisibility(SCR_NameTagData data, float timeSlice)
	{
		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();
		
		// current players tag, update tag specific vars and return
		if (data == m_CurrentPlayerTag)
		{
			data.UpdateTagPos();
			return false;
		}

		// out of zone range
		data.m_fTimeSliceUpdate += timeSlice;
		
		if (data.m_fTimeSliceUpdate >= POSITION_UPDATE) // avoid updating every frame
		{
			data.m_fTimeSliceUpdate = 0;
			
			data.m_fDistance = vector.DistanceSq(m_CurrentPlayerTag.m_vEntWorldPos, data.m_Entity.GetOrigin()); // checked through origin here to avoid more expensive pos check bottom
		}
		
		//int notScopedMaxRangeSqrd = m_ZoneCfg.m_fFarthestZoneRangePow2 - 2500; // don't show tags farther than 50m (50^2) unless scoping
		bool isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || SCR_BinocularsComponent.IsZoomedView()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();
		
		// increase max distance if player is using a scope or binoculars
		int distMax = m_ZoneCfg.m_fFarthestZoneRangePow2 + ((int)(isZoomed) * (m_ZoneCfg.m_fFarthestZoneRangePow2 * (m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.NAMETAG_MAGNIFICATION_MULTIPLICATION) - 1)));
		
		if (data.m_fDistance >= distMax) // distance of visible tag is updated per frame for scaling, which is why this check has its own scope
		{
			if (data.m_fDistance >= distMax + 100)		// cleanup tags which are more than distance + 10m (10^2) away from base zone query 
				data.m_fTimeSliceCleanup += timeSlice;			
			
			return false;
		}
		else
			data.m_fTimeSliceCleanup = 0;
		
		// vehicle visibility 
		if (data.m_Flags & ENameTagFlags.VEHICLE)	// in vehicle
		{
			if (!data.m_VehicleParent.m_bIsControlledPresent)	// dont show vehicle crew tags until inside the same vehicle
				return false;
			else if (m_CurrentPlayerTag.m_CharController.IsInThirdPersonView())	// dont show vehicle crew tags if in third person
				return false;
		}
		
		if (data.m_eType == ENameTagEntityType.VEHICLE)	// is vehicle
		{	
			if (data.m_Flags & ENameTagFlags.VEHICLE_DISABLE || SCR_VehicleTagData.Cast(data).m_bIsControlledPresent)
				return false;
		}
								
		// update tag position
		if (data.m_eAttachedTo == data.m_eAttachedToLast)
			data.UpdateTagPos();
		else
		{
			data.m_fTimeSlicePosChange += timeSlice * POS_TRANSITION_MULT;
			if ( data.m_fTimeSlicePosChange >= 1.0 )
			{
				data.UpdateTagPos();
				data.m_eAttachedToLast = data.m_eAttachedTo;
			}
			else 
			{
				data.UpdateTagPos();
				data.m_vTagWorldPos = vector.Lerp( data.m_vTagWorldPosLast, data.m_vTagWorldPos, data.m_fTimeSlicePosChange );
			}
		}
					
		CalculateScreenPos(data);
		if (m_iRefResolutionY * 1.1 < data.m_vTagScreenPos[1] || m_iRefResolutionX * 1.1 < data.m_vTagScreenPos[0] || 0 > data.m_vTagScreenPos[1] || 0 > data.m_vTagScreenPos[0])
			return false;	
			
		return true;	
	} 

	//------------------------------------------------------------------------------------------------
	//! Update for visible tags
	//! \param data is the subject nametag
	//! \param timeSlice is the OnFrame timeslice
	override protected void UpdateVisibleTag(SCR_NameTagData data, float timeSlice)
	{	
		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();
					
		if (data.m_eEntityStateFlags & ENameTagEntityState.HIDDEN)	// fade in tag			
			data.DeactivateEntityState(ENameTagEntityState.HIDDEN);

		bool isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || SCR_BinocularsComponent.IsZoomedView()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();
											
		// Zone selection
		int currentZone = -1;
		for (int i = 0; i < m_ZoneCfg.m_iZoneCount; i++) 	// get lowest nametag zone and use it to display the tag, for perf improvement, only nearest zone is searched for when deciding where to draw
		{
			
			// increase max zone distance if player is using a scope or binoculars
			int distMaxZone = m_ZoneCfg.m_aZones[i].m_iZoneEndPow2 + ((int)(isZoomed) * (m_ZoneCfg.m_aZones[i].m_iZoneEndPow2 * (m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.NAMETAG_MAGNIFICATION_MULTIPLICATION) - 1)));
			
			if ( data.m_fDistance <=  distMaxZone)
			{
				currentZone = i;
				break; 		// zones are ordered from lowest, if we find a match leave
			}
		}
		
		if (data.m_iZoneID != currentZone)
		{	
			if (data.m_iZoneID != -1)
				m_ZoneCfg.m_aZones[data.m_iZoneID].HideElements(data);		
				
			m_ZoneCfg.m_aZones[currentZone].OnZoneChanged(data); // mark for update if we moved between zones	
		}
																					
		data.m_iZoneID = currentZone;
		m_ZoneCfg.m_aZones[data.m_iZoneID].DrawElements(data);
									
		// Set opacity and ZOrder
		if ( ~data.m_eEntityStateFlags & ENameTagEntityState.FOCUSED )
		{
			int distMax = m_ZoneCfg.m_fFarthestZoneRangePow2 + ((int)(isZoomed) * m_ZoneCfg.m_fFarthestZoneRangePow2); // increase max distance if player is using a scope or binoculars
			
			data.m_fOpacityFade = Math.InverseLerp(distMax, 0, data.m_fDistance);	// fade opacity with distance
			data.m_NameTagWidget.SetOpacity(data.m_fVisibleOpacity * data.m_fOpacityFade);
			
			int zOrder = Math.Round((data.m_fOpacityFade - 1.0) * 100);	// reuse lerp above, the further away target, the higher -zOrder applied //  1.0 is max opacity // 100 is decimal correction 
			data.m_NameTagWidget.SetZOrder(zOrder);
		}
		
		// Set tag 2D pos
		FrameSlot.SetPos(data.m_NameTagWidget, data.m_vTagScreenPos[0], data.m_vTagScreenPos[1]);
						
		// VON indication
		if (data.m_eEntityStateFlags & ENameTagEntityState.VON)
		{
			if (data.m_fTimeSliceVON >= VON_STOP_TIME)
				data.DeactivateEntityState(ENameTagEntityState.VON);
			else
				data.m_fTimeSliceVON += timeSlice;
		}
		
		// Tag positioning for vehicle
		if ( data.m_Flags & ENameTagFlags.VEHICLE )
		{
			if (data.m_eAttachedTo != ENameTagPosition.BODY)
				data.SetTagPosition(ENameTagPosition.BODY, false);
		}
		else if (data.m_eAttachedTo == ENameTagPosition.BODY)
			data.SetTagPosition(ENameTagPosition.HEAD, false);
	}
}