class CSI_DataHelper
{	
	static ref array<CSI_EIcon> m_aSpecialtyIcons =
	{
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,
		CSI_EIcon.ENG,
		CSI_EIcon.AT,
		CSI_EIcon.DEMO,
		CSI_EIcon.SNIPER,
		CSI_EIcon.GREN,
		CSI_EIcon.MG,
		CSI_EIcon.MEDIC
	};

	//------------------------------------------------------------------------------------------------
	static int DeterminePlayerValue(int groupID, int playerID)
	{
		// Setup value variable.
		int value = 0;
		
		CSI_EIcon icon = ReturnAuthorityPlayerMapValue(groupID, playerID, "SSI"); // SSI = StoredSpecialtyIcon
		CSI_EColorTeam colorTeam = ReturnAuthorityPlayerMapValue(groupID, playerID, "CT"); // CT = ColorTeam

		// Sort player by their color so we can group color teams together (a lil bit racist).
		switch (colorTeam) 
		{
			case CSI_EColorTeam.RED    : {value = -3; break;};
			case CSI_EColorTeam.BLUE   : {value = -5; break;};
			case CSI_EColorTeam.YELLOW : {value = -7; break;};
			case CSI_EColorTeam.GREEN  : {value = -9; break;};
			default : {value = 2;  break;};
		};

		switch (true) 
		{
			// If the players is currently the SL, make him the most valuable player in the list
			case (icon == CSI_EIcon.SL) : {value = -1; break;};

			// Add/Remove value from a player if they're a Team Lead
			case (icon == CSI_EIcon.TL && colorTeam == CSI_EColorTeam.NONE) : {value--;    break;};
			case (icon == CSI_EIcon.TL && colorTeam != CSI_EColorTeam.NONE) : {value++;    break;};
		};

		// Return how valuable the player is
		return value;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsSpecialtyIcon(CSI_EIcon icon)
	{
		return m_aSpecialtyIcons.Contains(icon);
	}
}