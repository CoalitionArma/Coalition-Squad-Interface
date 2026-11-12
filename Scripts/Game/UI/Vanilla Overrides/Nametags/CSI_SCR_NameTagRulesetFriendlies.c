//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the field of view tags show in (vanilla m_iMaxAngle) comes from NAMETAG_FIELD_OF_VIEW and each tag
//! 				records where it is within it (for NAMETAG_FIELD_OF_VIEW_SIMPLIFIED), the line of sight check can be turned
//! 				off (NAMETAG_LOS_VISIBLE) and traces to the body when tags are on the body.
//! 				TraceLOS is a copy of vanilla SCR_NameTagRulesetBase.TraceLOS, with CSI's lines marked "CSI:".
//!
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
modded class SCR_NameTagRulesetFriendlies : SCR_NameTagRulesetBase
{
	const vector BODY_LOS_OFFSET = "0 -0.18 0"; //!< tag visual position offset for body
	protected CSI_SettingsManager m_SettingsManager;

	//------------------------------------------------------------------------------------------------
	override protected void DetermineVisibility(float timeSlice)
	{
		// CSI: vanilla m_iMaxAngle (set in NametagFriendlies.conf) follows the player's field of view setting, kept within
		// the slider's range in case a saved file or server value is outside it. 0 means the setting isn't loaded yet,
		// so the config's value is kept.
		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		if (m_SettingsManager)
		{
			int fieldOfView = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW);
			if (fieldOfView > 0)
				m_iMaxAngle = Math.ClampInt(fieldOfView, CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_MIN, CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_MAX);
		}

		super.DetermineVisibility(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		bool visible = super.TestVisibilityFiltered(data, timeSlice);

		// CSI: where the tag is within the angle it may show in at its distance, worked out like vanilla's check above,
		// so the outer part of the field of view can be simplified (SCR_NTElementBase.ShouldWeClearText)
		float visibleAngle = Math.InverseLerp(m_ZoneCfg.m_fFarthestZoneRangePow2, 0, data.m_fDistance) * (m_iMaxAngle / 2);
		if (visibleAngle > 0)
			data.m_fFieldOfViewFraction = data.m_fAngleToScreenCenter / visibleAngle;
		else
			data.m_fFieldOfViewFraction = 1;

		return visible;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param[in] data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	override protected bool TraceLOS(SCR_NameTagData data)
	{
		// CSI: LOS check disabled in settings, every tag counts as visible (without the settings manager, trace like vanilla)
		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		if (m_SettingsManager && !m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_LOS_VISIBLE))
			return true;

		// CSI: trace to the body when tags are on the body (vanilla always traces to the head)
		vector targetPos;

		if (data.m_eAttachedTo == ENameTagPosition.HEAD)
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
