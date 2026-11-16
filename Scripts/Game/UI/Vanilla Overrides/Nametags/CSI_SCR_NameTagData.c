//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: names with rank; adds player data, group name, field of view place and what CSI's
//! 				elements last set; tag position and height from settings.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_NameTagData : Managed
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	CSI_PlayerData m_PlayerData;	//!< null for AI and players without CSI data
	string m_sGroupName;			//!< set by GetGroupName
	float m_fFieldOfViewFraction;	//!< place in the tag's visible angle: 0 center, 1 edge

	// What CSI's elements last set, by m_aNametagElements index; a widget is only changed when its value differs
	string m_aElementTexts[10];		//!< text
	int m_aElementIconKeys[10];		//!< role icon, see SCR_NTIconBase.GetIconKey; 0 until set
	Color m_aElementColors[10];		//!< shared color; null until set

	//------------------------------------------------------------------------------------------------
	override void UpdateEntityType()
	{
		super.UpdateEntityType();
		UpdatePlayerData();
	}

	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		super.InitDefaults();

		// Cache managers
		if (!m_SettingsManager || !m_PlayerDataManager)
		{
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		};

		UpdateAttatchedTo();	// Position from NAMETAG_POSITION instead of HEAD
		m_PlayerData = null;
		m_sGroupName = string.Empty;
	};

	//------------------------------------------------------------------------------------------------
	override void GetName(out string name, out notnull array<string> nameParams)
	{
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			// CSI: name with rank, refresh CSI data
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
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group || !m_SettingsManager || !m_SettingsManager.GetSettingBool(CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE))
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
		UpdateAttatchedTo();	// Pick up NAMETAG_POSITION changes
		super.UpdateTagPos();

		// Tag position again, with the height offset from NAMETAG_POSITION_OFFSET (0.1 m steps)
		vector nametagOffsetVector = "0 0 0";
		int nametagOffset = 0;

		if (m_SettingsManager)
			nametagOffset = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_POSITION_OFFSET);

		if (m_eAttachedTo == ENameTagPosition.HEAD)
		{
			nametagOffsetVector[1] = ((nametagOffset + 1) * 0.1);
			m_vTagWorldPos = m_vEntHeadPos + nametagOffsetVector;
		}
		else if (m_eAttachedTo == ENameTagPosition.BODY)
		{
			nametagOffsetVector[1] = ((nametagOffset - 5) * 0.1);
			m_vTagWorldPos = m_vEntWorldPos + nametagOffsetVector;
		}
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

	//------------------------------------------------------------------------------------------------
	//! Sets m_eAttachedTo from NAMETAG_POSITION (BODY without settings).
	void UpdateAttatchedTo()
	{
		ENameTagPosition nametagPos = ENameTagPosition.BODY;

		if (m_SettingsManager)
			nametagPos = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_POSITION);

		m_eAttachedTo = nametagPos;
		m_eAttachedToLast = nametagPos;
	}
}
