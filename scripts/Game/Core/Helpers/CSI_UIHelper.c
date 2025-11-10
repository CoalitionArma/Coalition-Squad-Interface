class CSI_UIHelper
{	
	const static string VANILLA_MAN = "{2E717F4664C6E49D}UI/Textures/Nametags/Nametag-Filter-Icons/Player.edds";
	const static string STANDARD_COMPASS_RESOURCE = "{D19C93F5109F3E1D}UI/Textures/HUD/Compasses/Standard_Compass.edds";
	const static string CSI_ICONS_RESOURCE = "{4FE53F33D8545E0D}UI/Textures/HUD/Icons/CSI_ICONS.imageset";

	static ref array<CSI_EIcon> m_aNonThemedRegularIcons =
	{
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,
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
	
	//------------------------------------------------------------------------------------------------
	/**
	* Convert the color team enum to a color
	* @param colorEnum: The color enum to convert into a color
	* @param opacity: The opacity of the color (0-255)
	* @return Color team color
	*/
	static Color ConvertColorTeamToColor(CSI_EColorTeam colorEnum, int opacity = 255)
	{	
		Color colorFromInt;
		
		switch(colorEnum)
		{
			case CSI_EColorTeam.BLUE : 		colorFromInt = Color.FromInt(ARGB(opacity, 0, 92, 255)); 	break; // BLUE
			case CSI_EColorTeam.RED : 		colorFromInt = Color.FromInt(ARGB(opacity, 200, 65, 65)); 	break; // RED
			case CSI_EColorTeam.GREEN : 	colorFromInt = Color.FromInt(ARGB(opacity, 0, 190, 85)); 	break; // GREEN
			case CSI_EColorTeam.YELLOW : 	colorFromInt = Color.FromInt(ARGB(opacity, 230, 230, 0)); 	break; // YELLOW
			default : 					 	colorFromInt = Color.FromInt(ARGB(opacity, 165, 165, 165)); // NONE
		}
		
		return colorFromInt; 
	}

	//------------------------------------------------------------------------------------------------
	/**
	* Get icon string so we can set the icon through LoadImageFromSet
	* @param icon: Icon to find the string of
	* @param isSimpleIcon: Should we return the simple (non-circular) Icon
	* @return Icon string
	*/
	static string GetIconString(CSI_EIcon icon, bool isSimpleIcon = false)
	{
		string iconString;
		CSI_EIconTheme theme = CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.ICON_THEME);
		
		if (m_aNonThemedRegularIcons.Contains(icon) || m_aVehicleIcons.Contains(icon))
			iconString = string.Format("%1", SCR_Enum.GetEnumName(CSI_EIcon, icon));
		else	
			iconString = string.Format("%1 %2", SCR_Enum.GetEnumName(CSI_EIconTheme, theme), SCR_Enum.GetEnumName(CSI_EIcon, icon));

		if (isSimpleIcon)
			iconString = iconString + "_ICON";
		
		return iconString;
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	* Get players name, with a check if we need to add a players rank
	* @param playerID: The inputed player ID to pull the name of and rank of
	* @return Players name
	*/
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
	/**
	 * Search the widget hierarchy beginning at 'root' and collect icon widgets that match a given identifier.
	 * @param root: Root widget to start the search from.
	 * @param widgetString: Substring/pattern used to identify matching icon widgets (matched against widget names/identifiers).
	 * @param count: Maximum number of matches to collect; if non-positive, function will collect all matches.
	 * @return: An array of Widget handles for the matching icon widgets (empty if none found or if root is null).
	 */
	static array<Widget> GetAllIcons(Widget root, string widgetString, int count)
	{
		array<Widget> icons = {};
		
		if (!root || widgetString.IsEmpty() || count <= 0)
			return icons;
		
		for (int e = 0; e <= count; e++)
		{
			Widget icon = root.FindAnyWidget(widgetString + e.ToString());
			
			if (icon)
				icons.Insert(icon);
		};
		
		return icons;
	};
	
	//------------------------------------------------------------------------------------------------
	/**
	* Check the inputed text widget and add a "..." to the end of their name if the name is longer than the text widget
	* @param testWidget: widget to use to test a players name.
	* @param maxLength: Max length the name can bet.
	* @param name: Players name.
	* @return Players name with ellipsis at the end.
	*/
	static string CheckEllipsis(TextWidget testWidget, float maxLength, string name)
	{
		float sx = 0;
		float yx = 0;
		
		testWidget.SetText(name);
		testWidget.GetTextSize(sx, yx);

		if (sx > maxLength) 
		{
			for (int e = 0; sx > maxLength - 3.5; e++)
			{
				int nameLength = name.Length();
				nameLength = nameLength - 1;
				name = name.Substring(0, nameLength);

				testWidget.SetText(name);
				testWidget.GetTextSize(sx, yx);
			};
			name = string.Format("%1...", name);
		};
		return name;
	};
}