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
	const static ResourceName CSI_ICONS = "{4FE53F33D8545E0D}UI/Textures/HUD/Icons/CSI_ICONS.imageset";

	static ref array<CSI_EIcon> m_aNonThemedRegularIcons =
	{
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,
		CSI_EIcon.EMPTY,
	};
	
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
	//! Convert the color team enum to a color
	//! \param[in] colorEnum: The color enum to convert into a color
	//! \param[in] opacity: The opacity of the color (0-255)
	//! \return Color team color
	static Color ConvertColorTeamToColor(CSI_EColorTeam colorEnum, int opacity = 255)
	{	
		Color colorFromInt;
		
		switch(colorEnum)
		{
			case CSI_EColorTeam.BLUE : 		colorFromInt = Color.FromInt(ARGB(opacity, 0, 92, 255)); 	break; // BLUE
			case CSI_EColorTeam.RED : 		colorFromInt = Color.FromInt(ARGB(opacity, 200, 65, 65)); 	break; // RED
			case CSI_EColorTeam.GREEN : 		colorFromInt = Color.FromInt(ARGB(opacity, 0, 190, 85)); 	break; // GREEN
			case CSI_EColorTeam.YELLOW : 		colorFromInt = Color.FromInt(ARGB(opacity, 230, 230, 0)); 	break; // YELLOW
			default : 					 	colorFromInt = Color.FromInt(ARGB(opacity, 225, 225, 225)); // NONE
		}
		
		return colorFromInt; 
	}

	//------------------------------------------------------------------------------------------------
	//! Get icon string so we can set the icon through LoadImageFromSet
	//! \param[in] icon: Icon to find the string of
	//! \param[in] isSimpleIcon: Should we return the simple (non-circular) Icon
	//! \return Icon string
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
	//! Just a simple method to hold a switch statement for converting the compass enum to a resourceName
	//! \param[in] compassTheme: The inputed compass theme enum
	//! \return ResourceName of the inputed compass theme
	static ResourceName GetCompassThemeResource(CSI_ECompassTheme compassTheme)
	{
		switch (compassTheme)
		{
			default : return CSI_STANDARD_COMPASS;
		};
		
		// So the compiler doesnt yell at us
		return CSI_STANDARD_COMPASS;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get players name, with a check if we need to add a players rank
	//! \param[in] playerID: The inputed player ID to pull the name of and rank of
	//! \return Players name
	static string GetPlayersName(int playerID)
	{
		string name = GetGame().GetPlayerManager().GetPlayerName(playerID);
		CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(playerID);
		
		if (!playerData || !CSI_SettingsManager.GetInstance().GetSettingBool(CSI_GameSettings.RANK_VISIBLE))
			return name;
		
		SCR_ECharacterRank rankEnum = playerData.GetRank();
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		
		if (!faction)
			return name;
		
		string rank = faction.GetRankNameShort(rankEnum);
		
		if (rank.IsEmpty())
			return name;
		
		return string.Format("%1 %2", rank, name);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Search the widget hierarchy beginning at 'root' and collect icon widgets that match a given identifier
	//! \param[in] root: Root widget to start the search from.
	//! \param[in] widgetString: Substring/pattern used to identify matching icon widgets (matched against widget names/identifiers).
	//! \param[in] count: Maximum number of matches to collect; if non-positive, function will collect all matches.
	//! \return: An array of Widget handles for the matching icon widgets (empty if none found or if root is null).
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