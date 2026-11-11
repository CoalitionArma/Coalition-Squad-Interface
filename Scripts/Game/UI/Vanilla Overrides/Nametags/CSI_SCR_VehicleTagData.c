//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the vehicle tag shows the pilot in aircraft and prefers a turret gunner in other vehicles, and
//! 				copies the main tag's player data and group name. UpdateMainTag is a copy of vanilla, with CSI's lines marked "CSI:".
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_VehicleTagData
{
	//------------------------------------------------------------------------------------------------
	//! Update main tag. CSI order: aircraft pilot, else a turret occupant, else the driver, else the first passenger.
	override protected void UpdateMainTag()
	{
		m_MainTag = null;
		SCR_NameTagData localMainTag;

		if (!m_Entity)
			return;

		if (m_aPassengers.IsEmpty())
			return;

		// CSI: the pilot only wins outright in helicopters and planes
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

		// CSI: otherwise the first passenger in a turret
		if (!localMainTag)
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
			{
				if (localMainTag) break;
				CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(tagData.m_Entity.FindComponent(CompartmentAccessComponent));
				if (compartmentAccess)
				{
					BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
					if (compartment)
					{
						ECompartmentType compartmentType = compartment.GetType();

						if (compartmentType == ECompartmentType.TURRET)
							localMainTag = tagData;
					};
				};
			}
		};

		// Then the driver, like vanilla
		if (pilot && !localMainTag)
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
				if (tagData.m_Entity == pilot)
					localMainTag = tagData;
		}

		if (!localMainTag)
			localMainTag = m_aPassengers[0];

		m_MainTag = localMainTag;

		// CSI: the vehicle tag uses the main tag's player for CSI's elements
		m_iPlayerID = m_MainTag.m_iPlayerID;
		m_PlayerData = m_MainTag.m_PlayerData;

		if (m_MainTag.m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			ActivateEntityState(ENameTagEntityState.GROUP_MEMBER);
		else if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);

		GetGroupName();	// CSI
	}
};
