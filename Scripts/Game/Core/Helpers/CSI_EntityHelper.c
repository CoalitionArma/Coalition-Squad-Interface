//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
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
	//! Yaw the character is facing: head aim on foot, turret aim in a turret, vehicle yaw in other seats.
	//! \param[in] playerCharacter
	//! \return negated world yaw in degrees, 0 if it can't be read
	static float GetCharacterYaw(SCR_ChimeraCharacter playerCharacter)
	{
		if (!playerCharacter)
			return 0;

		AimingComponent playerAimingComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerAimingComponent)
			return 0;

		float yaw = -playerAimingComponent.GetAimingDirectionWorld().ToYaw();

		BaseCompartmentSlot compartment = GetCharacterVehicleCompartment(playerCharacter);
		if (compartment)
		{
			TurretControllerComponent turretControllerComp = TurretControllerComponent.Cast(compartment.GetController());
			if (!turretControllerComp)
				turretControllerComp = compartment.GetAttachedTurret();

			if (turretControllerComp)
				yaw = -turretControllerComp.GetTurretComponent().GetAimingDirectionWorld().ToYaw();
			else
				yaw = -playerCharacter.GetYawPitchRoll()[0];
		};

		return yaw;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerCharacter
	//! \return the vehicle compartment the entity is in, null if it isn't in a vehicle
	static BaseCompartmentSlot GetCharacterVehicleCompartment(IEntity playerCharacter)
	{
		if (!playerCharacter)
			return null;

		// Characters expose their compartment access directly (as vanilla uses it), avoiding a component search;
		// fall back to FindComponent for any other entity type
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