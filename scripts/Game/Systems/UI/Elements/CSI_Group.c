class CSI_GroupDisplay : SCR_ScriptedWidgetComponent
{
	protected CSI_ClientManager m_ClientComponent;
	protected CSI_AuthorityManager m_AuthorityComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	//------------------------------------------------------------------------------------------------
	void Update()
	{
		if (!m_AuthorityComponent || !m_ClientComponent || !m_GroupsManagerComponent) 
		{
			m_AuthorityComponent = CSI_AuthorityManager.GetInstance();
			m_ClientComponent = CSI_ClientManager.GetInstance();
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			return;
		};
		
		string groupDisplayVisible = m_ClientComponent.ReturnLocalCSISettings()[2];
		string rankVisible = m_ClientComponent.ReturnLocalCSISettings()[5];
		string hudAutoHidden = m_ClientComponent.ReturnLocalCSISettings()[14];
		
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
			
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, m_ClientComponent.SwitchStringToIcon(iconString));
			statusDisplay.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));
		};
		
		ClearGroupDisplay(groupArray.Count(), true);
	}

	//------------------------------------------------------------------------------------------------
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
}
