//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the vehicle tag shows the pilot in aircraft and prefers a turret gunner in other vehicles, and
//! 				copies the main tag's player data and group name.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_VehicleTagData
{
	//------------------------------------------------------------------------------------------------
	//! Main tag order: aircraft pilot, turret occupant, driver, first passenger.
	override protected void UpdateMainTag()
	{
		m_MainTag = null;
		SCR_NameTagData localMainTag;	// CSI: chosen here, then set as m_MainTag

		if (!m_Entity)
			return;

		if (m_aPassengers.IsEmpty())
			return;

		// CSI: pilot first in helicopters and planes
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

		// CSI: then the first turret occupant
		if (!localMainTag)
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
			{
				if (localMainTag) break;
				BaseCompartmentSlot compartment = tagData.GetVehicleCompartment();
				if (compartment && compartment.GetType() == ECompartmentType.TURRET)
					localMainTag = tagData;
			}
		};

		// Then the driver (CSI: no break, only one passenger is the pilot)
		if (pilot && !localMainTag)
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
				if (tagData.m_Entity == pilot)
					localMainTag = tagData;
		}

		if (!localMainTag)
			localMainTag = m_aPassengers[0];

		m_MainTag = localMainTag;

		// CSI: main tag's player for CSI's elements
		m_iPlayerID = m_MainTag.m_iPlayerID;
		m_PlayerData = m_MainTag.m_PlayerData;

		if (m_MainTag.m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			ActivateEntityState(ENameTagEntityState.GROUP_MEMBER);
		else if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);

		GetGroupName();	// CSI
	}
};
