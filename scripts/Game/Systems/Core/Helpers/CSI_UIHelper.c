class CSI_UIHelper
{	
	static ref array<CSI_EIcon> m_aNonThemedIcons =
	{
		// ----------- MAN -----------
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,

		// ----------- VEHICLE -----------
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
	* Get players name, with a check if we need to add a players rank
	* @param playerId: The inputed player ID to pull the name of and rank of
	* @return players name
	*/
	static string GetPlayersName(int playerId)
	{
		string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
		CSI_PlayerData playerData = CSI_AuthorityManager.GetInstance().GetPlayerData(playerId);
		
		if (!playerData)
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
	/**
	 * Gets a sorted list of player IDs sorted by their value to the group (how we group color teams, set TLs to the top of their color teams, etc)
	 * playersGroup: players group to pull the sorted group array of players
	 * @return The full list of player IDs
	 */
	static array<int> GetSortedGroupArray(SCR_AIGroup playersGroup)
	{
		array<int> playersGroupArray = {};
		array<string> tempLocalGroupArray = {};

		// Parse through current group array.
		foreach (int playerID : playersGroup.GetPlayerIDs())
		{
			int playerValue = CSI_AuthorityManager.GetInstance().GetPlayerData(playerID).GetPlayerValue();
			
			// Format a string with what we need for displaying/sorting a player.
			string playerStr = string.Format("%1;%2", playerValue, playerID);
			
			tempLocalGroupArray.Insert(playerStr);
		};

		tempLocalGroupArray.Sort(false);

		foreach (string playerStr : tempLocalGroupArray) 
		{
			array<string> outPlayerStrArray = {};
			playerStr.Split(";", outPlayerStrArray, false);
			
			playersGroupArray.Insert(outPlayerStrArray[1].ToInt());
		}
		
		return playersGroupArray;
	};
	
	//------------------------------------------------------------------------------------------------
	/**
	* Check the inputed text widget and add a "..." to the end of their name if the name is longer than the text widget
	* @param testWidget: widget to use to test a players name.
	* @param maxLength: max length the name can bet.
	* @param name: players name.
	* @return players name with a ellipsis at the end.
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
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Gets player ID from player name
	 * @param name The player name to search for
	 * @return The matching player ID or 0 if not found
	 */
	static int GetPlayerIdFromName(string name)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		foreach (int pid : playerIds)
		{
			if (GetGame().GetPlayerManager().GetPlayerName(pid) == name)
				return pid;
		}

		return 0;
	}
}