//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: shared helpers for CSI's nametag elements: distance-based text clearing, color team coloring,
//! 				and text and icon sizes from NAMTEAG_SCALE.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
modded class SCR_NTElementBase
{
	protected CSI_HUDSystem m_HUDSystem;
	protected CSI_SettingsManager m_SettingsManager;

	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	//! \return true if the tag should show only its icon: AI, no zone, or beyond the NAMTEAG_RANGE_SIMPLIFIED cutoff
	protected bool ShouldWeClearText(SCR_NameTagData data)
	{
		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone || data.m_eType == ENameTagEntityType.AI)
			return true;

		//-----------------------------------------------------------------------
		// Basic Static Distance Calculations
		int zoneEnd = zone.m_iZoneEnd;
		int zoneStart = zone.GetZoneStart();
		if (zoneStart < 1)
			zoneStart = 1;

		//-----------------------------------------------------------------------
		// Cutoff from the setting: the farthest N% of the zone shows icons only
		float scale = (m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED) * 0.01);
		float cutoffDist = scale * zoneEnd;
		cutoffDist = zoneEnd - cutoffDist;// flip it around

		//-----------------------------------------------------------------------
		// Dynamic Distance Calculation For Our Emulation Of Icon Nametags In Vanilla
		float dist = Math.InverseLerp(zoneStart, zoneEnd, data.m_fDistance);

		// Only players get simplified, and only when role icons are shown
		if (dist > cutoffDist
			&& scale != 0
			&& data.m_eType != ENameTagEntityType.AI
			&& data.m_eType != ENameTagEntityType.VEHICLE
			&& m_SettingsManager.GetSettingBool(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Colors an element: VON color while talking, color team for the local player's group members, else NONE.
	//! \param[in] data
	//! \param[in] index
	protected void SetWidgetCSIColor(SCR_NameTagData data, int index)
	{
		Widget widget = data.m_aNametagElements[index];
		if (!widget)
			return;

		if (data.m_ePriorityEntityState == ENameTagEntityState.VON)
			widget.SetColor(CSI_UIHelper.VANILLA_VON_COLOR);
		else if (data.m_PlayerData && m_HUDSystem.GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
			widget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
		else
			widget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
	}

	//------------------------------------------------------------------------------------------------
	//! \return font size for the NAMTEAG_SCALE setting (0-100 in steps of 20 maps to 8-13)
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
	//! \return icon size in px for the NAMTEAG_SCALE setting (0-100 in steps of 20 maps to 12-22)
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