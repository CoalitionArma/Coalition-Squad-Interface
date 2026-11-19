//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-19
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Static helpers for a character's facing and vehicle seat.
//! 
//------------------------------------------------------------------------------------------------
class CSI_EntityHelper
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC HELPER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Head aim on foot, turret aim in a turret, vehicle yaw in other seats.
	//! \param[in] playerCharacter
	//! \return negated world yaw in degrees, 0 if it can't be read
	static float GetCharacterYaw(SCR_ChimeraCharacter playerCharacter)
	{
		if (!playerCharacter)
			return 0;

		AimingComponent playerAimingComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerAimingComponent)
			return 0;

		// Called every frame for each radar icon: the seat is checked first, so only one aim is read
		BaseCompartmentSlot compartment;
		CompartmentAccessComponent compartmentAccess = playerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess)
			compartment = compartmentAccess.GetCompartment();

		if (!compartment)
			return -playerAimingComponent.GetAimingDirectionWorld().ToYaw();

		TurretControllerComponent turretControllerComp = TurretControllerComponent.Cast(compartment.GetController());
		if (!turretControllerComp)
			turretControllerComp = compartment.GetAttachedTurret();

		if (turretControllerComp)
			return -turretControllerComp.GetTurretComponent().GetAimingDirectionWorld().ToYaw();

		return -playerCharacter.GetYawPitchRoll()[0];
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