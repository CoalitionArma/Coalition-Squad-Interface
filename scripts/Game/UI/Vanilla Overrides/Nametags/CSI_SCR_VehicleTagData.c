modded class SCR_VehicleTagData
{
	override protected void UpdateMainTag()
	{
		m_MainTag = null;
		
		if (!m_Entity)
			return;
		
		if (m_aPassengers.IsEmpty())
			return;
		
		IEntity pilot = Vehicle.Cast(m_Entity).GetPilot();
		if (pilot)
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
			{
				if (tagData.m_Entity == pilot)
				{
					m_MainTag = tagData;
					break;
				}
			}
		}
		
		if (!m_MainTag) 
		{
			foreach (SCR_NameTagData tagData : m_aPassengers)
			{
				if (m_MainTag) break;
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
						{
							m_MainTag = tagData;
							break;
						}
					};
				};
			}
		};
		
		if (!m_MainTag)
			m_MainTag = m_aPassengers[0];
		
		m_iPlayerID = m_MainTag.m_iPlayerID;
		
		if (m_MainTag.m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			ActivateEntityState(ENameTagEntityState.GROUP_MEMBER);
		else if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);
	}
};
