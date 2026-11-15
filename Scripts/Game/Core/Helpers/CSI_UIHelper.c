//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Static UI helpers: resource paths, color team colors, compass textures, icon names and player names.
//! 
//------------------------------------------------------------------------------------------------
class CSI_UIHelper
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	// Vanilla Images/Imagesets/Colors
	const static ResourceName VANILLA_NAMETAG_ICONS = "{70E828A2F6EBE7D0}UI/Textures/Nametags/nametagicons.imageset";
	const static ref Color VANILLA_VON_COLOR = new Color(0.624, 0.506, 0.157, 1);

	// Modded Images/Imagesets
	const static ResourceName CSI_STANDARD_COMPASS = "{D19C93F5109F3E1D}UI/Textures/HUD/Compasses/Standard_Compass.edds";
	const static ResourceName CSI_SCI_FI_COMPASS = "{64AB432AC7BB503C}UI/Textures/HUD/Compasses/SciFi_Compass.edds";	//!< faint rings at 1/3 and 2/3 of radar range
	const static ResourceName CSI_WW2_COMPASS = "{77D4A8C53182F77A}UI/Textures/HUD/Compasses/WW2_Compass.edds";
	const static ResourceName CSI_ICONS = "{4FE53F33D8545E0D}UI/Textures/HUD/Icons/CSI_ICONS.imageset";

	const static ResourceName CSI_ICON_LAYOUT = "{4E4F52A9958E840C}UI/Layouts/HUD/Elements/Player Elements/CSI_Icon.layout";
	const static ResourceName CSI_PLAYER_LAYOUT = "{8904AB7E4A74F558}UI/Layouts/HUD/Elements/Player Elements/CSI_Player.layout";	//!< one group list row

	// Icons with a single image regardless of CSI_EIconTheme
	static ref array<CSI_EIcon> m_aNonThemedRegularIcons =
	{
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,
		CSI_EIcon.EMPTY,
	};

	// Seat icons; also unthemed, and drawn with the large icon widgets
	static ref array<CSI_EIcon> m_aVehicleIcons =
	{
		CSI_EIcon.DRIVER,
		CSI_EIcon.PASSANGER,
		CSI_EIcon.COMMANDER,
		CSI_EIcon.GUNNER,
		CSI_EIcon.HELIPILOT,
		CSI_EIcon.HELICREW,
	};

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC HELPER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] colorEnum
	//! \param[in] opacity alpha (0-255)
	//! \return the color team's color, light grey for NONE
	static Color ConvertColorTeamToColor(CSI_EColorTeam colorEnum, int opacity = 255)
	{
		Color colorFromInt;

		switch(colorEnum)
		{
			case CSI_EColorTeam.BLUE : 		colorFromInt = Color.FromInt(ARGB(opacity, 0, 92, 255)); 	break;
			case CSI_EColorTeam.RED : 		colorFromInt = Color.FromInt(ARGB(opacity, 200, 65, 65)); 	break;
			case CSI_EColorTeam.GREEN : 		colorFromInt = Color.FromInt(ARGB(opacity, 0, 190, 85)); 	break;
			case CSI_EColorTeam.YELLOW : 		colorFromInt = Color.FromInt(ARGB(opacity, 230, 230, 0)); 	break;
			default : 					 	colorFromInt = Color.FromInt(ARGB(opacity, 225, 225, 225)); // NONE
		}

		return colorFromInt;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] theme
	//! \return the theme's texture, the standard one if it has none
	static ResourceName GetCompassTexture(CSI_ECompassTheme theme)
	{
		switch (theme)
		{
			case CSI_ECompassTheme.SCI_FI :	return CSI_SCI_FI_COMPASS;
			case CSI_ECompassTheme.WW2 :	return CSI_WW2_COMPASS;
		}

		return CSI_STANDARD_COMPASS;
	}

	//------------------------------------------------------------------------------------------------
	//! Imageset entry name in CSI_ICONS; themed icons follow the ICON_THEME setting.
	//! \param[in] icon
	//! \param[in] isSimpleIcon non-circular version ("_ICON")
	//! \param[in] isAltIcon alternative version ("_ALT")
	//! \return e.g. "CLASSIC MEDIC_ICON"
	static string GetIconString(CSI_EIcon icon, bool isSimpleIcon = false, bool isAltIcon = false)
	{
		string iconString;
		CSI_EIconTheme theme = CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.ICON_THEME);

		if (m_aNonThemedRegularIcons.Contains(icon) || m_aVehicleIcons.Contains(icon))
			iconString = string.Format("%1", SCR_Enum.GetEnumName(CSI_EIcon, icon));
		else
			iconString = string.Format("%1 %2", SCR_Enum.GetEnumName(CSI_EIconTheme, theme), SCR_Enum.GetEnumName(CSI_EIcon, icon));

		if (isSimpleIcon)
			iconString = iconString + "_ICON";

		if (isAltIcon)
			iconString = iconString + "_ALT";

		return iconString;
	}

	//------------------------------------------------------------------------------------------------
	//! Player's display name, prefixed with their short rank when RANK_VISIBLE is on.
	//! \param[in] playerID
	//! \return
	static string GetPlayersName(PlayerId playerID)
	{
		// Player name with '<', '>' and '#' removed
		string name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);

		// Plain name on game modes without CSI's managers
		CSI_PlayerDataManager playerDataManager = CSI_PlayerDataManager.GetInstance();
		CSI_SettingsManager settingsManager = CSI_SettingsManager.GetInstance();
		if (!playerDataManager || !settingsManager)
			return name;

		CSI_PlayerData playerData = playerDataManager.GetPlayerData(playerID);

		if (!playerData || !settingsManager.GetSettingBool(CSI_GameSettings.RANK_VISIBLE))
			return name;

		SCR_ECharacterRank rankEnum = playerData.GetRank();
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));

		if (!faction || !faction.GetRanks())
			return name;

		string rank = faction.GetRanks().GetRankNameShort(rankEnum);

		if (rank.IsEmpty())
			return name;

		return string.Format("%1 %2", rank, name);
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the widgets named widgetString + 0 to widgetString + (count - 1) under root.
	//! \param[in] root
	//! \param[in] widgetString name prefix
	//! \param[in] count
	//! \return found widgets in index order
	static array<Widget> GetAllIcons(Widget root, string widgetString, int count)
	{
		array<Widget> icons = {};

		if (!root || widgetString.IsEmpty() || count <= 0)
			return icons;

		for (int e = 0; e < count; e++)
		{
			Widget icon = root.FindAnyWidget(widgetString + e.ToString());

			if (icon)
				icons.Insert(icon);
		};

		return icons;
	};
}