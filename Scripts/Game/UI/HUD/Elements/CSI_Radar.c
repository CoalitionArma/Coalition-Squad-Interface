//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-22
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Radar inside the compass: an icon per group member in range, placed and rotated relative to the
//! 				local player. Members outside the radar's field of view fade out. With the vehicle outline shown,
//! 				it also shows everyone aboard without a group, centered on the outline. CSI_HUD has it read its
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
	protected ref array<int> m_aShownPlayerIDs = {};	//!< group list plus the local player and passengers outside the group, when needed
	protected ref array<int> m_aPassengerPlayerIDs = {};	//!< players outside the group in the local player's vehicle
	protected ref array<int> m_aNewPassengerPlayerIDs = {};	//!< reused each update

	protected ref array<int> m_aOccupantPlayerIDs = {};		//!< every player in the local player's vehicle, them included
	protected float m_fFreeLookReturnTime;			//!< seconds the local icon still follows the weapon aim while the view eases back after freelook

	// Seats, refreshed when they change
	protected ref CSI_SeatInfo m_LocalSeat = new CSI_SeatInfo();
	protected ref map<int, ref CSI_SeatInfo> m_mIconSeats = new map<int, ref CSI_SeatInfo>();	//!< other members' seats by player ID, while they have an icon

	protected CSI_VehicleOutlineElement m_VehicleOutline;	//!< updated before the radar; null without one

	// Settings, read in ApplySettings
	protected bool m_bVisible;				//!< RADAR_VISIBLE
	protected int m_iRangeOnFoot;			//!< RADAR_RANGE_ON_FOOT in meters; 0 until the settings arrive
	protected int m_iRangeInVehicle;		//!< RADAR_RANGE_IN_VEHICLE in meters; 0 until the settings arrive
	protected bool m_bIconsUpright;			//!< RADAR_ICON_ARROWS_ROTATE
	protected bool m_bFieldOfViewLimited;	//!< RADAR_FIELD_OF_VIEW is below its max, so members outside it fade out
	protected float m_fHalfFieldOfView;		//!< degrees either side of straight ahead

	static const int ICON_WIDTH_AND_HEIGHT = 16;						//!< px at 100% RADAR_ICON_SIZE
	static const float RADAR_SIZE = 132;								//!< radar frame width in layout units
	protected static const float SIZE_PER_RANGE = 2.725;				//!< radar frame width in ranges
	protected static const float KEEP_RANGE_SHARE = 1.05;				//!< of the range, up to which an existing icon is kept (hidden)
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
	void SetVehicleOutline(CSI_VehicleOutlineElement vehicleOutline)
	{
		m_VehicleOutline = vehicleOutline;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		// Icon widgets are children of this layout, destroyed with it
		m_mRadarIcons.Clear();
		m_mFieldOfViewFades.Clear();
		m_mIconSeats.Clear();

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

		// With the vehicle outline shown, the radar shows even without a group
		bool outlineShown = m_VehicleOutline && m_VehicleOutline.IsShown();

		if (!m_bVisible || !localCharacter || (m_HUDSystem.GetLocalGroupCount() <= 1 && !outlineShown))
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

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> groupPlayerIDs = m_HUDSystem.GetLocalGroupPlayerIds();

		CSI_EntityHelper.SetSeatInfo(m_LocalSeat, CSI_EntityHelper.GetCharacterVehicleCompartment(localCharacter));

		// Center on the outline (on a mounted gun, its back)
		if (outlineShown)
			localOrigin = m_VehicleOutline.GetCenter();

		// Players outside the group riding in the vehicle show too
		bool passengersChanged = UpdatePassengers(outlineShown, groupPlayerIDs, localPlayerId, playerManager);

		// Add the local player and passengers when the group list lacks them
		array<int> radarPlayerIDs = groupPlayerIDs;
		if (outlineShown && (!groupPlayerIDs.Contains(localPlayerId) || !m_aPassengerPlayerIDs.IsEmpty()))
		{
			m_aShownPlayerIDs.Copy(groupPlayerIDs);
			if (!groupPlayerIDs.Contains(localPlayerId))
				m_aShownPlayerIDs.Insert(localPlayerId);

			m_aShownPlayerIDs.InsertAll(m_aPassengerPlayerIDs);
			radarPlayerIDs = m_aShownPlayerIDs;
		}

		float searchRadiusFadeStart = 0.8 * searchRadius;			// icons fade from about 2/3 of the range: 0.6 opacity here, 0 at the edge
		float radarScale = RADAR_SIZE / (searchRadius * SIZE_PER_RANGE);	// meters to radar px
		float halfIconSize = m_fIconSize * 0.5;
		float searchRadiusSq = searchRadius * searchRadius;
		float keepRadiusSq = searchRadiusSq * KEEP_RANGE_SHARE * KEEP_RANGE_SHARE;

		foreach (PlayerId playerID : radarPlayerIDs)
		{
			SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
			if (!playerCharacter)
			{
				RemoveRadarIcon(playerID);
				continue;
			};

			// The local player's icon is the center
			bool isLocalPlayer = playerID == localPlayerId;
			vector playerCharacterOrigin = localOrigin;
			if (!isLocalPlayer)
				playerCharacterOrigin = playerCharacter.GetOrigin();

			// Squared distance avoids a square root for out-of-range members
			float disSq = vector.DistanceSq(localOrigin, playerCharacterOrigin);

			// An icon is kept a little past the range, at 0 opacity, so a member on the edge doesn't rebuild it each crossing
			if (disSq > keepRadiusSq || (disSq > searchRadiusSq && !m_mRadarIcons.Get(playerID)))
			{
				RemoveRadarIcon(playerID);
				continue;
			};

			CSI_Icon icon = GetOrCreateRadarIcon(playerID);
			if (!icon)
				continue;

			bool inOutlinedVehicle = m_aOccupantPlayerIDs.Contains(playerID);
			bool onGunRear = false;
			CSI_SeatInfo seat = m_LocalSeat;
			if (isLocalPlayer)
			{
				onGunRear = outlineShown && m_VehicleOutline.IsCenterOnGun();
			}
			else
			{
				seat = GetIconSeat(playerID, playerCharacter);

				// On the outline, a mounted gun's gunner sits on the gun's back, as the local player does
				if (inOutlinedVehicle)
				{
					vector iconPosition;
					onGunRear = m_VehicleOutline.GetIconPosition(seat, playerCharacterOrigin, iconPosition);
					playerCharacterOrigin = iconPosition;
					disSq = vector.DistanceSq(localOrigin, playerCharacterOrigin);
				}
			}

			float dis = Math.Sqrt(disSq);

			// Defaults for the local player: center, fully shown
			float x = 0;
			float y = 0;
			float fieldOfViewFade = 1;

			if (!isLocalPlayer)
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

			// On a gun's back the icon faces where the player looks, as on foot
			float rotation;
			if (isLocalPlayer)
				rotation = GetLocalIconRotation(playerCharacter, localYaw, onGunRear, timeSlice);
			else if (onGunRear)
				rotation = -Math.Mod((CSI_EntityHelper.GetCharacterYaw(playerCharacter, null, null) - localYaw), 360);
			else
				rotation = -Math.Mod((GetSeatYaw(playerCharacter, seat) - localYaw), 360);

			icon.SetOutsider(m_aPassengerPlayerIDs.Contains(playerID));
			icon.SetInOutlinedVehicle(inOutlinedVehicle);

			Widget radarIcon = icon.GetRootWidget();

			// Center the icon on its position
			FrameSlot.SetPos(radarIcon, x - halfIconSize, y - halfIconSize);
			icon.SetRotation(rotation, m_bIconsUpright);
			radarIcon.SetOpacity(opacity);

			// The outline leaves its lines out around a gunner's or commander's icon and dims them under other seat icons
			if (inOutlinedVehicle && opacity > 0)
				m_VehicleOutline.AddIconGap(playerCharacterOrigin, seat.m_bGunnerSeat);
		};

		// The HUD system replaces the list when the group changes
		if (groupPlayerIDs == m_aCheckedGroupPlayerIDs && !passengersChanged)
			return;

		m_aCheckedGroupPlayerIDs = groupPlayerIDs;

		// Delete icons of players who left the group or the vehicle, or whose widget is gone
		m_aRadarIconsToRemove.Clear();
		foreach (PlayerId playerID, CSI_Icon icon : m_mRadarIcons)
		{
			if (!icon || !radarPlayerIDs.Contains(playerID))
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
	//! \param[in] playerID another member's
	//! \param[in] character theirs
	//! \return their seat info, kept by player ID while they have an icon
	protected CSI_SeatInfo GetIconSeat(PlayerId playerID, notnull SCR_ChimeraCharacter character)
	{
		CSI_SeatInfo seat = m_mIconSeats.Get(playerID);
		if (!seat)
		{
			seat = new CSI_SeatInfo();
			m_mIconSeats.Set(playerID, seat);
		}

		CSI_EntityHelper.SetSeatInfo(seat, CSI_EntityHelper.GetCharacterVehicleCompartment(character));
		return seat;
	}

	//------------------------------------------------------------------------------------------------
	//! Turret seats face their turret's aim (a sub-turret its parent's, a remote station the vehicle's); others as GetCharacterYaw.
	//! \param[in] character
	//! \param[in] seat the character's
	//! \return negated world yaw in degrees
	protected static float GetSeatYaw(notnull SCR_ChimeraCharacter character, notnull CSI_SeatInfo seat)
	{
		if (seat.m_ParentTurret)
			return -seat.m_ParentTurret.GetAimingDirectionWorld().ToYaw();

		if (CSI_EntityHelper.IsRemoteSeat(seat))
		{
			IEntity vehicle = seat.m_Compartment.GetVehicle();
			if (vehicle)
				return -vehicle.GetYawPitchRoll()[0];
		}

		return CSI_EntityHelper.GetCharacterYaw(character, seat.m_Compartment, seat.m_Turret);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aOccupantPlayerIDs and m_aPassengerPlayerIDs (riders outside the group with CSI data) from the outline's seats.
	//! \param[in] outlineShown false empties both lists
	//! \param[in] groupPlayerIDs
	//! \param[in] localPlayerId
	//! \param[in] playerManager
	//! \return true if m_aPassengerPlayerIDs changed
	protected bool UpdatePassengers(bool outlineShown, notnull array<int> groupPlayerIDs, int localPlayerId, notnull PlayerManager playerManager)
	{
		m_aNewPassengerPlayerIDs.Clear();
		m_aOccupantPlayerIDs.Clear();
		CSI_PlayerDataManager playerDataManager = CSI_PlayerDataManager.GetInstance();
		if (outlineShown && playerDataManager)
		{
			foreach (BaseCompartmentSlot seat : m_VehicleOutline.GetSeats())
			{
				if (!seat)
					continue;

				IEntity occupant = seat.GetOccupant();
				if (!occupant)
					continue;

				int playerID = playerManager.GetPlayerIdFromControlledEntity(occupant);
				if (playerID > 0)
					m_aOccupantPlayerIDs.Insert(playerID);

				if (playerID <= 0 || playerID == localPlayerId || groupPlayerIDs.Contains(playerID) || !playerDataManager.GetPlayerData(playerID))
					continue;

				if (!m_aNewPassengerPlayerIDs.Contains(playerID))
					m_aNewPassengerPlayerIDs.Insert(playerID);
			}
		}

		bool changed = m_aNewPassengerPlayerIDs.Count() != m_aPassengerPlayerIDs.Count();
		foreach (int playerID : m_aNewPassengerPlayerIDs)
		{
			if (!m_aPassengerPlayerIDs.Contains(playerID))
				changed = true;
		}

		if (changed)
			m_aPassengerPlayerIDs.Copy(m_aNewPassengerPlayerIDs);

		return changed;
	}

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
	//! the turret aim or vehicle yaw, as for other members, except on a gun's back: straight up, where the player looks.
	//! \param[in] localCharacter
	//! \param[in] localYaw the view's yaw, negated
	//! \param[in] onGunRear the icon is on the back of the player's mounted gun
	//! \param[in] timeSlice
	//! \return icon rotation in degrees
	protected float GetLocalIconRotation(notnull SCR_ChimeraCharacter localCharacter, float localYaw, bool onGunRear, float timeSlice)
	{
		if (m_LocalSeat.m_Compartment)
		{
			m_fFreeLookReturnTime = 0;
			if (onGunRear)
				return 0;

			return -Math.Mod((GetSeatYaw(localCharacter, m_LocalSeat) - localYaw), 360);
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
		m_mIconSeats.Remove(playerID);
		m_mRadarIcons.Remove(playerID);

		if (icon && icon.GetRootWidget())
			icon.GetRootWidget().RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearRadarIcons()
	{
		m_mFieldOfViewFades.Clear();
		m_mIconSeats.Clear();

		if (m_mRadarIcons.IsEmpty())
			return;

		foreach (PlayerId playerID, CSI_Icon icon : m_mRadarIcons)
		{
			if (icon && icon.GetRootWidget())
				icon.GetRootWidget().RemoveFromHierarchy();
		}

		m_mRadarIcons.Clear();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] range radar range in meters
	//! \return radar widths per meter at that range
	static float GetWidthsPerMeter(float range)
	{
		return 1 / (range * SIZE_PER_RANGE);
	}
}
