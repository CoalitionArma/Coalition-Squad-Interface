//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-21
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Static helpers for a character's facing and vehicle seat, and CSI_SeatInfo.
//! 
//------------------------------------------------------------------------------------------------
class CSI_EntityHelper
{
	protected static const float SEAT_MATCH_DISTANCE = 0.02;	//!< m (about 1° of turn) a seat bone's local and model transforms may differ by and still match

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC HELPER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Fills the seat info for a compartment; returns early if it's unchanged.
	//! \param[in] seat
	//! \param[in] compartment null on foot
	//! \return true if the seat changed
	static bool SetSeatInfo(notnull CSI_SeatInfo seat, BaseCompartmentSlot compartment)
	{
		if (compartment && compartment == seat.m_Compartment)
			return false;

		seat.m_Compartment = compartment;
		seat.m_Turret = null;
		seat.m_ParentTurret = null;
		seat.m_bTurretSeat = false;
		seat.m_bRemoteCandidate = false;
		seat.m_SeatBone = -1;
		if (!compartment)
			return true;

		TurretControllerComponent turretController = GetSeatTurretController(compartment);
		if (!turretController)
			return true;

		seat.m_bTurretSeat = true;
		seat.m_Turret = turretController.GetTurretComponent();
		if (!seat.m_Turret)
			return true;

		if (compartment.GetOwner())
			seat.m_ParentTurret = GetParentTurret(compartment.GetOwner());

		// Only a seat in a bone-turned turret's own prefab can be a remote station's
		IEntity turretEntity = seat.m_Turret.GetOwner();
		if (!turretEntity || compartment.GetOwner() != turretEntity || seat.m_Turret.HasRootRotation())
			return true;

		seat.m_bRemoteCandidate = true;
		Animation animation = turretEntity.GetAnimation();
		EntitySlotInfo seatPoint = compartment.GetPassengerPointInfo();
		if (animation && seatPoint)
			seat.m_SeatBone = animation.GetBoneIndex(seatPoint.GetBoneName());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return turret of the nearest parent that has one, null if none
	static TurretComponent GetParentTurret(notnull IEntity entity)
	{
		IEntity parent = entity.GetParent();
		while (parent)
		{
			TurretComponent turret = TurretComponent.Cast(parent.FindComponent(TurretComponent));
			if (turret)
				return turret;

			parent = parent.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A remote station's seat stays put while the turret turns: it has no bone, or its local and model transforms match.
	//! \param[in] seat
	//! \return true for a remote weapon station's seat
	static bool IsRemoteSeat(notnull CSI_SeatInfo seat)
	{
		if (!seat.m_bRemoteCandidate || !seat.m_Turret)
			return false;

		IEntity turretEntity = seat.m_Turret.GetOwner();
		if (!turretEntity)
			return false;

		if (seat.m_SeatBone == -1)
			return true;

		Animation animation = turretEntity.GetAnimation();
		vector modelMatrix[4];
		vector localMatrix[4];
		if (!animation || !animation.GetBoneMatrix(seat.m_SeatBone, modelMatrix) || !animation.GetBoneLocalMatrix(seat.m_SeatBone, localMatrix))
			return true;

		float matchSq = SEAT_MATCH_DISTANCE * SEAT_MATCH_DISTANCE;
		return vector.DistanceSq(modelMatrix[3], localMatrix[3]) < matchSq && vector.DistanceSq(modelMatrix[2], localMatrix[2]) < matchSq;
	}

	//------------------------------------------------------------------------------------------------
	//! Head aim on foot, turret aim in a turret, vehicle yaw in other seats.
	//! \param[in] playerCharacter
	//! \param[in] compartment the character's seat, null on foot
	//! \param[in] turret the turret the seat aims, null for other seats
	//! \return negated world yaw in degrees, 0 if it can't be read
	static float GetCharacterYaw(SCR_ChimeraCharacter playerCharacter, BaseCompartmentSlot compartment, TurretComponent turret)
	{
		if (!playerCharacter)
			return 0;

		AimingComponent playerAimingComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerAimingComponent)
			return 0;

		if (!compartment)
			return -playerAimingComponent.GetAimingDirectionWorld().ToYaw();

		if (turret)
			return -turret.GetAimingDirectionWorld().ToYaw();

		// Vehicle yaw even for seats facing another way, to match the outline
		IEntity vehicle = compartment.GetVehicle();
		if (vehicle)
			return -vehicle.GetYawPitchRoll()[0];

		return -playerCharacter.GetYawPitchRoll()[0];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartment
	//! \return the controller of the turret or mounted gun the seat aims, null for other seats
	static TurretControllerComponent GetSeatTurretController(notnull BaseCompartmentSlot compartment)
	{
		TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
		if (!turretController)
			turretController = compartment.GetAttachedTurret();

		return turretController;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartment
	//! \return the turret the seat aims, null for other seats
	static TurretComponent GetSeatTurret(notnull BaseCompartmentSlot compartment)
	{
		TurretControllerComponent turretController = GetSeatTurretController(compartment);
		if (!turretController)
			return null;

		return turretController.GetTurretComponent();
	}

	//------------------------------------------------------------------------------------------------
	static BaseCompartmentSlot GetCharacterVehicleCompartment(IEntity playerCharacter)
	{
		if (!playerCharacter)
			return null;

		CompartmentAccessComponent compartmentAccess;
		ChimeraCharacter character = ChimeraCharacter.Cast(playerCharacter);
		if (character)
			compartmentAccess = character.GetCompartmentAccessComponent();
		else
			compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));

		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			return compartment;
		};
		return null;
	};
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-21
//! \Source: 		Coalition Squad Interface
//!	\Description: 	A character's seat as the radar and vehicle outline need it, filled by CSI_EntityHelper.SetSeatInfo.
//!
//------------------------------------------------------------------------------------------------
class CSI_SeatInfo
{
	BaseCompartmentSlot m_Compartment;	//!< null on foot
	TurretComponent m_Turret;			//!< the turret the seat aims, null for other seats
	TurretComponent m_ParentTurret;		//!< turret that the seat's turret rides on, null if none
	bool m_bTurretSeat;					//!< seat aims a turret or mounted gun
	bool m_bRemoteCandidate;			//!< may be a remote station's seat (IsRemoteSeat decides)
	TNodeId m_SeatBone = -1;			//!< the passenger point's bone in the turret, -1 without one
}
