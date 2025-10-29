//------------------------------------------------------------------------------------------------
// Ruleset for display of all frindlies
[BaseContainerProps()]
modded class SCR_NameTagRulesetFriendlies : SCR_NameTagRulesetBase
{	
	//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	override protected bool TraceLOS(SCR_NameTagData data)
	{
		if (data.m_eType == ENameTagEntityType.VEHICLE || CSI_ClientComponent.GetInstance().ReturnLocalCSISettings()[10] == "false" || IsPlayerInVehicle(SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity())))
			return true;
		
		TraceParam param = new TraceParam;
		param.Start = m_CameraPosition;
		param.End = data.m_vEntHeadPos + HEAD_LOS_OFFSET;
		param.Flags = TraceFlags.ANY_CONTACT | TraceFlags.WORLD | TraceFlags.ENTS; 
		IEntity targetEntity = data.m_Entity;
		
		if (data.GetVehicleCompartment())
			targetEntity = data.GetVehicleCompartment().GetVehicle();
		
		array<IEntity> ExcludeArray = {};
		ExcludeArray.Insert(targetEntity);
		ExcludeArray.Insert(m_CurrentPlayerTag.m_Entity);
		param.ExcludeArray = ExcludeArray;
		
		param.LayerMask = TRACE_LAYER_CAMERA;
		float percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)
			return true;
		
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)
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
			if (compartment) 
				return true;
		};
		return false;
	};
};
