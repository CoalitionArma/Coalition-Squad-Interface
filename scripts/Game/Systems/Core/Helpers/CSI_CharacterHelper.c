class CSI_ChararcterHelper
{	
	//------------------------------------------------------------------------------------------------
	/**
	* Get characters yaw axis value
	* @param playerCharacter The inputed players character entity
	* @return The yaw of the character entity
	*/
	static float GetCharacterYaw(SCR_ChimeraCharacter playerCharacter)
	{
		AimingComponent playerControllerComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) 
			return 0;

		float yaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();

		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
				yaw = -playerCharacter.GetYawPitchRoll()[0];
		}
		return yaw;
	}

	//------------------------------------------------------------------------------------------------
	/**
	* Get characters current vehicle slot
	* @param playerCharacter The inputed players character entity
	* @return the compartment the entity is in (will return null if they aren't in a vehicle)
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