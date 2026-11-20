//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-19
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Radar inside the compass: an icon per group member in range, placed and rotated relative to the
//! 				local player. Members outside the radar's field of view fade out. CSI_HUD has it read its
//! 				settings (ApplySettings).
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

	protected ref map<int, CSI_Icon> m_mRadarIcons = new map<int, CSI_Icon>();		//!< radar icons by player ID
	protected ref array<int> m_aRadarIconsToRemove = {};							//!< reused each update
	protected ref map<int, float> m_mFieldOfViewFades = new map<int, float>();	//!< field of view fade by player ID, 1 shown to 0 faded out
	protected float m_fIconSize;				//!< px the icons were last sized to
	protected array<int> m_aCheckedGroupPlayerIDs;	//!< group list the icons were last checked against (CSI_HUDSystem's, replaced on change)
	protected float m_fFreeLookReturnTime;			//!< seconds the local icon still follows the weapon aim while the view eases back after freelook

	// Settings, read in ApplySettings
	protected bool m_bVisible;				//!< RADAR_VISIBLE
	protected int m_iRangeOnFoot;			//!< RADAR_RANGE_ON_FOOT in meters; 0 until the settings arrive
	protected int m_iRangeInVehicle;		//!< RADAR_RANGE_IN_VEHICLE in meters; 0 until the settings arrive
	protected bool m_bIconsUpright;			//!< RADAR_ICON_ARROWS_ROTATE
	protected bool m_bFieldOfViewLimited;	//!< RADAR_FIELD_OF_VIEW is below its max, so members outside it fade out
	protected float m_fHalfFieldOfView;		//!< degrees either side of straight ahead

	protected static int ICON_WIDTH_AND_HEIGHT = 16;	//!< px at 100% RADAR_ICON_SIZE
	protected static const float FIELD_OF_VIEW_FADE_OUT_TIME = 2;		//!< seconds
	protected static const float FIELD_OF_VIEW_FADE_IN_TIME = 0.25;	//!< seconds
	protected static const float AHEAD_BEARING = 180;					//!< relative bearing of straight ahead (top of the radar)
	protected static const float FREE_LOOK_RETURN_TIME = 1;			//!< seconds at most the view takes to ease back after freelook
	protected static const float FREE_LOOK_RETURNED_ANGLE = 1;			//!< degrees from the weapon aim at which the view is back

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
		// Icon widgets are children of this layout, destroyed with it
		m_mRadarIcons.Clear();
		m_mFieldOfViewFades.Clear();

		super.HandlerDeattached(w);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Reads the radar settings and sizes the icons for RADAR_ICON_SIZE. Called by CSI_HUD on a settings update.
	void ApplySettings()
	{
		if (!m_SettingsManager)
			return;

		m_bVisible = m_SettingsManager.GetSettingBool(CSI_GameSettings.RADAR_VISIBLE);
		m_iRangeOnFoot = m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_RANGE_ON_FOOT);
		m_iRangeInVehicle = m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_RANGE_IN_VEHICLE);
		m_bIconsUpright = m_SettingsManager.GetSettingBool(CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE);

		float iconSize = ICON_WIDTH_AND_HEIGHT * (m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01);
		if (iconSize != m_fIconSize)
			ResizeRadarIcons(iconSize);

		// Members outside this angle around straight ahead fade out; none at the max
		int fieldOfViewMin, fieldOfViewMax;
		m_SettingsManager.GetSettingLimits(CSI_GameSettings.RADAR_FIELD_OF_VIEW, fieldOfViewMin, fieldOfViewMax);
		int fieldOfView = m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_FIELD_OF_VIEW);
		m_fHalfFieldOfView = fieldOfView * 0.5;
		bool fieldOfViewLimited = fieldOfView < fieldOfViewMax;

		// No fades to keep without a limit. With a new limit, the members shown start fully shown, so those outside
		// it fade out
		if (!fieldOfViewLimited)
		{
			m_mFieldOfViewFades.Clear();
		}
		else if (!m_bFieldOfViewLimited)
		{
			foreach (PlayerId playerID, CSI_Icon icon : m_mRadarIcons)
			{
				m_mFieldOfViewFades.Set(playerID, 1);
			}
		}

		m_bFieldOfViewLimited = fieldOfViewLimited;
	}

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD.
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		SCR_ChimeraCharacter localCharacter = m_HUDSystem.GetLocalPlayerCharacter();

		if (!m_bVisible || !localCharacter || m_HUDSystem.GetLocalGroupCount() <= 1)
		{
			ClearRadarIcons();
			return;
		}

		// Hoist loop-invariant values
		float localYaw = m_HUDSystem.GetLocalYaw();
		vector localOrigin = localCharacter.GetOrigin();
		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();

		// Radar range in meters
		float searchRadius = m_iRangeOnFoot;
		if (m_HUDSystem.GetIsLocalPlayerInVehicle())
			searchRadius = m_iRangeInVehicle;

		// 0 until the settings arrive
		if (searchRadius <= 0)
		{
			ClearRadarIcons();
			return;
		}

		float searchRadiusFadeStart = 0.8 * searchRadius;			// icons fade from about 2/3 of the range: 0.6 opacity here, 0 at the edge
		float radarScale = 132.0 / (searchRadius * 2.725);			// meters to radar px
		float halfIconSize = m_fIconSize * 0.5;
		float searchRadiusSq = searchRadius * searchRadius;

		array<int> groupPlayerIDs = m_HUDSystem.GetLocalGroupPlayerIds();
		PlayerManager playerManager = GetGame().GetPlayerManager();

		foreach (PlayerId playerID : groupPlayerIDs)
		{
			SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
			if (!playerCharacter)
			{
				RemoveRadarIcon(playerID);
				continue;
			};

			vector playerCharacterOrigin = playerCharacter.GetOrigin();

			// Squared distance avoids a square root for out-of-range members
			float disSq = vector.DistanceSq(localOrigin, playerCharacterOrigin);

			if (disSq > searchRadiusSq)
			{
				RemoveRadarIcon(playerID);
				continue;
			};

			float dis = Math.Sqrt(disSq);

			// Defaults for the local player: center, fully shown
			float x = 0;
			float y = 0;
			float fieldOfViewFade = 1;

			if (playerID != localPlayerId)
			{
				// Bearing relative to the local player's facing
				float dir = vector.Direction(playerCharacterOrigin, localOrigin).ToYaw();
				float relDirDegrees = Math.Mod(360 - dir - localYaw, 360);
				float relDir = relDirDegrees * Math.DEG2RAD;

				float disT = dis * radarScale;
				x = Math.Sin(relDir) * disT;
				y = Math.Cos(relDir) * disT;

				if (m_bFieldOfViewLimited)
					fieldOfViewFade = UpdateFieldOfViewFade(playerID, SCR_Math.DeltaAngle(relDirDegrees, AHEAD_BEARING) <= m_fHalfFieldOfView, timeSlice);
			};

			// Clamped: the distance fade is above 1 for most of the range
			float opacity = Math.Clamp(Math.Map(dis, searchRadiusFadeStart, searchRadius, 0.6, 0), 0, 1) * fieldOfViewFade;
			float rotation;
			if (playerID == localPlayerId)
				rotation = GetLocalIconRotation(playerCharacter, localYaw, timeSlice);
			else
				rotation = -Math.Mod((CSI_EntityHelper.GetCharacterYaw(playerCharacter) - localYaw), 360);

			CSI_Icon icon = GetOrCreateRadarIcon(playerID);
			if (!icon)
				continue;

			Widget radarIcon = icon.GetRootWidget();

			// Center the icon on its position
			FrameSlot.SetPos(radarIcon, x - halfIconSize, y - halfIconSize);

			icon.SetRotation(rotation, m_bIconsUpright);
			radarIcon.SetOpacity(opacity);
		};

		// The HUD system replaces the list when the group changes
		if (groupPlayerIDs == m_aCheckedGroupPlayerIDs)
			return;

		m_aCheckedGroupPlayerIDs = groupPlayerIDs;

		// Delete icons of players who left the group or whose widget is gone
		m_aRadarIconsToRemove.Clear();
		foreach (PlayerId playerID, CSI_Icon icon : m_mRadarIcons)
		{
			if (!icon || !groupPlayerIDs.Contains(playerID))
				m_aRadarIconsToRemove.Insert(playerID);
		};

		foreach (PlayerId playerID : m_aRadarIconsToRemove)
			RemoveRadarIcon(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Sizes every icon; new icons get the size when created.
	//! \param[in] iconSize px
	protected void ResizeRadarIcons(float iconSize)
	{
		m_fIconSize = iconSize;
		foreach (CSI_Icon icon : m_mRadarIcons)
		{
			if (icon && icon.GetRootWidget())
				FrameSlot.SetSize(icon.GetRootWidget(), iconSize, iconSize);
		}
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return the icon, or null if it couldn't be created
	protected CSI_Icon GetOrCreateRadarIcon(PlayerId playerID)
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

		// Anchor to the radar's center; radar icons show arrows on turreted seats
		FrameSlot.SetAnchor(radarIcon, 0.5, 0.5);
		FrameSlot.SetSize(radarIcon, m_fIconSize, m_fIconSize);
		icon.SetIsRadarWidget(true);
		icon.IconUpdate(playerID);

		m_mRadarIcons.Set(playerID, icon);
		return icon;
	}

	//------------------------------------------------------------------------------------------------
	//! On foot the view is where the local player faces, so the icon points straight up, except while freelooking:
	//! then the weapon aim is, which stays put as the view turns. After freelook it follows the weapon aim until the
	//! view has eased back to it. Both aims trail the view on fast turns, so neither is read otherwise. In a seat,
	//! the turret aim or vehicle yaw, as for other members.
	//! \param[in] localCharacter
	//! \param[in] localYaw the view's yaw, negated
	//! \param[in] timeSlice
	//! \return icon rotation in degrees
	protected float GetLocalIconRotation(notnull SCR_ChimeraCharacter localCharacter, float localYaw, float timeSlice)
	{
		if (CSI_EntityHelper.GetCharacterVehicleCompartment(localCharacter))
		{
			m_fFreeLookReturnTime = 0;
			return -Math.Mod((CSI_EntityHelper.GetCharacterYaw(localCharacter) - localYaw), 360);
		}

		CharacterControllerComponent characterController = localCharacter.GetCharacterController();
		if (!characterController)
			return 0;

		bool freeLooking = characterController.IsFreeLookEnabled();
		if (freeLooking)
			m_fFreeLookReturnTime = FREE_LOOK_RETURN_TIME;
		else if (m_fFreeLookReturnTime > 0)
			m_fFreeLookReturnTime -= timeSlice;
		else
			return 0;

		AimingComponent weaponAimingComponent = characterController.GetAimingComponent();
		if (!weaponAimingComponent)
			return 0;

		float rotation = -Math.Mod((-weaponAimingComponent.GetAimingDirectionWorld().ToYaw() - localYaw), 360);

		// Eased back: the view is the facing again
		if (!freeLooking && SCR_Math.DeltaAngle(rotation, 0) < FREE_LOOK_RETURNED_ANGLE)
			m_fFreeLookReturnTime = 0;

		return rotation;
	}

	//------------------------------------------------------------------------------------------------
	//! Steps a member's fade towards shown or faded out; new members start at their target.
	//! \param[in] playerID
	//! \param[in] inFieldOfView
	//! \param[in] timeSlice
	//! \return 1 shown to 0 faded out
	protected float UpdateFieldOfViewFade(PlayerId playerID, bool inFieldOfView, float timeSlice)
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
	protected void RemoveRadarIcon(PlayerId playerID)
	{
		// Called every frame for members out of range; most have no icon
		CSI_Icon icon;
		if (!m_mRadarIcons.Find(playerID, icon))
			return;

		m_mFieldOfViewFades.Remove(playerID);
		m_mRadarIcons.Remove(playerID);

		if (icon && icon.GetRootWidget())
			icon.GetRootWidget().RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearRadarIcons()
	{
		m_mFieldOfViewFades.Clear();

		if (m_mRadarIcons.IsEmpty())
			return;

		foreach (PlayerId playerID, CSI_Icon icon : m_mRadarIcons)
		{
			if (icon && icon.GetRootWidget())
				icon.GetRootWidget().RemoveFromHierarchy();
		}

		m_mRadarIcons.Clear();
	}
}
