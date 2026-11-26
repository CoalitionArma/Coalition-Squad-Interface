//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-26
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the "Near" zone's range comes from NAMETAG_RANGE, multiplied while zoomed; hidden and
//! 				not updated while NAMETAG_VISIBLE is off; NAMETAG_BRIGHTNESS_DIMMING fades it by scene brightness and,
//! 				after sunset, the sun (NIGHT_OPACITY), and follows a night vision mod's gain. Reads the nametag
//! 				settings into CSI_NametagSettings on every settings update and holds the local group for the tags.
//!
//------------------------------------------------------------------------------------------------
modded class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{
	protected static const float UNDIMMED_OPACITY = 0.85;	//!< opacity with NAMETAG_BRIGHTNESS_DIMMING off
	protected static const float OPACITY_THROTTLE = 0.01;	//!< smallest opacity change applied
	protected static const float SUN_SET_HEIGHT = 0.1;	//!< sun direction height at which it has fully set
	protected static const float NIGHT_OPACITY = 0.2;	//!< highest opacity once the sun has set, wherever the player looks (the night sky reads brighter than the ground)

	// Dim in the dark: opacity by scene brightness, linear between points. About 0.15 and up is daylight, 0.03 a lit
	// area at night, 0.013 a dark field at night.
	protected static const ref array<float> DIM_CURVE_BRIGHTNESS = {0, 0.03, 0.09, 0.15};
	protected static const ref array<float> DIM_CURVE_OPACITY = {0.05, 0.297, 0.753, 1};

	// Night vision mods hold a set exposure (e.g. 0.05, 0.3, 3); auto exposure moves and lands between such values (e.g. 1.67153)
	protected static const float FIXED_EXPOSURE_STEP = 0.001;		//!< set exposures are whole multiples of this
	protected static const float FIXED_EXPOSURE_TOLERANCE = 0.000001;	//!< share of the exposure it may be off a whole multiple (float precision)
	// Under night vision the opacity follows the set exposure (the mod's gain) on a log scale, as its picture brightens with it
	protected static const float NIGHT_VISION_DIM_EXPOSURE = 0.1;		//!< exposure at and below which nametags are at NIGHT_VISION_DIM_OPACITY
	protected static const float NIGHT_VISION_DIM_OPACITY = 0.15;
	protected static const float NIGHT_VISION_BRIGHT_EXPOSURE = 10;		//!< exposure at and above which nametags are at NIGHT_VISION_BRIGHT_OPACITY
	protected static const float NIGHT_VISION_BRIGHT_OPACITY = 1;

	static ref array<int> s_aLocalGroupPlayerIds;	//!< the local player's group (CSI_HUDSystem's current list), for SCR_NameTagData.UpdateCSIState

	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManager;
	protected float m_fSunsetAmount;			//!< 0 while the sun is up, 1 once it has fully set

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;
	protected SCR_NameTagZone m_CachedNearZone;
	protected int m_iNametagRange;				//!< NAMETAG_RANGE
	protected int m_iNametagMagnification;		//!< NAMETAG_MAGNIFICATION_MULTIPLICATION, percent
	protected int m_iLastNametagsRange = -1;	//!< last range applied
	protected bool m_b2DSightsActive = false;
	protected bool m_bNametagsVisible = true;	//!< NAMETAG_VISIBLE
	protected bool m_bDimInDark = true;			//!< NAMETAG_BRIGHTNESS_DIMMING
	protected float m_fSceneBrightness = 1;		//!< camera's scene middle brightness at the last update
	protected bool m_bNightVision;				//!< a night vision mod holds the exposure
	protected float m_fExposure = -1;			//!< camera's HDR exposure at the last update, -1 before the first
	protected float m_fAppliedOpacity = -1;		//!< opacity last applied

	protected static const float NAME_REFRESH_INTERVAL = 2;	//!< s between RefreshNames
	protected float m_fNameRefreshTime;			//!< s until the next RefreshNames

	protected static const float SUN_READ_INTERVAL = 0.5;	//!< s between reads of the sun's height, which moves slowly
	protected float m_fSunReadTime;				//!< s until the next read of the sun's height

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_SettingsManager)
			ConnectSettingsManager();

		// Nothing to update while hidden
		if (!m_bNametagsVisible)
			return;

		// The local group, for the tags drawn below
		if (!m_HUDSystem)
			m_HUDSystem = CSI_HUDSystem.GetInstance();

		if (m_HUDSystem)
			s_aLocalGroupPlayerIds = m_HUDSystem.GetLocalGroupPlayerIds();

		super.DisplayUpdate(owner, timeSlice);

		UpdateBrightnessOpacity(timeSlice);

		m_fNameRefreshTime -= timeSlice;
		if (m_fNameRefreshTime <= 0)
		{
			m_fNameRefreshTime = NAME_REFRESH_INTERVAL;
			RefreshNames();
		}

		if (!m_SettingsManager || !m_CurrentPlayerTag || !m_CurrentPlayerTag.m_CharController || !SCR_PlayerController.s_pLocalPlayerController)
			return;

		bool isZoomed = (SCR_2DPIPSightsComponent.IsPIPActive() || m_b2DSightsActive || SCR_PlayerController.s_pLocalPlayerController.GetIsBinocularsZoomed()) && !m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled();
		int nametagsRange = m_iNametagRange;
		if (isZoomed)
			nametagsRange = m_iNametagRange * m_iNametagMagnification / 100; // increase max distance if player is using a scope or binoculars

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

		ConnectSettingsManager();
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
		s_aLocalGroupPlayerIds = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Follows the settings manager's updates and applies its settings; DisplayUpdate tries again while there is none.
	protected void ConnectSettingsManager()
	{
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		if (!m_SettingsManager)
			return;

		m_SettingsManager.GetOnSettingsUpdate().Insert(OnSettingsUpdate);
		OnSettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the settings the nametag classes use (CSI_NametagSettings), shows or hides the display for
	//! NAMETAG_VISIBLE, reapplies the opacity for NAMETAG_BRIGHTNESS_DIMMING, and reapplies element defaults (role
	//! icons, their position, size, group names) to tags already shown, which vanilla otherwise does only on a zone
	//! or state change.
	protected void OnSettingsUpdate()
	{
		if (m_SettingsManager)
		{
			CSI_NametagSettings.Refresh(m_SettingsManager);
			m_iNametagRange = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_RANGE);
			m_iNametagMagnification = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION);
		}

		// The icon-only distance follows NAMTEAG_RANGE_SIMPLIFIED
		array<ref SCR_NameTagZone> zones = GetNametagZones();
		if (zones)
		{
			foreach (SCR_NameTagZone zone : zones)
			{
				zone.UpdateIconOnlyDistance();
			}
		}

		foreach (SCR_NameTagData tag : m_aNameTags)
		{
			if (tag)
				UpdateTagElements(tag);
		}

		// Rank (RANK_VISIBLE) is part of the name, which vanilla rebuilds only on a name update
		RefreshNames();

		bool dimInDark = !m_SettingsManager || m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_BRIGHTNESS_DIMMING);
		if (dimInDark != m_bDimInDark)
		{
			m_bDimInDark = dimInDark;
			ApplyNametagOpacity();
		}

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
	//! Also clears the tag's zone, so a later state change can't show the pooled tag again.
	override void CleanupTag(notnull SCR_NameTagData data, bool removeFromArray = true)
	{
		super.CleanupTag(data, removeFromArray);
		data.m_iZoneID = -1;

		// A pooled tag isn't drawn: stop its talking bars, and load its icon again when it's reused
		for (int index = 0; index < SCR_NameTagData.ELEMENT_COUNT; index++)
		{
			ImageWidget icon = ImageWidget.Cast(data.m_aNametagElements[index]);
			if (!icon)
				continue;

			CSI_TalkingBarsComponent talkingBars = CSI_TalkingBarsComponent.Find(icon);
			if (talkingBars && talkingBars.IsActive())
			{
				talkingBars.SetActive(false);
				data.m_aElementIconKeys[index] = SCR_NTIconBase.ICON_KEY_NONE;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rebuilds the name (with rank) and group name of the tags in view. Vanilla does it only when a tag is hidden
	//! and shown again, so they would keep an old rank, rank setting or squad until they left view.
	protected void RefreshNames()
	{
		foreach (SCR_NameTagData tag : m_aNameTags)
		{
			// A hidden tag gets both when it's shown again
			if (!tag || !(tag.m_Flags & ENameTagFlags.VISIBLE))
				continue;

			tag.m_Flags |= ENameTagFlags.NAME_UPDATE;
			tag.GetGroupName();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Show(bool show, float speed = UIConstants.FADE_RATE_INSTANT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		// Also hidden while NAMETAG_VISIBLE is off; m_bShown keeps the requested state
		super.Show(show && m_bNametagsVisible, speed, curve);

		m_bShown = show;
	}

	//------------------------------------------------------------------------------------------------
	//! Applies GetNametagOpacity instead of the HUD manager's adaptive opacity, which stays bright in lit areas at night.
	//! \param[in] opacity unused
	//! \param[in] sceneBrightness
	//! \param[in] init
	override protected void AdaptiveOpacity_Update(float opacity, float sceneBrightness, bool init = false)
	{
		m_fAppliedOpacity = GetNametagOpacity();
		super.AdaptiveOpacity_Update(m_fAppliedOpacity, sceneBrightness, init);
	}

	//------------------------------------------------------------------------------------------------
	//! \return GetDimInDarkOpacity for the scene, capped by NIGHT_OPACITY after sunset; GetNightVisionOpacity under
	//! night vision; UNDIMMED_OPACITY with NAMETAG_BRIGHTNESS_DIMMING off
	protected float GetNametagOpacity()
	{
		if (!m_bDimInDark)
			return UNDIMMED_OPACITY;

		// Night vision's scene reads dark whatever its picture looks like
		if (m_bNightVision)
			return GetNightVisionOpacity();

		// The scene reads brighter looking at the sky; the sun's height doesn't depend on the view
		float opacity = Math.Min(GetDimInDarkOpacity(m_fSceneBrightness), Math.Lerp(1, NIGHT_OPACITY, m_fSunsetAmount));

		return Math.Clamp(opacity, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] sceneBrightness camera's scene middle brightness
	//! \return opacity on the DIM_CURVE_BRIGHTNESS / DIM_CURVE_OPACITY curve, its end values beyond it
	protected float GetDimInDarkOpacity(float sceneBrightness)
	{
		if (sceneBrightness <= DIM_CURVE_BRIGHTNESS[0])
			return DIM_CURVE_OPACITY[0];

		for (int i = 1, count = DIM_CURVE_BRIGHTNESS.Count(); i < count; i++)
		{
			if (sceneBrightness < DIM_CURVE_BRIGHTNESS[i])
				return Math.Lerp(DIM_CURVE_OPACITY[i - 1], DIM_CURVE_OPACITY[i], Math.InverseLerp(DIM_CURVE_BRIGHTNESS[i - 1], DIM_CURVE_BRIGHTNESS[i], sceneBrightness));
		}

		return DIM_CURVE_OPACITY[DIM_CURVE_OPACITY.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! \return opacity for the night vision mod's set exposure, from NIGHT_VISION_DIM_OPACITY to NIGHT_VISION_BRIGHT_OPACITY
	protected float GetNightVisionOpacity()
	{
		float gain = Math.InverseLerp(Math.Log10(NIGHT_VISION_DIM_EXPOSURE), Math.Log10(NIGHT_VISION_BRIGHT_EXPOSURE), Math.Log10(m_fExposure));
		return Math.Lerp(NIGHT_VISION_DIM_OPACITY, NIGHT_VISION_BRIGHT_OPACITY, Math.Clamp(gain, 0, 1));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] exposure HDR exposure
	//! \return true if it's a whole multiple of FIXED_EXPOSURE_STEP, as a set exposure is and auto exposure isn't
	protected bool IsFixedExposure(float exposure)
	{
		float nearestFixed = Math.Round(exposure / FIXED_EXPOSURE_STEP) * FIXED_EXPOSURE_STEP;
		return Math.AbsFloat(exposure - nearestFixed) <= exposure * FIXED_EXPOSURE_TOLERANCE;
	}

	//------------------------------------------------------------------------------------------------
	//! \return 0 while the sun is up, rising to 1 once it has fully set (SUN_SET_HEIGHT)
	protected float GetSunsetAmount()
	{
		if (!m_TimeAndWeatherManager)
		{
			ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
			if (world)
				m_TimeAndWeatherManager = world.GetTimeAndWeatherManager();

			if (!m_TimeAndWeatherManager)
				return 0;
		}

		vector sunDirection, moonDirection;
		float moonPhase;
		m_TimeAndWeatherManager.GetCurrentSunMoonDirAndPhase(sunDirection, moonDirection, moonPhase);
		return Math.Clamp(sunDirection[1] / SUN_SET_HEIGHT, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Applies GetNametagOpacity if it changed by OPACITY_THROTTLE.
	protected void ApplyNametagOpacity()
	{
		float opacity = GetNametagOpacity();
		if (Math.AbsFloat(opacity - m_fAppliedOpacity) < OPACITY_THROTTLE)
			return;

		m_fAppliedOpacity = opacity;
		super.AdaptiveOpacity_Update(opacity, m_fSceneBrightness);
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the scene brightness, sun and night vision, then applies the opacity (ApplyNametagOpacity skips small changes).
	//! \param[in] timeSlice
	protected void UpdateBrightnessOpacity(float timeSlice)
	{
		if (!m_bDimInDark)
			return;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		m_fSunReadTime -= timeSlice;
		if (m_fSunReadTime <= 0)
		{
			m_fSunReadTime = SUN_READ_INTERVAL;
			m_fSunsetAmount = GetSunsetAmount();
		}

		// Camera 0, as the HUD manager. Read every frame: night vision is told by the exposure not moving between frames
		m_fSceneBrightness = world.GetCameraSceneMiddleBrightness(0);

		float exposure = world.GetCameraHDRBrightness(0);
		if (exposure <= 0)
			return;

		// Night vision: the exposure is held at a set value; it ends as soon as auto exposure takes over again
		m_bNightVision = exposure == m_fExposure && IsFixedExposure(exposure);
		m_fExposure = exposure;

		ApplyNametagOpacity();
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: adds SetZoneEnd to resize a zone at runtime and the zone's icon-only distance, and
//! 				refreshes a tag's CSI state (SCR_NameTagData.UpdateCSIState) before its elements are drawn or reset.
//!
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagZoneTitle()]
modded class SCR_NameTagZone
{
	float m_fIconOnlyDistanceSq = float.MAX;	//!< squared distance beyond which a player's tag shows only its icon (CSI_NametagSettings.GetIconOnlyDistanceSq)

	//------------------------------------------------------------------------------------------------
	//! \param[in] range zone end in meters
	void SetZoneEnd(int range)
	{
		m_iZoneEnd = range;
		m_iZoneEndSq = range * range;
		UpdateIconOnlyDistance();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets m_fIconOnlyDistanceSq for NAMTEAG_RANGE_SIMPLIFIED.
	void UpdateIconOnlyDistance()
	{
		m_fIconOnlyDistanceSq = CSI_NametagSettings.GetIconOnlyDistanceSq(CSI_NametagSettings.s_fIconOnlyRange, m_iZoneStart, m_iZoneEnd);
	}

	//------------------------------------------------------------------------------------------------
	override void OnZoneChanged(SCR_NameTagData data)
	{
		data.UpdateCSIState(this);
		super.OnZoneChanged(data);
	}

	//------------------------------------------------------------------------------------------------
	override void DrawElements(SCR_NameTagData data)
	{
		data.UpdateCSIState(this);
		super.DrawElements(data);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElementDefs(SCR_NameTagData data)
	{
		data.UpdateCSIState(this);
		super.UpdateElementDefs(data);
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