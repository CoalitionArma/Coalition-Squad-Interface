//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-19
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: names with rank; adds player data, group name, field of view place, what CSI's
//! 				elements show (UpdateCSIState) and what they last set; tag position and height from settings; a
//! 				passenger talking marks their vehicle's tag too.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_NameTagData : Managed
{
	protected CSI_PlayerDataManager m_PlayerDataManager;
	CSI_PlayerData m_PlayerData;	//!< null for AI and players without CSI data
	string m_sGroupName;			//!< set by GetGroupName
	float m_fFieldOfViewFraction;	//!< place in the tag's visible angle: 0 center, 1 edge

	// What CSI's elements show, set by UpdateCSIState
	bool m_bLocalGroupMember;		//!< the tag's player is in the local player's group
	bool m_bIconOnly;				//!< only the icon shows: the name and group name are cleared
	Color m_TagColor;				//!< shared color (CSI_NametagSettings) of the name and role icon

	// What CSI's elements last set, by m_aNametagElements index; a widget is only changed when its value differs
	string m_aElementTexts[10];		//!< text
	int m_aElementIconKeys[10];		//!< role icon, see SCR_NTIconBase.GetIconKey; 0 until set
	Color m_aElementColors[10];		//!< shared color; null until set

	//------------------------------------------------------------------------------------------------
	override void ActivateEntityState(ENameTagEntityState state)
	{
		super.ActivateEntityState(state);

		// A passenger's own tag is hidden, so their vehicle's tag shows them talking (vanilla VoN and Coalition VON
		// both mark the passenger); vanilla's ruleset ends it VON_STOP_TIME after the last mark
		if (state == ENameTagEntityState.VON && m_VehicleParent && !(m_VehicleParent.m_Flags & ENameTagFlags.DELETED))
			m_VehicleParent.ActivateEntityState(state);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateEntityType()
	{
		// A dead player's body is no longer theirs (no player ID), which would make the tag an AI's: plain color, no
		// CSI data. It stays theirs until it's cleaned up; every name update (RefreshNames) runs this
		if ((m_eEntityStateFlags & ENameTagEntityState.DEAD) && m_eType == ENameTagEntityType.PLAYER && m_iPlayerID > 0)
			return;

		super.UpdateEntityType();
		UpdatePlayerData();
	}

	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		super.InitDefaults();

		m_PlayerData = null;
		m_sGroupName = string.Empty;
	};

	//------------------------------------------------------------------------------------------------
	override void GetName(out string name, out notnull array<string> nameParams)
	{
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			// CSI: name with rank, refresh CSI data. A player who has left has no name until their character goes;
			// keep the last one (SCR_NameTagDisplay.RefreshNames asks every 2 s) instead of vanilla's GETNAME_ERROR
			if (!SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_iPlayerID).IsEmpty() || m_sName.IsEmpty())
				m_sName = CSI_UIHelper.GetPlayersName(m_iPlayerID);

			UpdatePlayerData();
		} else if (m_eType == ENameTagEntityType.AI)
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
			};
		};

		name = m_sName;
		nameParams.Copy(m_aNameParams);
	}

	//------------------------------------------------------------------------------------------------
	//! Set visibility of nametag widget
	//! \param[in] widget
	//! \param[in] visible
	//! \param[in] visibleOpacity
	//! \param[in] animate unused, it always fades when the ruleset has a fade time
	override void SetVisibility(Widget widget, bool visible, float visibleOpacity, bool animate = true)
	{
		// Super only makes the widget visible when it doesn't fade
		if (widget && visible && visibleOpacity >= 0.1)
			widget.SetVisible(true);

		super.SetVisibility(widget, visible, visibleOpacity, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets m_sGroupName to the group's custom name or callsign; empty if none or the setting is off.
	void GetGroupName()
	{
		// TODO: Better AI handling
		SCR_AIGroup group;
		if (m_GroupManager)
			group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group || !CSI_NametagSettings.s_bGroupNames)
		{
			m_sGroupName = "";
			return;
		}

		m_sGroupName = group.GetCustomName();

		if (m_sGroupName.IsEmpty())
		{
			string company, platoon, squad, character, format;
			group.GetCallsigns(company, platoon, squad, character, format);
			company = WidgetManager.Translate(company);
			m_sGroupName = string.Format(format, company, platoon, squad, character);
		};
	}

	//------------------------------------------------------------------------------------------------
	//! Update tag position
	override void UpdateTagPos()
	{
		super.UpdateTagPos();

		// Tag position again: at the head or body by NAMETAG_POSITION, with the NAMETAG_POSITION_OFFSET height.
		// Vanilla's own head/body switching (m_eAttachedTo) is left to vanilla
		if (CSI_NametagSettings.s_iTagPosition == ENameTagPosition.HEAD)
			m_vTagWorldPos = m_vEntHeadPos + CSI_NametagSettings.s_vTagOffset;
		else
			m_vTagWorldPos = m_vEntWorldPos + CSI_NametagSettings.s_vTagOffset;
	}

	//------------------------------------------------------------------------------------------------
	//! Works out what CSI's elements show for this tag. SCR_NameTagZone calls it once before the tag's elements are
	//! drawn or reset, so the elements only read the result.
	//! \param[in] zone the zone the tag is drawn in
	void UpdateCSIState(notnull SCR_NameTagZone zone)
	{
		array<int> localGroupPlayerIds = SCR_NameTagDisplay.s_aLocalGroupPlayerIds;
		m_bLocalGroupMember = localGroupPlayerIds && localGroupPlayerIds.Contains(m_iPlayerID);

		// Icon only: AI always (their icon always shows). Players only with CSI data (without it they have no icon)
		// and role icons shown, beyond the zone's icon-only distance or in the outer part of the field of view
		if (m_eType == ENameTagEntityType.AI)
			m_bIconOnly = true;
		else if (!m_PlayerData || m_eType == ENameTagEntityType.VEHICLE || !CSI_NametagSettings.s_bRoleIcons)
			m_bIconOnly = false;
		else
			m_bIconOnly = m_fDistance > zone.m_fIconOnlyDistanceSq || (CSI_NametagSettings.s_fIconOnlyFieldOfView != 0 && m_fFieldOfViewFraction > 1 - CSI_NametagSettings.s_fIconOnlyFieldOfView);

		// Color: the talking color while talking, from the VON flag rather than the priority state (HIDDEN outranks
		// VON, so a talking tag fading out would turn back mid-fade; DEAD still wins, as in vanilla). A local group
		// member gets their fire team's color, or the Standard squad color without one; everyone else the Nametag color
		if ((m_eEntityStateFlags & ENameTagEntityState.VON) != 0 && (m_eEntityStateFlags & ENameTagEntityState.DEAD) == 0)
			m_TagColor = CSI_NametagSettings.s_VONColor;
		else if (!m_PlayerData || !m_bLocalGroupMember)
			m_TagColor = CSI_NametagSettings.s_NametagColor;
		else
			m_TagColor = CSI_NametagSettings.GetColorTeamColor(m_PlayerData.GetColorTeam());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerData()
	{
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_PlayerData = null;

		// A possessed AI stays AI but has the possessing player's ID
		if (m_PlayerDataManager && m_eType != ENameTagEntityType.AI && m_iPlayerID > 0)
			m_PlayerData = m_PlayerDataManager.GetPlayerData(m_iPlayerID);
	}
}
