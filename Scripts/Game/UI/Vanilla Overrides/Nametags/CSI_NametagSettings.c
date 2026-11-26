//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-25
//! \Source: 		Coalition Squad Interface
//!	\Description: 	The settings CSI's nametag classes use for every tag on every frame, read once per settings
//! 				update (SCR_NameTagDisplay.OnSettingsUpdate) into plain values and shared colors, so drawing a
//! 				tag looks nothing up; also the size and icon-only distance formulas, which Setting Info shares.
//!
//------------------------------------------------------------------------------------------------
class CSI_NametagSettings
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 VALUES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected static const string ROLE_ICON_WIDGET_PREFIX = "RoleIcon";	//!< CSI_Nametag.layout: RoleIconLEFT, RoleIconCENTER, RoleIconRIGHT

	static bool s_bRoleIcons;				//!< ROLE_IN_NAMETAG_VISIBLE
	static string s_sRoleIconWidgetName;	//!< the RoleIcon widget NAMETAG_ROLE_ICON_POSITION picks
	static int s_iIconTheme;				//!< ICON_THEME
	static int s_iTextSize;					//!< name font size (8-13) for NAMTEAG_SCALE
	static int s_iIconSize;					//!< icon size in px (12-22) for NAMTEAG_SCALE
	static float s_fIconOnlyRange;			//!< NAMTEAG_RANGE_SIMPLIFIED as a share of the range (0-1)
	static float s_fIconOnlyFieldOfView;	//!< NAMETAG_FIELD_OF_VIEW_SIMPLIFIED as a share of the field of view (0-1)
	static bool s_bGroupNames;				//!< GROUP_IN_NAMETAG_VISIBLE
	static bool s_bSquadmateGroupNames;		//!< SQUADMATE_GROUP_IN_NAMETAG_VISIBLE
	static bool s_bLineOfSight = true;		//!< NAMETAG_LOS_VISIBLE
	static int s_iFieldOfView;				//!< NAMETAG_FIELD_OF_VIEW in degrees; 0 = not loaded yet

	static int s_iTagPosition = ENameTagPosition.BODY;	//!< NAMETAG_POSITION, an ENameTagPosition
	static vector s_vTagOffset = "0 -0.5 0";			//!< added to the head or body position for NAMETAG_POSITION_OFFSET

	// Shared instances: the elements compare colors by reference
	static ref Color s_NametagColor = new Color(1, 1, 1, 1);			//!< NAMETAG_COLOR as drawn
	static ref Color s_StandardSquadColor = new Color(1, 1, 1, 1);	//!< STANDARD_SQUAD_COLOR as drawn
	static ref Color s_VONColor = new Color(1, 1, 1, 1);				//!< NAMETAG_VON_COLOR as drawn (CSI_ColorPresets.GetVONPreset)

	protected static int s_iColorTeamSet;	//!< COLOR_TEAM_SET, a CSI_EColorTeamSet
	protected static ref map<int, ref Color> s_mPresetColors = new map<int, ref Color>();			//!< one instance per CSI_EColorPreset
	protected static ref map<int, ref Color> s_mColorTeamSetColors = new map<int, ref Color>();	//!< one instance per fire team and set

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] colorTeam
	//! \return the fire team's color in the chosen set, the Standard squad color for NONE
	static Color GetColorTeamColor(CSI_EColorTeam colorTeam)
	{
		if (colorTeam == CSI_EColorTeam.NONE)
			return s_StandardSquadColor;

		return GetColorTeamSetColor(colorTeam, s_iColorTeamSet);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scale NAMTEAG_SCALE value (0-100 in steps of 20)
	//! \return name font size (8-13)
	static int GetTextSize(int scale)
	{
		return 8 + Math.ClampInt(scale / 20, 0, 5);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scale NAMTEAG_SCALE value (0-100 in steps of 20)
	//! \return icon size in px (12-22)
	static int GetIconSize(int scale)
	{
		return 12 + Math.ClampInt(scale / 20, 0, 5) * 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Squared distance beyond which a player's tag shows only its icon. The share applies to the zone's squared
	//! distance, so 85% of a 50 m range starts at about 19 m.
	//! \param[in] share NAMTEAG_RANGE_SIMPLIFIED as a share (0-1); 0 turns it off
	//! \param[in] zoneStart meters
	//! \param[in] zoneEnd meters
	//! \return float.MAX for a share of 0
	static float GetIconOnlyDistanceSq(float share, int zoneStart, int zoneEnd)
	{
		if (share == 0)
			return float.MAX;

		int start = zoneStart;
		if (start < 1)
			start = 1;

		float cutoff = zoneEnd - share * zoneEnd;
		return start + cutoff * (zoneEnd - start);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Reads every value again.
	//! \param[in] settingsManager
	static void Refresh(notnull CSI_SettingsManager settingsManager)
	{
		s_bRoleIcons = settingsManager.GetSettingBool(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE);
		s_sRoleIconWidgetName = ROLE_ICON_WIDGET_PREFIX + SCR_Enum.GetEnumName(CSI_ENametagIconPosition, settingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION));
		s_iIconTheme = settingsManager.GetSettingInt(CSI_GameSettings.ICON_THEME);

		int scale = settingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_SCALE);
		s_iTextSize = GetTextSize(scale);
		s_iIconSize = GetIconSize(scale);

		s_fIconOnlyRange = settingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED) * 0.01;
		s_fIconOnlyFieldOfView = settingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED) * 0.01;
		s_bGroupNames = settingsManager.GetSettingBool(CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE);
		s_bSquadmateGroupNames = settingsManager.GetSettingBool(CSI_GameSettings.SQUADMATE_GROUP_IN_NAMETAG_VISIBLE);
		s_bLineOfSight = settingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_LOS_VISIBLE);
		s_iFieldOfView = settingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW);

		// Height in 0.1 m steps: from 0.1 m above the head, or from 0.5 m below the neck
		s_iTagPosition = settingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_POSITION);
		int positionOffset = settingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_POSITION_OFFSET);
		vector tagOffset = vector.Zero;
		if (s_iTagPosition == ENameTagPosition.HEAD)
			tagOffset[1] = (positionOffset + 1) * 0.1;
		else
			tagOffset[1] = (positionOffset - 5) * 0.1;

		s_vTagOffset = tagOffset;

		// Colors as drawn: a nametag color that matches a fire team color shows Gray, the talking color stands out
		s_iColorTeamSet = settingsManager.GetSettingInt(CSI_GameSettings.COLOR_TEAM_SET);
		int standardSquadPreset = settingsManager.GetSettingInt(CSI_GameSettings.STANDARD_SQUAD_COLOR);
		int nametagPreset = settingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_COLOR);
		int vonPreset = settingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_VON_COLOR);

		s_NametagColor = GetPresetColor(CSI_ColorPresets.GetNonClashingPreset(nametagPreset, s_iColorTeamSet));
		s_StandardSquadColor = GetPresetColor(CSI_ColorPresets.GetNonClashingPreset(standardSquadPreset, s_iColorTeamSet));
		s_VONColor = GetPresetColor(CSI_ColorPresets.GetVONPreset(vonPreset, s_iColorTeamSet, standardSquadPreset, nametagPreset));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] preset CSI_EColorPreset value
	//! \return the preset's shared color
	protected static Color GetPresetColor(int preset)
	{
		Color color = s_mPresetColors.Get(preset);
		if (!color)
		{
			color = CSI_ColorPresets.GetColor(preset);
			s_mPresetColors.Insert(preset, color);
		}

		return color;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] colorTeam a fire team, not NONE
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \return the fire team's shared color in that set
	protected static Color GetColorTeamSetColor(CSI_EColorTeam colorTeam, int teamSet)
	{
		int key = teamSet * 100 + colorTeam;
		Color color = s_mColorTeamSetColors.Get(key);
		if (!color)
		{
			color = CSI_ColorPresets.GetColorTeamColor(colorTeam, teamSet, CSI_EColorPreset.GRAY);
			s_mColorTeamSetColors.Insert(key, color);
		}

		return color;
	}
}
