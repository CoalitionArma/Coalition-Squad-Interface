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
		};
		return yaw;
	}

	//------------------------------------------------------------------------------------------------
	/**
	* Get characters yaw axis value
	* @param playerCharacter The inputed players character entity
	* @return The yaw of the character entity
	*/
	static float GetLocalAimingYaw()
	{
		SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());
		
		// Freelook Direction
        float yaw;

		AimingComponent playerControllerComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) 
			return yaw;

		if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(playerCharacter) || playerControllerComponent.GetAimingDirection().ToYaw() > 0)
		{
			vector transform[4];
			GetGame().GetWorld().GetCurrentCamera(transform);

			yaw = -Math3D.MatrixToAngles(transform)[0];
		} else
			yaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();

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