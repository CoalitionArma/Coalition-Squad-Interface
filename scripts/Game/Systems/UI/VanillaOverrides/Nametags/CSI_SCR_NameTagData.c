modded class SCR_NameTagData : Managed
{
	const vector BODY_OFFSET = "0 -0.315 0"; // tag visual position offset for body

	protected CSI_SettingsManager m_SettingsManager;

	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		if (!m_SettingsManager) 
			m_SettingsManager = CSI_SettingsManager.GetInstance();
		
		m_eEntityStateFlags = ENameTagEntityState.HIDDEN | ENameTagEntityState.DEFAULT;
	 	m_ePriorityEntityState = ENameTagEntityState.HIDDEN;

		UpdateAttatchedTo();
		
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
		if (m_eType == ENameTagEntityType.PLAYER)
			m_sName = CSI_UIHelper.GetPlayersName(m_iPlayerID);
			
		else if (m_eType == ENameTagEntityType.AI)
		{
			SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(m_Entity.FindComponent(SCR_CharacterIdentityComponent));
			if (scrCharIdentity)
			{
				scrCharIdentity.GetFormattedFullName(m_sName, m_aNameParams);
			} else {
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
	//! Update tag position
	override void UpdateTagPos()
	{
		vector matPos[4];
		Animation anim = m_Entity.GetAnimation();
		anim.GetBoneMatrix(m_iSpineBone, matPos);
		m_vEntWorldPos = m_Entity.CoordToParent(matPos[3]);
		anim.GetBoneMatrix(m_iHeadBone, matPos);
		m_vEntHeadPos = m_Entity.CoordToParent(matPos[3]);
		
		vector nametagOffsetVector = "0 0 0";
		int nametagOffset = m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.NAMETAG_POSITION_OFFSET);
		
		nametagOffsetVector[1] = ((nametagOffset - 5) * 0.1);
		
		if (m_eAttachedTo == ENameTagPosition.HEAD)
		{
			m_vTagWorldPos = m_vEntHeadPos + nametagOffsetVector;
		}
		else if (m_eAttachedTo == ENameTagPosition.BODY)
		{
			m_vTagWorldPos = m_vEntWorldPos + nametagOffsetVector;
		}

		if (m_eType != ENameTagEntityType.PLAYER && GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Entity) > 0)
		{
			m_Flags |= ENameTagFlags.ENT_TYPE_UPDATE;
			m_Flags |= ENameTagFlags.NAME_UPDATE;
		}
	}

	//------------------------------------------------------------------------------------------------
	string GetGroupName()
	{
		// TODO: Better AI handling
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group || !m_SettingsManager.GetCSISettingBool(CSI_SettingsManager.GROUP_IN_NAMETAG_VISIBLE)) 
			return "";

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
	void UpdateAttatchedTo()
	{
		if (!m_SettingsManager) 
			return;

		ENameTagPosition nametagPos = m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.NAMETAG_POSITION);

		m_eAttachedTo = nametagPos;
		m_eAttachedToLast = nametagPos;
	}
}
