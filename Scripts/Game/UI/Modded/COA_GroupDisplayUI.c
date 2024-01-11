class COA_GroupDisplay : SCR_InfoDisplay
{	
	private int groupRefresh = 30;
	protected bool GroupDisplayVisable = true;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleGroupDisplay", EActionTrigger.DOWN, ToggleGroupDisplay);
		GetGame().GetInputManager().AddActionListener("PlayerSelectionMenu", EActionTrigger.DOWN, PlayerSelectionMenu);
	}
	
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		if (groupRefresh < 35) {
			groupRefresh++; 
			return;
		};
		
		groupRefresh = 0;
		
		if (!GroupDisplayVisable) {
			ClearGroupDisplay(0, true);
			return;
		};
		
		SCR_GroupsManagerComponent vanillaGroupManager = SCR_GroupsManagerComponent.GetInstance();
		COA_GroupDisplayManagerComponent groupManagerCOA = COA_GroupDisplayManagerComponent.GetInstance();
		if (!vanillaGroupManager) return;
		 
		SCR_AIGroup playersGroup = vanillaGroupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		// If player doesn't have a group we can't do much, better check then clear their group display, just in case.
		if (!playersGroup) {ClearGroupDisplay(0, false); return; };
			
		array<int> PlayerIDsArray = playersGroup.GetPlayerIDs();
		int groupCount = PlayerIDsArray.Count();
			
		if (groupCount <= 1) {ClearGroupDisplay(0, false); return;};
			
		array<string> PlayerGroupStringArray = {};
			
		foreach (int PlayerID : PlayerIDsArray) {
			IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(PlayerID);
			if (!localplayer) continue;
			string PlayerStr = groupManagerCOA.ReturnLocalPlayerMapValue("PlayerGroupValues", PlayerID);
			if (!PlayerStr || PlayerStr == "") continue;
			PlayerGroupStringArray.Insert(PlayerStr);
		}
			
		PlayerGroupStringArray.Sort(false);
			
		foreach (int i, string PlayerString : PlayerGroupStringArray) {
				
			array<string> removeValueArray = {};
			PlayerString.Split("╣", removeValueArray, true);
			PlayerString = removeValueArray[1];
				
			array<string> localPlayerStringSplit = {};
			PlayerString.Split("║", localPlayerStringSplit, false);

			string playerName = localPlayerStringSplit[0];
			string colorTeam  = localPlayerStringSplit[1];
			string icon       = localPlayerStringSplit[2];

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));

			playerDisplay.SetText(playerName);
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, icon);
				
			float sx = 0;
			float yx = 0;
			playerDisplay.GetTextSize(sx, yx);
			
			if (sx > 145) {
			  	for (int e = 0; sx > 132.5; e++)
				{
					int playerNameLength = playerName.Length();
					playerNameLength = playerNameLength - 1;
					playerName = playerName.Substring(0, playerNameLength);
						
					//Players display wont visually update until the UpdateValues function ends, so they don't see this.
					playerDisplay.SetText(playerName);
					playerDisplay.GetTextSize(sx, yx);
				};
				playerName = string.Format("%1...", playerName);
				playerDisplay.SetText(playerName);
			};

			if (colorTeam) {
				switch (colorTeam)
				{
					case "Red"    : {statusDisplay.SetColorInt(ARGB(255, 200, 65, 65));   playerDisplay.SetColorInt(ARGB(255, 200, 65, 65));   break; };
					case "Blue"   : {statusDisplay.SetColorInt(ARGB(255, 0, 92, 255));    playerDisplay.SetColorInt(ARGB(255, 0, 92, 255));    break; };
					case "Yellow" : {statusDisplay.SetColorInt(ARGB(255, 230, 230, 0));   playerDisplay.SetColorInt(ARGB(255, 230, 230, 0));   break; };
					case "Green"  : {statusDisplay.SetColorInt(ARGB(255, 0, 190, 85));    playerDisplay.SetColorInt(ARGB(255, 0, 190, 85));    break; };
					case "None"   : {statusDisplay.SetColorInt(ARGB(235, 235, 235, 255)); playerDisplay.SetColorInt(ARGB(235, 235, 235, 255)); break; };
				};
			};
		};
		ClearGroupDisplay(groupCount, true);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions for updating the group display
	
	//------------------------------------------------------------------------------------------------
	
	protected void ToggleGroupDisplay()
	{
		GroupDisplayVisable = !GroupDisplayVisable;
	}

	void ClearGroupDisplay(int positionToStartClearing, bool forceClear)
	{
		//Check if there's anything to clear
		ImageWidget DisplayCheck = ImageWidget.Cast(m_wRoot.FindAnyWidget("Status0"));
		int Check = DisplayCheck.GetOpacity();
		if (Check == 1 || forceClear) {
			for (int e = positionToStartClearing; e<=19; e++)
			{
				// Get group display widgets.
				TextWidget playerRemoveDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", e)));
				ImageWidget statusRemoveDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", e)));

				// Skip ahead to next for-loop iteration if either of these are false.
				if (!playerRemoveDisplay || !statusRemoveDisplay) continue;

				// Clear widgets.
				playerRemoveDisplay.SetText("");
				statusRemoveDisplay.SetOpacity(0);
			}
		};
	}
	
	//------------------------------------------------------------------------------------------------

	// Additionals
	
	//------------------------------------------------------------------------------------------------
		
	protected void PlayerSelectionMenu()
	{	
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.COA_PlayerSelectionUI);
	}
}
