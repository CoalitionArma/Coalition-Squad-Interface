//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Radar inside the compass: an icon per group member within range, positioned and rotated relative to the
//! 				local player. Icons are created from CSI_Icon.layout when a member comes into range and deleted when they leave it.
//! 				The range comes from RADAR_RANGE_ON_FOOT or RADAR_RANGE_IN_VEHICLE. Members outside the RADAR_FIELD_OF_VIEW
//! 				angle around where the local player looks slowly fade out.
//!
//------------------------------------------------------------------------------------------------
class CSI_Radar : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;

	protected Widget m_wRadarFrame;	//!< parent of the icon widgets

	protected ref map<int, CSI_Icon> m_mRadarIcons = new map<int, CSI_Icon>();		//!< icons on the radar by player ID: created when a member comes into range, deleted when they leave it
	protected ref array<int> m_aRadarIconsToRemove = {};							//!< reused each update
	protected ref map<int, float> m_mFieldOfViewFades = new map<int, float>();	//!< field of view fade by player ID, 1 shown to 0 faded out; kept as long as the icon

	protected static int ICON_WIDTH_AND_HEIGHT = 16;	//!< px at 100% RADAR_ICON_SIZE
	protected static const float FIELD_OF_VIEW_FADE_OUT_TIME = 2;		//!< seconds for a member who leaves the field of view to fade out
	protected static const float FIELD_OF_VIEW_FADE_IN_TIME = 0.25;	//!< seconds for a member back in the field of view to fade in
	protected static const float AHEAD_BEARING = 180;					//!< relative bearing (as worked out in Update) of straight ahead, the top of the radar

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();

		m_wRadarFrame = w.FindAnyWidget("RadarFrame");
		if (!m_wRadarFrame)
			m_wRadarFrame = w;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		// The icon widgets are children of this layout and get destroyed with it
		m_mRadarIcons.Clear();
		m_mFieldOfViewFades.Clear();

		super.HandlerDeattached(w);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD.
	//! \param[in] timeSlice seconds since the last update, for the field of view fade
	void Update(float timeSlice)
	{
		SCR_ChimeraCharacter localCharacter = m_HUDSystem.GetLocalPlayerCharacter();

		if (!localCharacter || m_HUDSystem.GetLocalGroupCount() <= 1 || !m_SettingsManager.GetSettingBool(CSI_GameSettings.RADAR_VISIBLE))
		{
			ClearRadarIcons();
			return;
		}

		// Hoist loop-invariant values
		float localYaw = m_HUDSystem.GetLocalYaw();
		vector localOrigin = localCharacter.GetOrigin();
		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();

		// Radar range in metres from the settings, one for on foot and one for in vehicles
		float searchRadius;
		if (m_HUDSystem.GetIsLocalPlayerInVehicle())
			searchRadius = Math.ClampInt(m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_RANGE_IN_VEHICLE), CSI_GameSettings.RADAR_RANGE_IN_VEHICLE_MIN, CSI_GameSettings.RADAR_RANGE_IN_VEHICLE_MAX);
		else
			searchRadius = Math.ClampInt(m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_RANGE_ON_FOOT), CSI_GameSettings.RADAR_RANGE_ON_FOOT_MIN, CSI_GameSettings.RADAR_RANGE_ON_FOOT_MAX);

		float searchRadiusFadeStart = 0.8 * searchRadius;			// icons fade out over the last 20% of the range
		float radarScale = 132.0 / (searchRadius * 2.725);			// metres to radar px
		float iconSize = ICON_WIDTH_AND_HEIGHT * (m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01);

		float searchRadiusSq = searchRadius * searchRadius;

		// Members further than this from straight ahead fade out; the full 360 shows everyone
		int fieldOfView = Math.ClampInt(m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_FIELD_OF_VIEW), CSI_GameSettings.RADAR_FIELD_OF_VIEW_MIN, CSI_GameSettings.RADAR_FIELD_OF_VIEW_MAX);
		float halfFieldOfView = fieldOfView * 0.5;

		array<int> groupPlayerIDs = m_HUDSystem.GetLocalGroupPlayerIds();
		PlayerManager playerManager = GetGame().GetPlayerManager();

		foreach (int playerID : groupPlayerIDs)
		{
			SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
			if (!playerCharacter)
			{
				RemoveRadarIcon(playerID);
				continue;
			};

			vector playerCharacterOrigin = playerCharacter.GetOrigin();

			// Range check on squared distance (no square root) - most group members are usually out of radar range
			float disSq = vector.DistanceSq(localOrigin, playerCharacterOrigin);

			if (disSq > searchRadiusSq)
			{
				RemoveRadarIcon(playerID);
				continue;
			};

			float dis = Math.Sqrt(disSq);

			// The local player stays in the centre of the radar, always shown
			float x = 0;
			float y = 0;
			float fieldOfViewFade = 1;

			if (playerID != localPlayerId)
			{
				// Bearing to the player relative to where the local player is facing
				float dir = vector.Direction(playerCharacterOrigin, localOrigin).ToYaw();
				float relDirDegrees = Math.Mod(360 - dir - localYaw, 360);
				float relDir = relDirDegrees * Math.DEG2RAD;

				float disT = dis * radarScale;
				x = Math.Sin(relDir) * disT;
				y = Math.Cos(relDir) * disT;

				bool inFieldOfView = fieldOfView >= CSI_GameSettings.RADAR_FIELD_OF_VIEW_MAX || SCR_Math.DeltaAngle(relDirDegrees, AHEAD_BEARING) <= halfFieldOfView;
				fieldOfViewFade = UpdateFieldOfViewFade(playerID, inFieldOfView, timeSlice);
			};

			// Clamped so the field of view fade shows: the distance fade is above 1 for most of the range
			float opacity = Math.Clamp(Math.Map(dis, searchRadiusFadeStart, searchRadius, 0.6, 0), 0, 1) * fieldOfViewFade;
			float rotation = -Math.Mod((CSI_EntityHelper.GetCharacterYaw(playerCharacter) - localYaw), 360);

			CSI_Icon icon = GetOrCreateRadarIcon(playerID);
			if (!icon)
				continue;

			Widget radarIcon = icon.GetRootWidget();

			// Centre the icon on its position
			FrameSlot.SetPos(radarIcon, (x - iconSize/2), (y - iconSize/2));
			FrameSlot.SetSize(radarIcon, iconSize, iconSize);

			icon.SetRotation(rotation);
			radarIcon.SetOpacity(opacity);
		};

		// Delete icons of players who have left the group (or whose widget no longer exists)
		m_aRadarIconsToRemove.Clear();
		foreach (int playerID, CSI_Icon icon : m_mRadarIcons)
		{
			if (!icon || !groupPlayerIDs.Contains(playerID))
				m_aRadarIconsToRemove.Insert(playerID);
		};

		foreach (int playerID : m_aRadarIconsToRemove)
			RemoveRadarIcon(playerID);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Returns the radar icon for a player, creating it from CSI_Icon.layout if they don't have one yet.
	//! \param[in] playerID
	//! \return the icon's handler, or null if the widget couldn't be created
	protected CSI_Icon GetOrCreateRadarIcon(int playerID)
	{
		CSI_Icon icon = m_mRadarIcons.Get(playerID);
		if (icon)
			return icon;

		Widget radarIcon = GetGame().GetWorkspace().CreateWidgets(CSI_UIHelper.CSI_ICON_LAYOUT, m_wRadarFrame);
		if (!radarIcon)
			return null;

		icon = CSI_Icon.Cast(radarIcon.FindHandler(CSI_Icon));
		if (!icon)
		{
			radarIcon.RemoveFromHierarchy();
			return null;
		};

		// Same setup the icons placed in CSI_Radar.layout had: anchored to the radar's centre, radar arrows on turreted seats
		FrameSlot.SetAnchor(radarIcon, 0.5, 0.5);
		icon.SetIsRadarWidget(true);
		icon.IconUpdate(playerID);

		m_mRadarIcons.Set(playerID, icon);
		return icon;
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a member's field of view fade towards shown or faded out. A member new on the radar starts at the end they
	//! belong at, rather than fading in or out as they appear.
	//! \param[in] playerID
	//! \param[in] inFieldOfView
	//! \param[in] timeSlice seconds since the last update
	//! \return the fade, 1 shown to 0 faded out
	protected float UpdateFieldOfViewFade(int playerID, bool inFieldOfView, float timeSlice)
	{
		float fade;
		if (!m_mFieldOfViewFades.Find(playerID, fade))
		{
			fade = 0;
			if (inFieldOfView)
				fade = 1;
		}
		else if (inFieldOfView)
		{
			fade = Math.Min(fade + timeSlice / FIELD_OF_VIEW_FADE_IN_TIME, 1);
		}
		else
		{
			fade = Math.Max(fade - timeSlice / FIELD_OF_VIEW_FADE_OUT_TIME, 0);
		}

		m_mFieldOfViewFades.Set(playerID, fade);
		return fade;
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes a player's radar icon, if they have one.
	//! \param[in] playerID
	protected void RemoveRadarIcon(int playerID)
	{
		m_mFieldOfViewFades.Remove(playerID);

		CSI_Icon icon = m_mRadarIcons.Get(playerID);
		if (!icon && !m_mRadarIcons.Contains(playerID))
			return;

		m_mRadarIcons.Remove(playerID);

		if (icon && icon.GetRootWidget())
			icon.GetRootWidget().RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes every radar icon.
	protected void ClearRadarIcons()
	{
		m_mFieldOfViewFades.Clear();

		if (m_mRadarIcons.IsEmpty())
			return;

		// Every entry goes, so delete the widgets and clear the map in one go (no key copy needed)
		foreach (int playerID, CSI_Icon icon : m_mRadarIcons)
		{
			if (icon && icon.GetRootWidget())
				icon.GetRootWidget().RemoveFromHierarchy();
		}

		m_mRadarIcons.Clear();
	}
}
