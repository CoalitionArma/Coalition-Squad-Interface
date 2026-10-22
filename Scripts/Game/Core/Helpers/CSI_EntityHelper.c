class CSI_EntityHelper
{	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC HELPER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Get characters yaw axis value
	//! \param[in] playerCharacter: The inputed players character entity
	//! \return The yaw of the character entity
	static float GetCharacterYaw(SCR_ChimeraCharacter playerCharacter)
	{
		if (!playerCharacter)
			return 0;
		
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
	//! Get characters current vehicle slot
	//! \param[in] playerCharacter: The inputed players character entity
	//! \return The compartment the entity is in (will return null if they aren't in a vehicle)
	static BaseCompartmentSlot GetCharacterVehicleCompartment(IEntity playerCharacter)
	{
		if (!playerCharacter)
			return null;
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess) 
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			return compartment;
		};
		return null;
	};
	
	//------------------------------------------------------------------------------------------------
	//! Returns IlluminationFactor from PerceivableComponent - we use this to "dim" nametags depending on the ambient light arround the entity
	//! \param[in] entity: The enttiy to check
	//! \return The float value of the light arround the entity
	static float GetEntityIlluminationFactor(notnull IEntity entity)
	{
		// Common case first
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (character && character.m_pPerceivableComponent)
		{
			return character.m_pPerceivableComponent.GetIlluminationFactor();
		}
		
		Vehicle vehicle = Vehicle.Cast(entity);
		if (vehicle && vehicle.m_pPerceivableComponent)
		{
			return vehicle.m_pPerceivableComponent.GetIlluminationFactor();
		}
		
		// Worst case - some other entity, perform component search
		PerceivableComponent perceivableComponent = PerceivableComponent.Cast(entity.FindComponent(PerceivableComponent));
		if (perceivableComponent)
			return perceivableComponent.GetIlluminationFactor();
		
		return 1;
	}
}