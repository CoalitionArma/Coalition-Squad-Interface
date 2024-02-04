//------------------------------------------------------------------------------------------------
//! Nametag UI 
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController
modded class SCR_NameTagDisplay
{																
	override protected void InitializeTag(IEntity entity, bool friendlyOnly = true)
	{
		if (!m_wRoot)
			return;
		
		// if this is a current controlled entity
		if (entity == SCR_PlayerController.GetLocalControlledEntity())
		{
			if (m_aUninitializedTags.IsEmpty())
				CreateTagWidget();
			
			// init tag & insert into arrays
			SCR_NameTagData tag = m_aUninitializedTags[0];
			tag.InitTag(this, entity, s_NametagCfg, true);
			m_CurrentPlayerTag = tag;
			
			m_aNameTags.Insert(tag);
			m_aNameTagEntities.Insert(entity, tag);
			m_aUninitializedTags.Remove(0);
			
			// Init FactionAffiliationComponent, required to detect factions
			FactionManager factionMgr = GetGame().GetFactionManager();
			if (factionMgr)
			{
				FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast( entity.FindComponent(FactionAffiliationComponent) );
				if (factionComponent)
					m_CurrentFaction = factionComponent.GetAffiliatedFaction();
			}
		
			return;
		}
		
		if (friendlyOnly)
		{
			if ( !IsFactionFriendly(entity) ) 	// Do not create tags for enemy factions
				return;
		}
			
		// init tag & insert into arrays			
		if (m_aUninitializedTags.IsEmpty())
			CreateTagWidget();
		
		SCR_NameTagData tag = m_aUninitializedTags[0];
		if ( tag.InitTag(this, entity, s_NametagCfg) )
		{	
			m_aNameTags.Insert(tag);
			m_aNameTagEntities.Insert(entity, tag);
			m_aUninitializedTags.Remove(0);
			// Add group name to player tag
			PrintFormat("Group name: %1",tag.m_sGroupName);
			TextWidget m_wPlayerGroup = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerGroupName"));
			m_wPlayerGroup.SetText(tag.m_sGroupName);
		}
	}
};