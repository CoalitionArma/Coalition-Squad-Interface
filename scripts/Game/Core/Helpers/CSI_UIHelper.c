class CSI_UIHelper
{	
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

	// All Color Teams
	static int m_iCTNone   = ARGB(255, 165, 165, 165);
	static int m_iCTRed    = ARGB(255, 200, 65, 65);
	static int m_iCTBlue   = ARGB(255, 0, 92, 255);
	static int m_iCTYellow = ARGB(255, 230, 230, 0);
	static int m_iCTGreen  = ARGB(255, 0, 190, 85);
	
	//------------------------------------------------------------------------------------------------
	/**
	* Convert the color team enum to a color
	* @param colorEnum: The color enum to convert into a color
	* @return color team color
	*/
	static Color ConvertColorTeamToColor(CSI_EColorTeam colorEnum)
	{	
		Color colorFromInt;
		
		switch(colorEnum)
		{
			case CSI_EColorTeam.NONE : colorFromInt = Color.FromInt(m_iCTNone); break;
			case CSI_EColorTeam.BLUE : colorFromInt = Color.FromInt(m_iCTBlue); break;
			case CSI_EColorTeam.RED : colorFromInt = Color.FromInt(m_iCTRed); break;
			case CSI_EColorTeam.GREEN : colorFromInt = Color.FromInt(m_iCTGreen); break;
			case CSI_EColorTeam.YELLOW : colorFromInt = Color.FromInt(m_iCTYellow); break;
		}
		
		return colorFromInt; 
	}

	//------------------------------------------------------------------------------------------------
	/**
	* Get icon string so we can set the icon through LoadImageFromSet
	* @param icon: Icon to find the string of
	* @param isSimpleIcon: should we return the simple (non-circular) Icon
	* @return icon string
	*/
	static string GetIconString(CSI_EIcon icon, bool isSimpleIcon = false)
	{
		string iconString;
		CSI_EIconTheme theme = CSI_SettingsManager.GetInstance().GetCSISettingInt(CSI_SettingsManager.ICON_THEME);
		
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
	* @param playerId: The inputed player ID to pull the name of and rank of
	* @return players name
	*/
	static string GetPlayersName(int playerId)
	{
		string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
		CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(playerId);
		
		if (!playerData || !CSI_SettingsManager.GetInstance().GetCSISettingBool(CSI_SettingsManager.RANK_VISIBLE))
			return name;
		
		SCR_ECharacterRank rankEnum = playerData.GetRank();
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerId));
		
		if (!faction)
			return name;
		
		string rank = faction.GetRankName(rankEnum);
		
		if (rank.IsEmpty())
			return name;
		
		return string.Format("%1 %2", rank, name);
	}
	
	//------------------------------------------------------------------------------------------------
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
	* @param maxLength: max length the name can bet.
	* @param name: players name.
	* @return players name with ellipsis at the end.
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