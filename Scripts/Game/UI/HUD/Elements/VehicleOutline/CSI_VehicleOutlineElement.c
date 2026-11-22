//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-21
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Radar HUD element that outlines the player's vehicle, turrets and weapon racks and marks free
//! 				seats. Outlines come from CSI_VehicleOutlineSystem, or are built here if they don't arrive in time.
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutlineElement : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected static const string CANVAS_NAME = "VehicleOutline";
	protected static const int OUTLINE_TRACES_PER_FRAME = 64;			//!< traces per frame for a local build
	protected static const float SERVER_OUTLINE_WAIT = 15;				//!< seconds to wait for the server's outlines before building them here
	protected static const float CENTER_GAP_ICON_SHARE = 0.45;		//!< dimmed gap around the center, in icon sizes
	protected static const float CENTER_GAP_CUT_ICON_SHARE = 0.52;	//!< gap left clear for a gunner, in icon sizes
	protected static const int MAX_TURRET_SEATS = 1;				//!< most seats a gun mount may have to be centered on
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

	// The vehicle
	protected IEntity m_OutlineVehicle;						//!< vehicle the outline is for
	protected ref array<ref CSI_VehicleOutlinePart> m_aOutlineParts = {};	//!< its hull, then its turrets and weapon racks
	protected ref array<BaseCompartmentSlot> m_aOutlineSeats = {};			//!< every seat of the vehicle and its parts
	protected ref array<float> m_aSeatMarks = {};			//!< reused each update
	protected ref array<vector> m_aTakenSeatPositions = {};	//!< reused each update
	protected ref array<vector> m_aEmptySeatPositions = {};	//!< reused each update
	protected CSI_VehicleOutlineSystem m_OutlineSystem;		//!< null unless the server runs here
	protected float m_fOutlineWaitTime;						//!< seconds since the server was asked for the outlines

	// The local player's seat, updated when it changes
	protected ref CSI_SeatInfo m_LocalSeat = new CSI_SeatInfo();
	protected bool m_bLocalSeatCenters;						//!< only seat of a mount not on a turret
	protected bool m_bLocalHeldGun;							//!< its gunner can aim without a sight

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

		if (CSI_EntityHelper.SetSeatInfo(m_LocalSeat, CSI_EntityHelper.GetCharacterVehicleCompartment(localCharacter)))
			UpdateLocalSeatCenter();

		// Center on a mounted gun's rear so the vehicle doesn't swing around the radar as the gun turns
		m_vCenter = GetTurretSeatCenter(localCharacter.GetOrigin());

		// Lines over the player's icon: cut for a gunner, dimmed in other seats
		bool inTurretSeat = m_LocalSeat.m_bTurretSeat;
		float gapShare = CENTER_GAP_ICON_SHARE;
		if (inTurretSeat)
			gapShare = CENTER_GAP_CUT_ICON_SHARE;

		m_Outline.SetCenterGap(m_fIconSize * gapShare / CSI_Radar.RADAR_SIZE);
		m_Outline.SetCenterGapCut(inTurretSeat);

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
	//! \return seats of the vehicle and its parts, while the outline shows
	array<BaseCompartmentSlot> GetSeats()
	{
		return m_aOutlineSeats;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 CENTER
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Works out whether the local player's seat centers the outline on its gun.
	protected void UpdateLocalSeatCenter()
	{
		BaseCompartmentSlot compartment = m_LocalSeat.m_Compartment;

		// Only a one-seat mount that doesn't ride a turret
		m_bLocalSeatCenters = compartment && GetSeatCount(compartment) <= MAX_TURRET_SEATS && !CSI_VehicleOutline.IsOnTurret(compartment.GetOwner());

		// A turret piece only if its gunner can aim without a sight, like the UAZ's PKM mount
		m_bLocalHeldGun = false;
		if (m_bLocalSeatCenters)
		{
			TurretControllerComponent turretController = CSI_EntityHelper.GetSeatTurretController(compartment);
			m_bLocalHeldGun = turretController && !turretController.GetCanAimOnlyInADS();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] localOrigin the character's origin
	//! \return back of the seat's mounted gun at the character's height, else localOrigin
	protected vector GetTurretSeatCenter(vector localOrigin)
	{
		TurretComponent seatTurret = m_LocalSeat.m_Turret;
		if (!m_bLocalSeatCenters || !seatTurret)
			return localOrigin;

		foreach (CSI_VehicleOutlinePart part : m_aOutlineParts)
		{
			if (part.m_Turret != seatTurret || !part.m_Entity || !part.m_Outline || part.m_Outline.IsEmpty())
				continue;

			if (part.m_eKind == CSI_EVehicleOutlineKind.GUN || (part.m_eKind == CSI_EVehicleOutlineKind.TURRET && m_bLocalHeldGun))
			{
				vector gunCenter = GetGunRear(part);
				gunCenter[1] = localOrigin[1];
				return gunCenter;
			}
		}

		return localOrigin;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartment
	//! \return seats of the turret, mount or vehicle part the compartment belongs to
	protected int GetSeatCount(notnull BaseCompartmentSlot compartment)
	{
		BaseCompartmentManagerComponent compartmentManager = compartment.GetManager();
		if (!compartmentManager)
			return 1;

		array<BaseCompartmentSlot> seats = {};
		return compartmentManager.GetCompartments(seats);
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
		m_fOutlineWaitTime += timeSlice;
		UpdateOutlineBuilds(vehicle, timeSlice);

		// Pieces go in the vehicle's ground frame; the outline turns that to the view as a whole
		vector vehicleTransform[4];
		vehicle.GetWorldTransform(vehicleTransform);
		float viewYaw = -m_HUDSystem.GetLocalYaw() * Math.DEG2RAD;
		float baseYaw = Math.Atan2(vehicleTransform[2][0], vehicleTransform[2][2]);
		float sinBase = Math.Sin(baseYaw);
		float cosBase = Math.Cos(baseYaw);
		m_Outline.SetRotation(viewYaw - baseYaw);

		foreach (int index, CSI_VehicleOutlinePart part : m_aOutlineParts)
		{
			if (!part.m_Entity || !part.m_Outline)
			{
				m_Outline.SetPiece(index, null, 1, 0, 0, 1, 0, 0, part.m_eKind);
				continue;
			}

			// The outline's frame on the ground: the entity's own, turned to a turret's aim
			vector frame[4];
			CSI_VehicleOutline.GetWorldFrame(part.m_Entity, part.m_Turret, part.m_vFramePivot, frame);
			float offsetX = frame[3][0] - m_vCenter[0];
			float offsetZ = frame[3][2] - m_vCenter[2];

			m_Outline.SetPiece(index, part.m_Outline,
				frame[0][0] * cosBase - frame[0][2] * sinBase,
				frame[2][0] * cosBase - frame[2][2] * sinBase,
				frame[0][0] * sinBase + frame[0][2] * cosBase,
				frame[2][0] * sinBase + frame[2][2] * cosBase,
				offsetX * cosBase - offsetZ * sinBase,
				offsetX * sinBase + offsetZ * cosBase,
				part.m_eKind);
		}

		UpdateSeatMarks(sinBase, cosBase);
		m_Outline.Update(CSI_Radar.GetWidthsPerMeter(m_iRange));
	}

	//------------------------------------------------------------------------------------------------
	//! Picks up the server's outlines and builds here those it didn't send in time.
	//! \param[in] vehicle
	//! \param[in] timeSlice
	protected void UpdateOutlineBuilds(notnull IEntity vehicle, float timeSlice)
	{
		bool traced;
		int serverBuilding = -1;	// -1 until checked, then 0 or 1
		foreach (CSI_VehicleOutlinePart part : m_aOutlineParts)
		{
			if (!part.m_Entity || part.TakeCached())
				continue;

			if (!part.m_Builder)
			{
				// Wait while the server may still send it or is building it here
				if (serverBuilding == -1)
				{
					serverBuilding = 0;
					if (m_fOutlineWaitTime < SERVER_OUTLINE_WAIT || (m_OutlineSystem && m_OutlineSystem.IsOutlining(vehicle)))
						serverBuilding = 1;
				}

				if (serverBuilding == 1)
					continue;

				part.StartBuilding();
			}

			if (part.StepBuild(timeSlice, traced, OUTLINE_TRACES_PER_FRAME))
				part.FinishBuild();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Marks empty seats, except those at a taken seat (a vehicle may list two seats in one place).
	//! \param[in] sinBase sine of the vehicle's yaw
	//! \param[in] cosBase cosine of the vehicle's yaw
	protected void UpdateSeatMarks(float sinBase, float cosBase)
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

			float offsetX = position[0] - m_vCenter[0];
			float offsetZ = position[2] - m_vCenter[2];
			m_aSeatMarks.Insert(offsetX * cosBase - offsetZ * sinBase);
			m_aSeatMarks.Insert(offsetX * sinBase + offsetZ * cosBase);
		}

		m_Outline.SetSeats(m_aSeatMarks);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the vehicle's parts and seats and asks the server for missing outlines, or builds them here if it can't ask.
	//! \param[in] vehicle
	protected void StartVehicleOutline(notnull IEntity vehicle)
	{
		m_OutlineVehicle = vehicle;
		CSI_VehicleOutline.GetOutlineParts(vehicle, m_aOutlineParts, false);
		m_Outline.SetPieceCount(m_aOutlineParts.Count());
		m_fOutlineWaitTime = 0;
		m_OutlineSystem = CSI_VehicleOutlineSystem.GetInstance();

		m_aOutlineSeats.Clear();
		CSI_VehicleOutline.GetSeats(vehicle, m_aOutlineSeats);

		foreach (CSI_VehicleOutlinePart part : m_aOutlineParts)
		{
			if (part.m_Outline)
				continue;

			CSI_RplToAuthorityManager rplToAuthority = CSI_RplToAuthorityManager.GetInstance();
			if (!rplToAuthority || !rplToAuthority.Owner_RequestVehicleOutlines(vehicle))
				m_fOutlineWaitTime = SERVER_OUTLINE_WAIT;

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HideVehicleOutline()
	{
		// The vehicle may be deleted already
		if (m_aOutlineParts.IsEmpty())
			return;

		m_OutlineVehicle = null;
		m_OutlineSystem = null;
		m_aOutlineParts.Clear();
		m_aOutlineSeats.Clear();

		if (m_Outline)
			m_Outline.Clear();
	}
}
