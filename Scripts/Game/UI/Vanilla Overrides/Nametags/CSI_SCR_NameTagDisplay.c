//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the "Near" zone's range comes from NAMETAG_RANGE, multiplied while zoomed; hidden and
//! 				not updated while NAMETAG_VISIBLE is off; NAMETAG_BRIGHTNESS_DIMMING sets the opacity instead of
//! 				the HUD's adaptive opacity.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected static const float UNDIMMED_OPACITY = 0.85;				//!< opacity with NAMETAG_BRIGHTNESS_DIMMING off
	protected static const float BRIGHTNESS_OPACITY_THROTTLE = 0.01;	//!< smallest opacity change applied

	protected CSI_SettingsManager m_SettingsManager;
	protected SCR_NameTagZone m_CachedNearZone;
	protected int m_iLastNametagsRange = -1;	//!< last range applied
	protected bool m_b2DSightsActive = false;
	protected bool m_bNametagsVisible = true;	//!< NAMETAG_VISIBLE
	protected float m_fBrightnessOpacity = 1;	//!< last opacity applied by UpdateBrightnessOpacity

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// Nothing to update while hidden
		if (!m_bNametagsVisible)
			return;

		super.DisplayUpdate(owner, timeSlice);

		if (!m_SettingsManager)
			m_SettingsManager = CSI_SettingsManager.GetInstance();

		UpdateBrightnessOpacity();

		if (!m_SettingsManager || !m_CurrentPlayerTag || !m_CurrentPlayerTag.m_CharController || !SCR_PlayerController.s_pLocalPlayerController)
			return;

		bool isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || m_b2DSightsActive || SCR_PlayerController.s_pLocalPlayerController.GetIsBinocularsZoomed()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();
		int baseRange = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_RANGE);
		int nametagsRange;
		if (isZoomed)
			nametagsRange = baseRange * m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION) / 100; // increase max distance if player is using a scope or binoculars
		else
			nametagsRange = baseRange;

		if (nametagsRange != m_iLastNametagsRange)
		{
			// A larger range gathers entities on the next update, not on the collector's next timed gather
			if (nametagsRange > m_iLastNametagsRange && s_NametagCfg.m_aSourceEntities)
				s_NametagCfg.m_aSourceEntities.RequestGather();

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
	//! Records whether 2D sights are zoomed; DisplayUpdate applies the range.
	//! \param[in] adjust true while zoomed
	//! \param[in] FOVZoomed unused
	protected override void AdjustRange(bool adjust, float FOVZoomed)
	{
		if (adjust)
			m_b2DSightsActive = true;
		else
			m_b2DSightsActive = false;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Insert(OnSettingsUpdate);

		OnSettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);

		if (m_SettingsManager)
			m_SettingsManager.GetOnSettingsUpdate().Remove(OnSettingsUpdate);

		// The zone belonged to the config super just dropped
		m_CachedNearZone = null;
		m_iLastNametagsRange = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows or hides the display for NAMETAG_VISIBLE.
	protected void OnSettingsUpdate()
	{
		bool nametagsVisible = !m_SettingsManager || m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_VISIBLE);
		if (nametagsVisible == m_bNametagsVisible)
			return;

		m_bNametagsVisible = nametagsVisible;
		Show(m_bShown);

		// Entities that came in range while hidden are gathered at once
		if (nametagsVisible && s_NametagCfg && s_NametagCfg.m_aSourceEntities)
			s_NametagCfg.m_aSourceEntities.RequestGather();
	}

	//------------------------------------------------------------------------------------------------
	override void Show(bool show, float speed = UIConstants.FADE_RATE_INSTANT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		// Also hidden while NAMETAG_VISIBLE is off; m_bShown keeps the requested state
		super.Show(show && m_bNametagsVisible, speed, curve);

		m_bShown = show;
	}

	//------------------------------------------------------------------------------------------------
	//! Applies CSI's brightness opacity instead of the HUD manager's adaptive opacity.
	//! \param[in] opacity unused
	//! \param[in] sceneBrightness
	//! \param[in] init
	override protected void AdaptiveOpacity_Update(float opacity, float sceneBrightness, bool init = false)
	{
		super.AdaptiveOpacity_Update(m_fBrightnessOpacity, sceneBrightness, init);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the display's opacity from scene brightness, or to UNDIMMED_OPACITY with NAMETAG_BRIGHTNESS_DIMMING off.
	protected void UpdateBrightnessOpacity()
	{
		float opacity = UNDIMMED_OPACITY;

		BaseWorld world = GetGame().GetWorld();
		CameraManager cameraManager = GetGame().GetCameraManager();

		bool dimWithBrightness = !m_SettingsManager || m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_BRIGHTNESS_DIMMING);

		if (world && cameraManager && dimWithBrightness)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			if (camera)
			{
				int cameraIndex = camera.GetCameraIndex();

				float brightness = Math.Clamp(world.GetCameraSceneMiddleBrightness(cameraIndex), 0.1, 10);

				float hdrBrightnessFactor = world.GetCameraHDRBrightness(cameraIndex);
				if (hdrBrightnessFactor > 1) // Only let HDR raise brightness (NVGs)
					brightness = brightness * hdrBrightnessFactor;

				opacity = Math.Clamp(brightness, 0.1, 1);
			}
		}

		if (Math.AbsFloat(opacity - m_fBrightnessOpacity) < BRIGHTNESS_OPACITY_THROTTLE)
			return;

		m_fBrightnessOpacity = opacity;
		super.AdaptiveOpacity_Update(opacity, 0);
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: adds SetZoneEnd to resize a zone at runtime.
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
//! \Last Updated: 	2026-11-14
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: adds ResetFarthestZone to recompute the farthest zone range after a zone is resized.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
modded class SCR_NameTagConfig
{
	//------------------------------------------------------------------------------------------------
	//! Recomputes the farthest zone range values from the current zones.
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

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: adds RequestGather so a larger nametag range gathers entities at once.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
modded class SCR_NameTagCollectorBase
{
	//------------------------------------------------------------------------------------------------
	//! Gathers entities on the next UpdatePerformed instead of after m_fGatherEntitiesTimer.
	void RequestGather()
	{
		m_fQueryCooldown = m_fGatherEntitiesTimer;
	}
}