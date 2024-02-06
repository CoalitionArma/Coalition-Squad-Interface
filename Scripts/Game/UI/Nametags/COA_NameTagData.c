enum ECOATagEntityState: ENameTagEntityState
{
	RED_TEAM		= 1<<7,
	BLUE_TEAM		= 1<<8,
	GREEN_TEAM		= 1<<9,
	YELLOW_TEAM		= 1<<10
};

enum ECOANameTagFlags: ENameTagFlags
{
	GROUP_UPDATE 		= 1<<13		// request group update
};

modded class SCR_NameTagData
{
	ECOATagEntityState m_eEntityStateFlags;
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
		// TODO: Better AI handling
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		
		if (group) 
			m_sGroupName = group.GetCustomNameWithOriginal();
		else
			m_sGroupName = "";
		
		PrintFormat("[CSI NT] Group name: %1",m_sGroupName);
		return m_sGroupName;
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