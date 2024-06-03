class CSI_GroupDisplay : SCR_InfoDisplay
{
	protected CSI_ClientComponent m_ClientComponent;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	
	protected int m_iCurrentFrame = 35;

	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		if (m_iCurrentFrame < 35) 
		{
			m_iCurrentFrame++;
			return;
		};

		m_iCurrentFrame = 0;

		if (!m_AuthorityComponent || !m_ClientComponent || !m_GroupsManagerComponent) 
		{
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			m_ClientComponent = CSI_ClientComponent.GetInstance();
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			return;
		};
		
		string groupDisplayVisible = m_ClientComponent.ReturnLocalCSISettings()[2];
		string rankVisible = m_ClientComponent.ReturnLocalCSISettings()[5];
		
		array<string> groupArray = m_ClientComponent.GetLocalGroupArray();
		
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		if (groupDisplayVisible == "false" || !groupArray || groupArray.Count() <= 1 || !playersGroup) 
		{
			ClearGroupDisplay(0, true);
			return;
		};

		foreach (int i, string playerStringToSplit : groupArray) {
			array<string> playerSplitArray = {};
			playerStringToSplit.Split(":", playerSplitArray, false);

			// Get all values we need to display this player.
			int playerID = playerSplitArray[1].ToInt();
			string colorTeamString = m_AuthorityComponent.ReturnLocalPlayerMapValue(playersGroup.GetGroupID(), playerID, "CT"); // CT = ColorTeam
			string iconString = m_AuthorityComponent.ReturnLocalPlayerMapValue(playersGroup.GetGroupID(), playerID, "DI"); // DI = DisplayIcon

			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
			
			if (playerName.IsEmpty() || iconString.IsEmpty()) 
				return;

			if (rankVisible == "true") 
			{
				string rank = m_AuthorityComponent.ReturnLocalPlayerMapValue(-1, playerID, "PR"); // PR = PlayerRank
				if (!rank.IsEmpty()) 
					playerName = string.Format("%1 %2", rank, playerName);
			};

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));
			
			if (!playerDisplay || !statusDisplay) 
				continue;

			// Check if we need to add ... to the end of players names.
			playerName = CheckEllipsis(106, playerName);

			playerDisplay.SetText(playerName);
			playerDisplay.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));
			
			if (iconString == "DRV" || iconString == "PAX") 
			{
				FrameSlot.SetSize(statusDisplay, 17, 17);	
				switch (true) {
					case (i >= 0 && i <= 4)   : {FrameSlot.SetPosX(statusDisplay, 88.6);  break;};
					case (i >= 5 && i <= 9)   : {FrameSlot.SetPosX(statusDisplay, 218.8); break;};
					case (i >= 10 && i <= 14) : {FrameSlot.SetPosX(statusDisplay, 349.0); break;};
					case (i >= 15 && i <= 19) : {FrameSlot.SetPosX(statusDisplay, 478.6); break;};
					case (i >= 20 && i <= 24) : {FrameSlot.SetPosX(statusDisplay, 608.6); break;};
				};
			} else {
				FrameSlot.SetSize(statusDisplay, 23.2, 23.2);
				switch (true) {
					case (i >= 0 && i <= 4)   : {FrameSlot.SetPosX(statusDisplay, 85.6125);  break;};
					case (i >= 5 && i <= 9)   : {FrameSlot.SetPosX(statusDisplay, 216.0125);   break;};
					case (i >= 10 && i <= 14) : {FrameSlot.SetPosX(statusDisplay, 345.6125); break;};
					case (i >= 15 && i <= 19) : {FrameSlot.SetPosX(statusDisplay, 475.6125); break;};
					case (i >= 20 && i <= 24) : {FrameSlot.SetPosX(statusDisplay, 605.6125); break;};
				};
			};
			
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, m_ClientComponent.SwitchStringToIcon(iconString));
			statusDisplay.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));
		};
		ClearGroupDisplay(groupArray.Count(), true);
	}

	protected void ClearGroupDisplay(int positionToStartClearing, bool forceClear)
	{
		//Check if there's anything to clear
		ImageWidget displayCheck = ImageWidget.Cast(m_wRoot.FindAnyWidget("Status0"));
		int check = displayCheck.GetOpacity();
		if (check == 1 || forceClear) 
		{
			for (int e = positionToStartClearing; e <= 24; e++)
			{
				// Get group display widgets.
				TextWidget playerRemoveDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", e)));
				ImageWidget statusRemoveDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", e)));

				// Skip ahead to next for-loop iteration if either of these are false.
				if (!playerRemoveDisplay || !statusRemoveDisplay) 
					continue;

				// Clear widgets.
				playerRemoveDisplay.SetText("");
				statusRemoveDisplay.SetOpacity(0);
			}
		};
	}

	string CheckEllipsis(float maxLength, string name)
	{
		float sx = 0;
		float yx = 0;
		
		TextWidget testWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("TestPlayerName"));
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
