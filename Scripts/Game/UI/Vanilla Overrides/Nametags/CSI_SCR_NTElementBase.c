//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: helpers for CSI's nametag elements: text clearing by distance and field of view,
//! 				shared fire team and nametag color setting colors set only when they change (a nametag color
//! 				that matches a fire team color shows Gray), and text and icon sizes from NAMTEAG_SCALE.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
modded class SCR_NTElementBase
{
	protected CSI_HUDSystem m_HUDSystem;
	protected CSI_SettingsManager m_SettingsManager;

	protected static ref map<int, ref Color> s_mColorTeamColors = new map<int, ref Color>();	//!< see GetColorTeamColor
	protected static ref map<int, ref Color> s_mPresetColors = new map<int, ref Color>();	//!< see GetNametagColor

	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	//! \return true if the tag should show only its icon
	protected bool ShouldWeClearText(SCR_NameTagData data)
	{
		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone)
			return true;

		// AI tags are always icon only (SCR_NTIconBase always shows their icon); players without CSI data have no
		// icon, so they keep their name
		if (data.m_eType == ENameTagEntityType.AI)
			return true;

		if (!data.m_PlayerData)
			return false;

		bool roleIcons = m_SettingsManager.GetSettingBool(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE);

		//-----------------------------------------------------------------------
		// Icon only beyond the NAMTEAG_RANGE_SIMPLIFIED cutoff. The percentage applies to the zone's squared distance
		// (m_fDistance), so 85% of a 50 m range starts at about 19 m, as CSI has always worked and as Setting Info
		// shows (CSI_SettingInfoNametagExamples.GetSimplifiedDistance); keep the two in step
		int zoneEnd = zone.m_iZoneEnd;
		int zoneStart = zone.GetZoneStart();
		if (zoneStart < 1)
			zoneStart = 1;
		float scale = (m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED) * 0.01);
		float cutoff = zoneEnd - scale * zoneEnd;
		float cutoffDistanceSq = zoneStart + cutoff * (zoneEnd - zoneStart);

		// Only players get simplified, and only when role icons are shown
		if (data.m_eType == ENameTagEntityType.VEHICLE || !roleIcons)
			return false;

		if (scale != 0 && data.m_fDistance > cutoffDistanceSq)
			return true;

		//-----------------------------------------------------------------------
		// The outer N% of the field of view shows icons only too
		int fieldOfViewPercentage = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED);
		float fieldOfViewScale = fieldOfViewPercentage * 0.01;
		if (fieldOfViewScale != 0 && data.m_fFieldOfViewFraction > 1 - fieldOfViewScale)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Nametag VON color while talking; local group members get their fire team's color, or the Standard squad
	//! color without one; everyone else the Nametag color.
	//! \param[in] data
	//! \param[in] index
	protected void SetWidgetCSIColor(SCR_NameTagData data, int index)
	{
		Color color;
		if (data.m_ePriorityEntityState == ENameTagEntityState.VON)
			color = GetNametagColor(CSI_GameSettings.NAMETAG_VON_COLOR);
		else if (!data.m_PlayerData || !IsLocalGroupMember(data))
			color = GetNametagColor(CSI_GameSettings.NAMETAG_COLOR);
		else if (data.m_PlayerData.GetColorTeam() != CSI_EColorTeam.NONE)
			color = GetColorTeamColor(data.m_PlayerData.GetColorTeam());
		else
			color = GetNametagColor(CSI_GameSettings.STANDARD_SQUAD_COLOR);

		SetElementColor(data, index, color);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	//! \return true if the tag's player is in the local player's group
	protected bool IsLocalGroupMember(SCR_NameTagData data)
	{
		if (!m_HUDSystem)
			return false;

		array<int> groupPlayerIds = m_HUDSystem.GetLocalGroupPlayerIds();
		return groupPlayerIds && groupPlayerIds.Contains(data.m_iPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting a CSI_GameSettings.NON_TEAM_COLOR_SETTINGS name
	//! \return the setting's color, Gray if it matches a fire team color (the VON color: CSI_ColorPresets.GetVONPreset);
	//! one shared instance per CSI_EColorPreset
	protected Color GetNametagColor(string setting)
	{
		int teamSet = m_SettingsManager.GetSettingInt(CSI_GameSettings.COLOR_TEAM_SET);
		int preset;
		if (setting == CSI_GameSettings.NAMETAG_VON_COLOR)
			preset = CSI_ColorPresets.GetVONPreset(m_SettingsManager.GetSettingInt(setting), teamSet, m_SettingsManager.GetSettingInt(CSI_GameSettings.STANDARD_SQUAD_COLOR), m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_COLOR));
		else
			preset = CSI_ColorPresets.GetNonClashingPreset(m_SettingsManager.GetSettingInt(setting), teamSet);

		Color color = s_mPresetColors.Get(preset);
		if (!color)
		{
			color = CSI_ColorPresets.GetColor(preset);
			s_mPresetColors.Insert(preset, color);
		}

		return color;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the element's color unless it's the color last set here.
	//! \param[in] data
	//! \param[in] index
	//! \param[in] color a shared color, compared by reference
	protected void SetElementColor(SCR_NameTagData data, int index, notnull Color color)
	{
		Widget widget = data.m_aNametagElements[index];
		if (!widget || color == data.m_aElementColors[index])
			return;

		widget.SetColor(color);
		data.m_aElementColors[index] = color;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] colorTeam a fire team, not NONE (GetNametagColor(STANDARD_SQUAD_COLOR))
	//! \return the color team's color in the chosen set, one shared instance per color team and set
	protected Color GetColorTeamColor(CSI_EColorTeam colorTeam)
	{
		int teamSet = m_SettingsManager.GetSettingInt(CSI_GameSettings.COLOR_TEAM_SET);
		int key = teamSet * 100 + colorTeam;
		Color color = s_mColorTeamColors.Get(key);
		if (!color)
		{
			color = CSI_ColorPresets.GetColorTeamColor(colorTeam, teamSet, CSI_EColorPreset.GRAY);
			s_mColorTeamColors.Insert(key, color);
		}

		return color;
	}

	//------------------------------------------------------------------------------------------------
	//! \return font size (8-13) for NAMTEAG_SCALE
	protected int GetNametagTextScale()
	{
		int scale = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_SCALE);

		switch (scale)
		{
			case 0 : return 8;
			case 20 : return 9;
			case 40 : return 10;
			case 60 : return 11;
			case 80 : return 12;
			default : return 13;
		}

		return 13;
	}

	//------------------------------------------------------------------------------------------------
	//! \return icon size in px (12-22) for NAMTEAG_SCALE
	protected int GetNametagImageScale()
	{
		int scale = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_SCALE);

		switch (scale)
		{
			case 0 : return 12;
			case 20 : return 14;
			case 40 : return 16;
			case 60 : return 18;
			case 80 : return 20;
			default : return 22;
		}

		return 22;
	}
};