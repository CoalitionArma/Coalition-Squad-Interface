//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-22
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Radar HUD element that outlines the player's vehicle, turrets and weapon racks and marks free
//! 				seats. Outlines come only from the server's CSI_VehicleOutlineSystem.
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutlineElement : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected static const string CANVAS_NAME = "VehicleOutline";
	protected static const float REQUEST_RETRY_TIME = 30;				//!< seconds between asks while the server hasn't sent every outline
	protected static const float ICON_GAP_SHARE = 0.52;				//!< gap left clear around a gunner's or commander's icon, in icon sizes
	protected static const float ICON_DIM_SHARE = 0.375;			//!< lines are dimmed under other seat icons out to their ring, in icon sizes
	protected static const float TAKEN_SEAT_DISTANCE = 0.3;			//!< m; empty seats this close to a taken one aren't marked

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;
	protected CSI_VehicleOutlineComponent m_Outline;		//!< draws on the element's canvas

	// This frame's state, read by the radar
	protected bool m_bShown;
	protected vector m_vCenter;
	protected bool m_bCenterOnGun;							//!< m_vCenter is the back of the local player's mounted gun
	protected vector m_vLocalCenter;						//!< m_vCenter in the vehicle's space
	protected float m_fSinTurn;								//!< of the outline's turn from the vehicle's heading to the view
	protected float m_fCosTurn = 1;
	protected ref array<float> m_aIconGaps = {};			//!< right, forward per icon the radar leaves a gap around, m; drawn next frame
	protected ref array<float> m_aIconDims = {};			//!< right, forward per icon the radar dims lines under, m; drawn next frame

	// The vehicle
	protected IEntity m_OutlineVehicle;						//!< vehicle the outline is for
	protected ref array<ref CSI_VehicleOutlinePart> m_aOutlineParts = {};	//!< its hull, then its turrets and weapon racks
	protected ref array<BaseCompartmentSlot> m_aOutlineSeats = {};			//!< every seat of the vehicle and its parts
	protected ref array<float> m_aSeatMarks = {};			//!< reused each update
	protected ref array<vector> m_aTakenSeatPositions = {};	//!< reused each update
	protected ref array<vector> m_aEmptySeatPositions = {};	//!< reused each update
	protected float m_fRequestRetryTime;					//!< seconds until the server is asked again for missing outlines

	// The local player's seat, updated when it changes
	protected ref CSI_SeatInfo m_LocalSeat = new CSI_SeatInfo();

	// Settings, read in ApplySettings
	protected bool m_bEnabled;				//!< RADAR_VISIBLE and RADAR_VEHICLE_OUTLINE
	protected int m_iRange;					//!< RADAR_RANGE_IN_VEHICLE in meters; 0 until the settings arrive
	protected float m_fIconSize;			//!< px, the radar icons' (RADAR_ICON_SIZE)

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();

		m_Outline = CSI_VehicleOutlineComponent.GetVehicleOutline(CANVAS_NAME, w);
		if (!m_Outline)
			PrintFormat("CSI_VehicleOutlineElement: canvas '%1' with CSI_VehicleOutlineComponent not found, vehicle outlines won't show", CANVAS_NAME, level: LogLevel.WARNING);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Called by CSI_HUD on a settings update.
	void ApplySettings()
	{
		if (!m_SettingsManager)
			return;

		m_bEnabled = m_SettingsManager.GetSettingBool(CSI_GameSettings.RADAR_VISIBLE) && m_SettingsManager.GetSettingBool(CSI_GameSettings.RADAR_VEHICLE_OUTLINE);
		m_iRange = m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_RANGE_IN_VEHICLE);
		m_fIconSize = CSI_Radar.ICON_WIDTH_AND_HEIGHT * (m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01);

		if (m_Outline)
			m_Outline.SetColor(CSI_ColorPresets.GetSettingColor(CSI_GameSettings.COMPASS_COLOR));
	}

	//------------------------------------------------------------------------------------------------
	//! Called every frame by CSI_HUD, before the radar.
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		m_bShown = false;
		m_bCenterOnGun = false;
		SCR_ChimeraCharacter localCharacter = m_HUDSystem.GetLocalPlayerCharacter();
		IEntity vehicle = m_HUDSystem.GetLocalVehicle();

		// m_iRange is 0 until the settings arrive
		if (!m_bEnabled || !m_Outline || !localCharacter || !vehicle || m_iRange <= 0)
		{
			HideVehicleOutline();
			return;
		}

		m_bShown = true;
		if (vehicle != m_OutlineVehicle)
			StartVehicleOutline(vehicle);

		CSI_EntityHelper.SetSeatInfo(m_LocalSeat, CSI_EntityHelper.GetCharacterVehicleCompartment(localCharacter));

		// Center on a mounted gun's rear so the vehicle doesn't swing around the radar as the gun turns
		vector center;
		m_bCenterOnGun = GetIconPosition(m_LocalSeat, localCharacter.GetOrigin(), center);
		m_vCenter = center;

		UpdateVehicleOutline(vehicle, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	bool IsShown()
	{
		return m_bShown;
	}

	//------------------------------------------------------------------------------------------------
	//! \return world position: the back of a mounted gun, else the local player
	vector GetCenter()
	{
		return m_vCenter;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the center is the back of the local player's mounted gun
	bool IsCenterOnGun()
	{
		return m_bCenterOnGun;
	}

	//------------------------------------------------------------------------------------------------
	//! \return seats of the vehicle and its parts, while the outline shows
	array<BaseCompartmentSlot> GetSeats()
	{
		return m_aOutlineSeats;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ICONS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] seat a character's in the outlined vehicle
	//! \param[in] origin the character's
	//! \param[out] position where their icon goes: the back of a one-seat mount's gun at the character's height, else origin
	//! \return true if it's the gun's back
	bool GetIconPosition(notnull CSI_SeatInfo seat, vector origin, out vector position)
	{
		position = origin;
		if (!seat.m_bOnlyMountSeat || !seat.m_Turret)
			return false;

		foreach (CSI_VehicleOutlinePart part : m_aOutlineParts)
		{
			if (part.m_Turret != seat.m_Turret || !part.m_Entity || !part.m_Outline || part.m_Outline.IsEmpty())
				continue;

			// A turret piece only if its gunner aims over the gun by hand, like the UAZ's PKM mount
			if (part.m_eKind == CSI_EVehicleOutlineKind.GUN || (part.m_eKind == CSI_EVehicleOutlineKind.TURRET && seat.m_bHeldGun))
			{
				position = GetGunRear(part);
				position[1] = origin[1];
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Leaves the outline's lines out around an icon, or dims them under it, from the next Update. Called by the radar.
	//! \param[in] position world position of the icon
	//! \param[in] cut leave lines out around it (a gunner's or commander's), else dim them under it
	void AddIconGap(vector position, bool cut)
	{
		if (!m_OutlineVehicle)
			return;

		array<float> gaps = m_aIconDims;
		if (cut)
			gaps = m_aIconGaps;

		vector offset = m_OutlineVehicle.CoordToLocal(position) - m_vLocalCenter;
		gaps.Insert(offset[0]);
		gaps.Insert(offset[2]);
	}

	//------------------------------------------------------------------------------------------------
	//! Where a point in the outlined vehicle lies on the radar: level in the vehicle's space, as the outline is drawn.
	//! \param[in] position world
	//! \param[out] right m right of the radar's center
	//! \param[out] down m below the radar's center
	void GetRadarOffset(vector position, out float right, out float down)
	{
		right = 0;
		down = 0;
		if (!m_OutlineVehicle)
			return;

		vector offset = m_OutlineVehicle.CoordToLocal(position) - m_vLocalCenter;
		right = offset[0] * m_fCosTurn - offset[2] * m_fSinTurn;
		down = -(offset[0] * m_fSinTurn + offset[2] * m_fCosTurn);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] gunPart a mounted gun or hand-aimed mount, with an outline
	//! \return world position of its outline's rear, on its forward axis
	protected vector GetGunRear(notnull CSI_VehicleOutlinePart gunPart)
	{
		vector frame[4];
		CSI_VehicleOutline.GetWorldFrame(gunPart.m_Entity, gunPart.m_Turret, gunPart.m_vFramePivot, frame);
		return frame[3] + frame[2] * gunPart.m_Outline.GetRearZ();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 OUTLINE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Places the outline's pieces and seat marks around m_vCenter.
	//! \param[in] vehicle
	//! \param[in] timeSlice
	protected void UpdateVehicleOutline(notnull IEntity vehicle, float timeSlice)
	{
		TakeServerOutlines(vehicle, timeSlice);

		// Everything goes in the vehicle's own space, level whatever its pitch and roll; the outline turns that to the
		// view by the vehicle's heading as a whole
		vector vehicleTransform[4];
		vehicle.GetWorldTransform(vehicleTransform);
		float viewYaw = -m_HUDSystem.GetLocalYaw() * Math.DEG2RAD;
		float baseYaw = Math.Atan2(vehicleTransform[2][0], vehicleTransform[2][2]);
		float turn = viewYaw - baseYaw;
		m_fSinTurn = Math.Sin(turn);
		m_fCosTurn = Math.Cos(turn);
		m_Outline.SetRotation(turn);
		m_vLocalCenter = vehicle.CoordToLocal(m_vCenter);

		foreach (int index, CSI_VehicleOutlinePart part : m_aOutlineParts)
		{
			if (!part.m_Entity || !part.m_Outline)
			{
				m_Outline.SetPiece(index, null, 1, 0, 0, 1, 0, 0, part.m_eKind);
				continue;
			}

			// The outline's frame: the entity's own, turned to a turret's aim, in the vehicle's space
			vector frame[4];
			CSI_VehicleOutline.GetWorldFrame(part.m_Entity, part.m_Turret, part.m_vFramePivot, frame);
			vector axisX = vehicle.VectorToLocal(frame[0]);
			vector axisZ = vehicle.VectorToLocal(frame[2]);
			vector offset = vehicle.CoordToLocal(frame[3]) - m_vLocalCenter;

			m_Outline.SetPiece(index, part.m_Outline, axisX[0], axisZ[0], axisX[2], axisZ[2], offset[0], offset[2], part.m_eKind);
		}

		UpdateSeatMarks(vehicle);

		// The radar placed its icons after the last Update
		float iconShare = m_fIconSize / CSI_Radar.RADAR_SIZE;
		m_Outline.SetIconGaps(m_aIconGaps, iconShare * ICON_GAP_SHARE, m_aIconDims, iconShare * ICON_DIM_SHARE);
		m_aIconGaps.Clear();
		m_aIconDims.Clear();

		m_Outline.Update(CSI_Radar.GetWidthsPerMeter(m_iRange));
	}

	//------------------------------------------------------------------------------------------------
	//! Takes the outlines the server sent, and asks again every REQUEST_RETRY_TIME while some are missing.
	//! \param[in] vehicle
	//! \param[in] timeSlice
	protected void TakeServerOutlines(notnull IEntity vehicle, float timeSlice)
	{
		bool missing;
		foreach (CSI_VehicleOutlinePart part : m_aOutlineParts)
		{
			// Only parts with a prefab are sent
			if (!part.m_Entity || part.m_sPrefab.IsEmpty() || part.TakeCached())
				continue;

			missing = true;
		}

		if (!missing)
			return;

		m_fRequestRetryTime -= timeSlice;
		if (m_fRequestRetryTime > 0)
			return;

		m_fRequestRetryTime = REQUEST_RETRY_TIME;
		CSI_RplToAuthorityManager rplToAuthority = CSI_RplToAuthorityManager.GetInstance();
		if (rplToAuthority)
			rplToAuthority.Owner_RequestVehicleOutlines(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	//! Marks empty seats, except those at a taken seat (a vehicle may list two seats in one place).
	//! \param[in] vehicle
	protected void UpdateSeatMarks(notnull IEntity vehicle)
	{
		m_aSeatMarks.Clear();
		m_aTakenSeatPositions.Clear();
		m_aEmptySeatPositions.Clear();

		foreach (BaseCompartmentSlot seat : m_aOutlineSeats)
		{
			if (!seat)
				continue;

			vector position = seat.GetPosition();
			if (position == vector.Zero)
				continue;

			if (seat.GetOccupant())
				m_aTakenSeatPositions.Insert(position);
			else
				m_aEmptySeatPositions.Insert(position);
		}

		float takenDistanceSq = TAKEN_SEAT_DISTANCE * TAKEN_SEAT_DISTANCE;
		foreach (vector position : m_aEmptySeatPositions)
		{
			bool nearTaken = false;
			foreach (vector takenPosition : m_aTakenSeatPositions)
			{
				if (vector.DistanceSq(position, takenPosition) < takenDistanceSq)
				{
					nearTaken = true;
					break;
				}
			}

			if (nearTaken)
				continue;

			vector offset = vehicle.CoordToLocal(position) - m_vLocalCenter;
			m_aSeatMarks.Insert(offset[0]);
			m_aSeatMarks.Insert(offset[2]);
		}

		m_Outline.SetSeats(m_aSeatMarks);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the vehicle's parts and seats; missing outlines are asked for at once (TakeServerOutlines).
	//! \param[in] vehicle
	protected void StartVehicleOutline(notnull IEntity vehicle)
	{
		m_OutlineVehicle = vehicle;
		CSI_VehicleOutline.GetOutlineParts(vehicle, m_aOutlineParts);
		m_Outline.SetPieceCount(m_aOutlineParts.Count());
		m_fRequestRetryTime = 0;
		m_aIconGaps.Clear();
		m_aIconDims.Clear();

		m_aOutlineSeats.Clear();
		CSI_VehicleOutline.GetSeats(vehicle, m_aOutlineSeats);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideVehicleOutline()
	{
		m_aIconGaps.Clear();
		m_aIconDims.Clear();

		// The vehicle may be deleted already
		if (m_aOutlineParts.IsEmpty())
			return;

		m_OutlineVehicle = null;
		m_aOutlineParts.Clear();
		m_aOutlineSeats.Clear();

		if (m_Outline)
			m_Outline.Clear();
	}
}
