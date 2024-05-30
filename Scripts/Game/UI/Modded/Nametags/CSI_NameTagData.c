modded class SCR_NameTagData : Managed
{
	const vector BODY_OFFSET = "0 -0.245 0";			// tag visual position offset for body

	protected CSI_ClientComponent m_ClientComponent;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected string m_sNametagsPos;

	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		if (!m_AuthorityComponent || !m_ClientComponent) 
		{
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			m_ClientComponent = CSI_ClientComponent.GetInstance();
		}
		
		m_eEntityStateFlags = ENameTagEntityState.HIDDEN | ENameTagEntityState.DEFAULT;
	 	m_ePriorityEntityState = ENameTagEntityState.HIDDEN;

		m_sNametagsPos = m_ClientComponent.ReturnLocalCSISettings()[11];

		if (m_sNametagsPos == "HEAD") 
		{
			m_eAttachedTo = ENameTagPosition.HEAD;
			m_eAttachedToLast = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
			m_eAttachedToLast = ENameTagPosition.BODY;
		};
		
		m_iZoneID = -1;
		m_iGroupID = -1;
		m_iPlayerID = -1;
		m_fTimeSliceUpdate = 1.0;
		m_fTimeSliceVON = 0;
		m_fTimeSlicePosChange = 0;
		m_fTimeSliceCleanup = 0;
		m_fTimeSliceFade = 0;
		m_fTimeSliceVisibility = 0;
		m_fDistance = 0;
		m_fOpacityFade = 1;
		m_sName = string.Empty;
		m_aNameParams = {};
	};

	//------------------------------------------------------------------------------------------------
	override void GetName(out string name, out notnull array<string> nameParams)
	{
		if (!m_ClientComponent) return;
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			string roleNametagVisible = m_ClientComponent.ReturnLocalCSISettings()[7];
			string rankVisible = m_ClientComponent.ReturnLocalCSISettings()[5];

			PlayerManager playerMgr = GetGame().GetPlayerManager();
			if (playerMgr)
			{
				m_sName = playerMgr.GetPlayerName(m_iPlayerID);

				if (rankVisible == "true")
				{
					string rank = m_AuthorityComponent.ReturnLocalPlayerMapValue(-1, m_iPlayerID, "PR"); // PR = PlayerRank
					
					if (!rank.IsEmpty()) 
						m_sName = string.Format("%1 %2", rank, m_sName);
				};
				if (roleNametagVisible == "true")
				{
					string icon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iPlayerID, "DI"); // DI = DisplayIcon
					
					if (icon != "MAN" && !icon.IsEmpty())
						m_sName = string.Format("%1 [%2]", m_sName, icon);
				}
			} else { m_sName = "No player manager!" };
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

		name = m_sName;
		nameParams.Copy(m_aNameParams);
	}

	//------------------------------------------------------------------------------------------------
	string GetGroupName()
	{
		// TODO: Better AI handling
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group) return "";

		string groupName = group.GetCustomName();

		if (groupName.IsEmpty())
		{
			string company, platoon, squad, character, format;
			group.GetCallsigns(company, platoon, squad, character, format);
			company = WidgetManager.Translate(company);
			groupName = string.Format(format, company, platoon, squad, character);
		};

		return groupName;
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerColorTeam()
	{		
		m_ClientComponent = CSI_ClientComponent.GetInstance();
		
		if (!m_ClientComponent) return 0;
		
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group || !m_ClientComponent || (!(m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER) || (m_ePriorityEntityState & ENameTagEntityState.VON))) return 0;

		return m_ClientComponent.SwitchStringToColorTeam(m_AuthorityComponent.ReturnLocalPlayerMapValue(group.GetGroupID(), m_iPlayerID, "CT")); // CT = ColorTeam
	}

	//------------------------------------------------------------------------------------------------
	void UpdateAttatchedTo()
	{
		if (!m_ClientComponent) return;

		m_sNametagsPos = m_ClientComponent.ReturnLocalCSISettings()[11];

		if (m_sNametagsPos == "HEAD") 
		{
			m_eAttachedTo = ENameTagPosition.HEAD;
			m_eAttachedToLast = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
			m_eAttachedToLast = ENameTagPosition.BODY;
		};
	}

	//------------------------------------------------------------------------------------------------
	override void SetTagPosition(ENameTagPosition pos, bool gradualChange = false)
	{
		if (m_sNametagsPos == "HEAD") 
		{
			m_eAttachedTo = ENameTagPosition.HEAD;
			m_eAttachedToLast = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
			m_eAttachedToLast = ENameTagPosition.BODY;
		};

		m_fTimeSlicePosChange = 0;
		m_vTagWorldPosLast = m_vTagWorldPos;
	}
}
