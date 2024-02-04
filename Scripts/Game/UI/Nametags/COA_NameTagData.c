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
	string m_sGroupName;
	
	override void InitDefaults()
	{
		super.InitDefaults();
		// Set tags to be over the body by default
		m_eAttachedTo = ENameTagPosition.BODY;
		m_eAttachedToLast = ENameTagPosition.BODY;
	}
	
	override void GetName(out string name, out notnull array<string> nameParams)
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			PlayerManager playerMgr = GetGame().GetPlayerManager();
			if (playerMgr)
				m_sName = playerMgr.GetPlayerName(m_iPlayerID);
			else 
				m_sName = "No player manager!";
		}
		else if (m_eType == ENameTagEntityType.AI)
		{
			SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(m_Entity.FindComponent(SCR_CharacterIdentityComponent));
			if (scrCharIdentity)
			{
				scrCharIdentity.GetFormattedFullName(m_sName, m_aNameParams);
			}
			else
			{
				CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(m_Entity.FindComponent(CharacterIdentityComponent));
				if (charIdentity && charIdentity.GetIdentity())
					m_sName = charIdentity.GetIdentity().GetName();
				else 
					m_sName = "No character identity!";
				
			}
		}
		
		if (group) 
			m_sGroupName = group.GetCustomNameWithOriginal();
		else
			m_sGroupName = "Unknown Group";
		
		name = m_sName;
		nameParams.Copy(m_aNameParams);
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