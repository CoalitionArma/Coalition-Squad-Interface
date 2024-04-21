//------------------------------------------------------------------------------------------------
// Ruleset for dosplay of all frindlies
[BaseContainerProps()]
modded class SCR_NameTagRulesetFriendlies : SCR_NameTagRulesetBase
{
	//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	override protected bool TraceLOS(SCR_NameTagData data)
	{
		if (data.m_eType == ENameTagEntityType.VEHICLE || IsPlayerInVehicle(SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity())))
			return true;
		
		TraceParam param = new TraceParam;
		param.Start = m_CurrentPlayerTag.m_vEntHeadPos;
		param.End = data.m_vEntHeadPos + HEAD_LOS_OFFSET;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS; 
		array<IEntity> excludes = { m_CurrentPlayerTag.m_Entity, data.m_Entity };
		param.ExcludeArray = excludes;
			
		float percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)	// If trace travels the entire path, return true
			return true;
				
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsPlayerInVehicle(SCR_ChimeraCharacter playerCharacter)
	{
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess) 
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment) return true;
		};
		return false;
	};
};
