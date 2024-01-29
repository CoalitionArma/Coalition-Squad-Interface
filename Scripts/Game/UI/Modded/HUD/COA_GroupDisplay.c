class COA_GroupDisplay : SCR_InfoDisplay
{	
	private int m_iGroupDisplayRefresh;
	protected bool m_bGroupDisplayVisable = true;
	protected COA_GroupDisplayManagerComponent m_GroupDisplayManagerComponent = null;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleGroupDisplay", EActionTrigger.DOWN, ToggleGroupDisplay);
		GetGame().GetInputManager().AddActionListener("PlayerSelectionMenu", EActionTrigger.DOWN, PlayerSelectionMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		// Only update the group displpay every 15 frames.
		if (m_iGroupDisplayRefresh < 15) {
			m_iGroupDisplayRefresh++; 
			return;
		};
		
		m_iGroupDisplayRefresh = 0;
		
		if (!m_bGroupDisplayVisable) {
			ClearGroupDisplay(0, true);
			return;
		};
		
		if (!m_GroupDisplayManagerComponent) {
			m_GroupDisplayManagerComponent = COA_GroupDisplayManagerComponent.GetInstance();
			return;
		};
		 
		array<string> groupArray = m_GroupDisplayManagerComponent.GetLocalGroupArray();
		
		foreach (int i, string playerStringToSplit : groupArray) {
			
			array<string> playerSplitArray = {};
			playerStringToSplit.Split("╣", playerSplitArray, false);
			
			// Get all values we need to display this player.
			int playerID = playerSplitArray[1].ToInt();
			string colorTeam = playerSplitArray[2];
			string icon = playerSplitArray[3];

			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));
			
			// Check if we need to add ... to the end of players names.
			playerName = CheckEllipsis(135, playerDisplay, playerName);

			playerDisplay.SetColorInt(colorTeam.ToInt());
			playerDisplay.SetText(playerName);
			statusDisplay.SetColorInt(colorTeam.ToInt());
			statusDisplay.SetOpacity(1);
			statusDisplay.LoadImageTexture(0, icon);
		};
		ClearGroupDisplay(groupArray.Count(), true);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions for updating the group display
	
	//------------------------------------------------------------------------------------------------
	
	protected void ToggleGroupDisplay()
	{
		m_bGroupDisplayVisable = !m_bGroupDisplayVisable;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearGroupDisplay(int positionToStartClearing, bool forceClear)
	{
		//Check if there's anything to clear
		ImageWidget displayCheck = ImageWidget.Cast(m_wRoot.FindAnyWidget("Status0"));
		int check = displayCheck.GetOpacity();
		if (check == 1 || forceClear) {
			for (int e = positionToStartClearing; e <= 19; e++)
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
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.COA_PlayerSelectionDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	string CheckEllipsis(float maxLength, TextWidget nameWidget, string name)
	{	
		float sx = 0;
		float yx = 0;
		nameWidget.GetTextSize(sx, yx);
		
		if (sx > maxLength) {
			for (int e = 0; sx > maxLength - 3.5; e++)
			{
				int nameLength = name.Length();
				nameLength = nameLength - 1;
				name = name.Substring(0, nameLength);

				nameWidget.SetText(name);
				nameWidget.GetTextSize(sx, yx);
			};
			name = string.Format("%1...", name);
		};
		return name;
	};
}
