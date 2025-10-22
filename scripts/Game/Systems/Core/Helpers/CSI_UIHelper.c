class CSI_UIHelper
{	
	//------------------------------------------------------------------------------------------------
	static int DeterminePlayerValue(int groupID, int playerID)
	{
		// Setup value variable.
		int value = 0;
		
		string icon = ReturnAuthorityPlayerMapValue(groupID, playerID, "SSI"); // SSI = StoredSpecialtyIcon
		string colorTeam = ReturnAuthorityPlayerMapValue(groupID, playerID, "CT"); // CT = ColorTeam

		// Sort player by their color so we can group color teams together (a lil bit racist).
		switch (colorTeam) 
		{
			case "R" : {value = -3; break;};
			case "B" : {value = -5; break;};
			case "Y" : {value = -7; break;};
			case "G" : {value = -9; break;};
			default  : {value = 2;  break;};
		};

		switch (true) 
		{
			// If the players is currently the SL, make him the most valuable player in the list
			case (icon == "SL")                        : {value = -1; break;};

			// Add/Remove value from a player if they're a Team Lead
			case (icon == "FTL" && colorTeam == "N/A") : {value--;    break;};
			case (icon == "FTL" && colorTeam != "N/A") : {value++;    break;};
		};

		// Return how valuable the player is
		return value;
	}
	
	static void AddToNextAvalibleGridSpace(GridLayoutWidget grid, Widget widget)
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
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