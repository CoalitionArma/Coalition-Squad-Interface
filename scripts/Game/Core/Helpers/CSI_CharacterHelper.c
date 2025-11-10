class CSI_ChararcterHelper
{	
	//------------------------------------------------------------------------------------------------
	/**
	* Get characters yaw axis value
	* @param playerCharacter: The inputed players character entity
	* @return The yaw of the character entity
	*/
	static float GetCharacterYaw(SCR_ChimeraCharacter playerCharacter)
	{
		AimingComponent playerAimingComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerAimingComponent) 
			return 0;

		float yaw = playerAimingComponent.GetAimingDirectionWorld().ToYaw();

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
	/**
	* Get characters current vehicle slot
	* @param playerCharacterL The inputed players character entity
	* @return The compartment the entity is in (will return null if they aren't in a vehicle)
	*/
	static BaseCompartmentSlot GetCharacterVehicleCompartment(IEntity playerCharacter)
	{
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess) 
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			return compartment;
		};
		return null;
	};
}