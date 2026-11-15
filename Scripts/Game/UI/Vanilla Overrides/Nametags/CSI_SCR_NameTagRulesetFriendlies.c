//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: tag field of view from NAMETAG_FIELD_OF_VIEW, with each tag's place in it recorded;
//! 				the line of sight check can be turned off and traces to the body when tags are on the body.
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
		// CSI: m_iMaxAngle from the field of view setting; 0 = not loaded yet
		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		if (m_SettingsManager)
		{
			int fieldOfView = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW);
			if (fieldOfView > 0)
				m_iMaxAngle = CSI_GameSettings.ClampToLimits(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW, fieldOfView);
		}

		super.DetermineVisibility(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		bool visible = super.TestVisibilityFiltered(data, timeSlice);

		// CSI: record the tag's place in its visible angle, for ShouldWeClearText
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
		// CSI: LOS check off in settings, every tag is visible
		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		if (m_SettingsManager && !m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_LOS_VISIBLE))
			return true;

		// CSI: trace to the body when tags are on the body
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
