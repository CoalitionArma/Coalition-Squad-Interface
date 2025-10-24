class CSI_DataHelper
{	
	//------------------------------------------------------------------------------------------------
	/**
	* This is a simple way of determining a players value for listing them on the group screen.
	* This is so we can list players by color team together and certain roles (SL's and TL's, etc get listed at the top).
	* @param playerData The player data of the player you want to return a value on
	* @return the players current value, should exclusively be used in group displays/Z order of the compass icons.
	*/
	static int DeterminePlayerValue(CSI_PlayerData playerData)
	{
		// Setup value variable.
		int value = 0;
		
		bool isSL // poll group manager and check if the inputed player is a squad lead.
		bool isTL = playerData.GetIsTeamLeader();
		CSI_EColorTeam colorTeam = playerData.GetColorTeam();

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
			case (isTL && colorTeam == CSI_EColorTeam.NONE) : {value--;    break;};
			case (isTL && colorTeam != CSI_EColorTeam.NONE) : {value++;    break;};
		};

		// Return how valuable the player is
		return value;
	}
}