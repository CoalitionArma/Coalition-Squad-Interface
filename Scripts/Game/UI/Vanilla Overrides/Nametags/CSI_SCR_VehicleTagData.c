modded class SCR_VehicleTagData
{
	//------------------------------------------------------------------------------------------------
	override protected void UpdateMainTag()
	{
		m_MainTag = null;
		SCR_NameTagData localMainTag;
		
		if (!m_Entity)
			return;
		
		if (m_aPassengers.IsEmpty())
			return;
		
		IEntity pilot = Vehicle.Cast(m_Entity).GetPilot();
		if (pilot)
		{
			VehicleHelicopterSimulation heloSim = VehicleHelicopterSimulation.Cast(m_Entity.FindComponent(VehicleHelicopterSimulation));
			VehicleFixedWingSimulation planeSim = VehicleFixedWingSimulation.Cast(m_Entity.FindComponent(VehicleFixedWingSimulation));

			foreach (SCR_NameTagData tagData : m_aPassengers)
			{
				if (tagData.m_Entity == pilot && (heloSim || planeSim))
					localMainTag = tagData;
			}
		}
		
		if (!localMainTag) 
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
			{
				if (localMainTag) break;
				// Check if player is in a vehicle.
				CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(tagData.m_Entity.FindComponent(CompartmentAccessComponent));
				if (compartmentAccess)
				{
					// Check players current compartment.
					BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
					if (compartment)
					{
						// Check players current compartment type.
						ECompartmentType compartmentType = compartment.GetType();
						
						if (compartmentType == ECompartmentType.TURRET)
							localMainTag = tagData;
					};
				};
			}
		};
		
		if (pilot && !localMainTag)
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
				if (tagData.m_Entity == pilot)
					localMainTag = tagData;
		}
		
		if (!localMainTag)
			localMainTag = m_aPassengers[0];
		
		m_MainTag = localMainTag;
		
		m_iPlayerID = m_MainTag.m_iPlayerID;
		m_PlayerData = m_MainTag.m_PlayerData;
		
		if (m_MainTag.m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			ActivateEntityState(ENameTagEntityState.GROUP_MEMBER);
		else if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);
		
		GetGroupName();
	}
};
