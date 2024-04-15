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
};
