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
						switch (compartmentType)
						{
							case ECompartmentType.TURRET  : {m_MainTag = tagData; break;};
						};
					};
				};
			}
		};
		
		if (!m_MainTag)
			m_MainTag = m_aPassengers[0];
		
		if (m_MainTag.m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			ActivateEntityState(ENameTagEntityState.GROUP_MEMBER);
		else if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
			DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetName(out string name, out notnull array<string> nameParams)
	{
		UpdateMainTag();
		
		m_sName = string.Empty;
		m_aNameParams = {};
		
		if (m_MainTag)
		{			
			m_MainTag.GetName(m_sName, m_aNameParams);
			m_iPlayerID = m_MainTag.m_iPlayerID;

			if (m_sName == string.Empty)	// passenger tag might need entity update in case of lost connection 
			{
				m_aPassengers[0].UpdateEntityType();
				m_aPassengers[0].GetName(m_sName, m_aNameParams);
			}
			
			int count = m_aPassengers.Count();
			if (count > 1)
				m_sName = m_sName + "  (+" + (count - 1).ToString() + ")";
		}
			
		name = m_sName;
		nameParams.Copy(m_aNameParams);
	}
};
