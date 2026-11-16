//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	The colors a color setting can pick (CSI_EColorPreset; CSI_ColorPickerComponent shows them) and
//! 				the fire team color sets (CSI_EColorTeamSet). Read a setting's color with GetSettingColor, a
//! 				fire team's with CSI_UIHelper.ConvertColorTeamToColor.
//!
//------------------------------------------------------------------------------------------------
class CSI_ColorPresets
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] preset CSI_EColorPreset value
	//! \return a new color, white for an unknown value
	static Color GetColor(int preset)
	{
		switch (preset)
		{
			// First row: the bright vanilla map marker colors
			case CSI_EColorPreset.AMBER :			return new Color(0.947, 0.651, 0.136, 1);
			case CSI_EColorPreset.ORANGE :			return new Color(0.761, 0.392, 0.078, 1);	// the UI orange
			case CSI_EColorPreset.RED :				return new Color(0.855, 0.027, 0.028, 1);
			case CSI_EColorPreset.GREEN :			return new Color(0.014, 0.445, 0.08, 1);
			case CSI_EColorPreset.BLUE :			return new Color(0.002, 0.198, 0.847, 1);
			case CSI_EColorPreset.CYAN :			return new Color(0.014, 0.546, 0.896, 1);
			case CSI_EColorPreset.PINK :			return new Color(0.871, 0.039, 0.705, 1);
			case CSI_EColorPreset.PURPLE :			return new Color(0.45, 0.08, 0.86, 1);
			case CSI_EColorPreset.GRAY :			return FromRGB(0xE1E1E1);

			// Second row; some are fire team colors in a set (GetColorTeamColor)
			case CSI_EColorPreset.YELLOW :			return FromRGB(0xFFEB00);
			case CSI_EColorPreset.LIGHT_YELLOW :	return FromRGB(0xFFF08C);
			case CSI_EColorPreset.LIME :			return FromRGB(0x96E600);
			case CSI_EColorPreset.LIGHT_GREEN :		return FromRGB(0x82E696);
			case CSI_EColorPreset.TEAL :			return FromRGB(0x009E73);
			case CSI_EColorPreset.LIGHT_BLUE :		return FromRGB(0x82AAFF);
			case CSI_EColorPreset.LAVENDER :		return FromRGB(0xB496FF);
			case CSI_EColorPreset.LIGHT_RED :		return FromRGB(0xFF8282);
			case CSI_EColorPreset.LIGHT_PINK :		return FromRGB(0xFFAADC);
			case CSI_EColorPreset.BROWN :			return FromRGB(0xA06432);
		}

		return new Color(1, 1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rgb 0xRRGGBB
	//! \param[in] opacity 0-255
	//! \return a new color
	protected static Color FromRGB(int rgb, int opacity = 255)
	{
		return Color.FromInt(ARGB(opacity, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] preset CSI_EColorPreset value
	//! \return name shown to players, empty for an unknown value
	static string GetName(int preset)
	{
		switch (preset)
		{
			case CSI_EColorPreset.WHITE :	return "White";
			case CSI_EColorPreset.AMBER :	return "Amber";
			case CSI_EColorPreset.ORANGE :	return "Orange";
			case CSI_EColorPreset.RED :		return "Red";
			case CSI_EColorPreset.GREEN :	return "Green";
			case CSI_EColorPreset.BLUE :	return "Blue";
			case CSI_EColorPreset.CYAN :	return "Cyan";
			case CSI_EColorPreset.PINK :	return "Pink";
			case CSI_EColorPreset.PURPLE :	return "Purple";
			case CSI_EColorPreset.GRAY :	return "Gray";
			case CSI_EColorPreset.YELLOW :			return "Yellow";
			case CSI_EColorPreset.LIGHT_YELLOW :	return "Light yellow";
			case CSI_EColorPreset.LIME :			return "Lime";
			case CSI_EColorPreset.LIGHT_GREEN :		return "Light green";
			case CSI_EColorPreset.TEAL :			return "Teal";
			case CSI_EColorPreset.LIGHT_BLUE :		return "Light blue";
			case CSI_EColorPreset.LAVENDER :		return "Lavender";
			case CSI_EColorPreset.LIGHT_RED :		return "Light red";
			case CSI_EColorPreset.LIGHT_PINK :		return "Light pink";
			case CSI_EColorPreset.BROWN :			return "Brown";
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \return number of colors (values 0 to Count() - 1)
	static int Count()
	{
		array<int> presets = {};
		return SCR_Enum.GetEnumValues(CSI_EColorPreset, presets);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting a CSI_GameSettings.COLOR_SETTINGS name
	//! \return the setting's color, white without a settings manager
	static Color GetSettingColor(string setting)
	{
		CSI_SettingsManager settingsManager = CSI_SettingsManager.GetInstance();
		if (!settingsManager)
			return GetColor(CSI_EColorPreset.WHITE);

		return GetColor(settingsManager.GetSettingInt(setting));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] colorTeam
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \param[in] noTeamPreset STANDARD_SQUAD_COLOR value (CSI_EColorPreset), NONE's color
	//! \param[in] opacity 0-255
	//! \return a new color
	static Color GetColorTeamColor(CSI_EColorTeam colorTeam, int teamSet, int noTeamPreset, int opacity = 255)
	{
		// 0xRRGGBB per set: Classic, Bright, Pastel, Color-blind
		int rgb;
		switch (colorTeam)
		{
			case CSI_EColorTeam.BLUE :		rgb = PickForSet(teamSet, 0x005CFF, 0x196EFF, 0x82AAFF, 0x0072B2); break;
			case CSI_EColorTeam.RED :		rgb = PickForSet(teamSet, 0xC84141, 0xFF2D2D, 0xFF8282, 0xD55E00); break;
			case CSI_EColorTeam.GREEN :		rgb = PickForSet(teamSet, 0x00BE55, 0x28EB46, 0x82E696, 0x009E73); break;
			case CSI_EColorTeam.YELLOW :	rgb = PickForSet(teamSet, 0xE6E600, 0xFFF000, 0xFFF08C, 0xF0E442); break;
			default :
			{
				Color noTeamColor = GetColor(GetNonClashingPreset(noTeamPreset, teamSet));
				noTeamColor.SetA(opacity / 255.0);
				return noTeamColor;
			}
		}

		return FromRGB(rgb, opacity);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] colorTeam
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \return the palette color (CSI_EColorPreset) the fire team's color matches, -1 for NONE
	static int GetColorTeamPreset(CSI_EColorTeam colorTeam, int teamSet)
	{
		switch (colorTeam)
		{
			case CSI_EColorTeam.BLUE :		return PickForSet(teamSet, CSI_EColorPreset.BLUE, CSI_EColorPreset.BLUE, CSI_EColorPreset.LIGHT_BLUE, CSI_EColorPreset.BLUE);
			case CSI_EColorTeam.RED :		return PickForSet(teamSet, CSI_EColorPreset.RED, CSI_EColorPreset.RED, CSI_EColorPreset.LIGHT_RED, CSI_EColorPreset.ORANGE);
			case CSI_EColorTeam.GREEN :		return PickForSet(teamSet, CSI_EColorPreset.GREEN, CSI_EColorPreset.GREEN, CSI_EColorPreset.LIGHT_GREEN, CSI_EColorPreset.TEAL);
			case CSI_EColorTeam.YELLOW :	return PickForSet(teamSet, CSI_EColorPreset.YELLOW, CSI_EColorPreset.YELLOW, CSI_EColorPreset.LIGHT_YELLOW, CSI_EColorPreset.YELLOW);
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] preset CSI_EColorPreset value
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \return true if one of the set's fire team colors matches it
	static bool ClashesWithColorTeams(int preset, int teamSet)
	{
		return preset == GetColorTeamPreset(CSI_EColorTeam.BLUE, teamSet)
			|| preset == GetColorTeamPreset(CSI_EColorTeam.RED, teamSet)
			|| preset == GetColorTeamPreset(CSI_EColorTeam.GREEN, teamSet)
			|| preset == GetColorTeamPreset(CSI_EColorTeam.YELLOW, teamSet);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] preset a CSI_GameSettings.NON_TEAM_COLOR_SETTINGS value (CSI_EColorPreset)
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \return the preset, or GRAY if it matches a fire team color
	static int GetNonClashingPreset(int preset, int teamSet)
	{
		if (ClashesWithColorTeams(preset, teamSet))
			return CSI_EColorPreset.GRAY;

		return preset;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \return name shown to players
	static string GetColorTeamSetName(int teamSet)
	{
		switch (teamSet)
		{
			case CSI_EColorTeamSet.BRIGHT :		return "Bright";
			case CSI_EColorTeamSet.PASTEL :		return "Pastel";
			case CSI_EColorTeamSet.COLORBLIND :	return "Color-blind";
		}

		return "Classic";
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] teamSet CSI_EColorTeamSet value, CLASSIC if unknown
	//! \param[in] classic
	//! \param[in] bright
	//! \param[in] pastel
	//! \param[in] colorblind
	//! \return the value for the set
	protected static int PickForSet(int teamSet, int classic, int bright, int pastel, int colorblind)
	{
		switch (teamSet)
		{
			case CSI_EColorTeamSet.BRIGHT :		return bright;
			case CSI_EColorTeamSet.PASTEL :		return pastel;
			case CSI_EColorTeamSet.COLORBLIND :	return colorblind;
		}

		return classic;
	}
}
