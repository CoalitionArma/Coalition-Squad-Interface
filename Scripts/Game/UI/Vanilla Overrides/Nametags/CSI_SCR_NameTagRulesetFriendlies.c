[BaseContainerProps()]
modded class SCR_NameTagRulesetFriendlies : SCR_NameTagRulesetBase
{
	protected CSI_SettingsSystem m_SettingsSystem;
	
	//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param[in] data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	override protected bool TraceLOS(SCR_NameTagData data)
	{
		if (!m_SettingsSystem)
			m_SettingsSystem = CSI_SettingsSystem.GetInstance();
		
		if (!m_SettingsSystem.GetSettingBool(CSI_GameSettings.NAMETAG_LOS_VISIBLE))
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
};
