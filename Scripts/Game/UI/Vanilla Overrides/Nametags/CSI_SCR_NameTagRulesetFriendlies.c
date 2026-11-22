//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-19
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: tag field of view from NAMETAG_FIELD_OF_VIEW, with each drawn tag's distance from the
//! 				center of the view recorded as a share of it; the line of sight check can be turned off and
//! 				traces to the body when tags are on the body; the talking state also times out on fading and
//! 				hidden tags.
//!
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
modded class SCR_NameTagRulesetFriendlies : SCR_NameTagRulesetBase
{
	const vector BODY_LOS_OFFSET = "0 -0.18 0"; //!< tag visual position offset for body

	//------------------------------------------------------------------------------------------------
	override protected void DetermineVisibility(float timeSlice)
	{
		// CSI: m_iMaxAngle from the field of view setting; 0 = not loaded yet
		if (CSI_NametagSettings.s_iFieldOfView > 0)
			m_iMaxAngle = CSI_NametagSettings.s_iFieldOfView;

		// CSI: this frame's camera, for TestVisibilityFiltered (super reads it again for its line of sight checks)
		vector cameraMat[4];
		m_World.GetCurrentCamera(cameraMat);
		m_CameraPosition = cameraMat[3];
		m_vCameraForward = cameraMat[2];

		super.DetermineVisibility(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Also ends the talking state on fading and hidden tags. Vanilla only counts VON_STOP_TIME down in
	//! UpdateVisibleTag, so a tag that lost line of sight while its player talked stayed talking until it showed again.
	override protected bool DisableTag(SCR_NameTagData data, float timeSlice)
	{
		UpdateVONTimeout(data, timeSlice);

		return super.DisableTag(data, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Ends the tag's talking state VON_STOP_TIME after the last voice received. Once a frame per tag.
	//! \param[in] data
	//! \param[in] timeSlice
	protected void UpdateVONTimeout(SCR_NameTagData data, float timeSlice)
	{
		if (!(data.m_eEntityStateFlags & ENameTagEntityState.VON))
			return;

		if (data.m_fTimeSliceVON >= VON_STOP_TIME)
			data.DeactivateEntityState(ENameTagEntityState.VON);
		else
			data.m_fTimeSliceVON += timeSlice;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		if (!super.TestVisibilityFiltered(data, timeSlice))
		{
			// Super then calls DisableTag only for a shown or fading tag; a tag already hidden (behind cover, or
			// outside the field of view) has its talking state timed out here
			if (!(data.m_Flags & ENameTagFlags.VISIBLE) && !(data.m_Flags & ENameTagFlags.UPDATE_DISABLE))
				UpdateVONTimeout(data, timeSlice);

			return false;
		}

		// Only a player's tag can turn icon only by its place (SCR_NameTagData.UpdateCSIState)
		if (CSI_NametagSettings.s_fIconOnlyFieldOfView == 0 || !CSI_NametagSettings.s_bRoleIcons || data.m_eType != ENameTagEntityType.PLAYER)
			return true;

		// CSI: how far the drawn tag is from the center of the view, as a share of its visible angle. Super's angle
		// (m_fAngleToScreenCenter) is to the neck and counts height 0.3 times, so the names stayed in a tall zone
		// that sat below the center of the screen for a tag on the torso
		float visibleAngle = Math.InverseLerp(m_ZoneCfg.m_fFarthestZoneRangePow2, 0, data.m_fDistance) * (m_iMaxAngle / 2);
		if (visibleAngle > 0)
			data.m_fFieldOfViewFraction = GetAngleToViewCenter(data.m_vTagWorldPos) / visibleAngle;
		else
			data.m_fFieldOfViewFraction = 1;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] position world position
	//! \return degrees between the position and the center of the view, the same in every direction
	protected float GetAngleToViewCenter(vector position)
	{
		vector direction = position - m_CameraPosition;
		float distance = direction.Length();
		if (distance == 0)
			return 0;

		return Math.Acos(Math.Clamp(vector.Dot(m_vCameraForward, direction) / distance, -1, 1)) * Math.RAD2DEG;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param[in] data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	override protected bool TraceLOS(SCR_NameTagData data)
	{
		// CSI: LOS check off in settings, every tag is visible
		if (!CSI_NametagSettings.s_bLineOfSight)
			return true;

		// CSI: trace to the body when tags are on the body
		vector targetPos;

		if (CSI_NametagSettings.s_iTagPosition == ENameTagPosition.HEAD)
			targetPos = data.m_vEntHeadPos + HEAD_LOS_OFFSET;
		else
			targetPos = data.m_vEntWorldPos + BODY_LOS_OFFSET;

		vector dirToTarget = targetPos - m_CameraPosition;
		dirToTarget.Normalize();

		float dot = vector.Dot(m_vCameraForward, dirToTarget);
		if (dot < m_fFOVThreshold)
			return false;

		IEntity targetEntity = data.m_Entity;
		if (data.GetVehicleCompartment())
			targetEntity = data.GetVehicleCompartment().GetVehicle();

		m_aTraceExcludeArray.Clear();
		m_aTraceExcludeArray.Insert(targetEntity);
		m_aTraceExcludeArray.Insert(m_CurrentPlayerTag.m_Entity);

		TraceParam param = new TraceParam();
		param.Start = m_CameraPosition;
		param.End = targetPos;
		param.Flags = TraceFlags.ANY_CONTACT | TraceFlags.WORLD | TraceFlags.ENTS;
		param.ExcludeArray = m_aTraceExcludeArray;

		param.LayerMask = TRACE_LAYER_CAMERA;
		float percent = m_World.TraceMove(param, null);
		if (percent == 1)
			return true;

		param.LayerMask = EPhysicsLayerDefs.Projectile;
		percent = m_World.TraceMove(param, TraceFilter);
		if (percent == 1)
			return true;

		return false;
	}
};
