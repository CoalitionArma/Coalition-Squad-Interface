enum ECOATagEntityState: ENameTagEntityState
{
	RED_TEAM		= 1<<7,
	BLUE_TEAM		= 1<<8,
	GREEN_TEAM		= 1<<9,
	YELLOW_TEAM		= 1<<10
};

modded class SCR_NameTagData
{
	ECOATagEntityState m_eEntityStateFlags;
	SCR_GroupPreset gp;
	string m_sGroupName;
	
	override void InitDefaults()
	{
		super.InitDefaults();
		// Set tags to be over the body by default
		m_eAttachedTo = ENameTagPosition.BODY;
		m_eAttachedToLast = ENameTagPosition.BODY;
	}
	
	string GetGroupName()
	{
		return m_sGroupName;
	}
	
	// Set group name so we can display in the layout
	override void SetGroup(SCR_AIGroup group)
	{
		if (group)
		{
			m_iGroupID = group.GetGroupID();
			m_sGroupName = group.GetCustomNameWithOriginal();
			if (m_bIsCurrentPlayer)
			{
				m_NTDisplay.CleanupAllTags();	// cleanup other tags because we need to compare groups again
				return;
			}
		
			if (m_NTDisplay.m_CurrentPlayerTag.m_iGroupID == m_iGroupID)
				ActivateEntityState(ENameTagEntityState.GROUP_MEMBER);
		}
		else
		{
			m_iGroupID = -1;
			if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
				DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);
			
			if (m_bIsCurrentPlayer)
				m_NTDisplay.CleanupAllTags();
			
			return;
		}
	}
	
	// Overriding this whole thing to force these fucking tags to the body
	override void SetTagPosition(ENameTagPosition pos, bool gradualChange = false)
	{
		gradualChange = false;
		m_eAttachedTo = ENameTagPosition.BODY;
		m_fTimeSlicePosChange = 0;
		m_vTagWorldPosLast = m_vTagWorldPos;
		
		if (!gradualChange)
			m_eAttachedToLast = pos;
	}
}