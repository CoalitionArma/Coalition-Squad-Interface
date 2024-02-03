class CSI_GroupDisplay : SCR_InfoDisplay
{	
	protected int m_iGroupDisplayVisible;
	protected int m_iRankVisible;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	
	protected int m_iCheckSettingsFrame = 65;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().ActivateContext("CoalitionSquadInterfaceContext", 36000000);
		GetGame().GetInputManager().AddActionListener("CSISettingsMenu", EActionTrigger.DOWN, ToggleCSISettingsMenu);
		GetGame().GetInputManager().AddActionListener("PlayerSelectionMenu", EActionTrigger.DOWN, TogglePlayerSelectionMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		if (!m_AuthorityComponent) {
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			return;
		};
		
		if (m_iCheckSettingsFrame >= 65) {
			m_iCheckSettingsFrame = 0;
			int groupDisplayVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[2];
			switch (groupDisplayVisibleServerOverride)
			{
				case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("groupDisplayVisible", m_iGroupDisplayVisible); break;};
				case (0) : { m_iGroupDisplayVisible = 0; break;};
				case (1) : { m_iGroupDisplayVisible = 1; break;};
			};
			
			int rankVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[5];
			switch (rankVisibleServerOverride)
			{
				case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("rankVisible", m_iRankVisible); break;};
				case (0)  : { m_iRankVisible = 0; break;};
				case (1)  : { m_iRankVisible = 1; break;};
			};
		} else { m_iCheckSettingsFrame++; };
		
		if (m_iGroupDisplayVisible == 0) {
			ClearGroupDisplay(0, true);
			return;
		};
		 
		array<string> groupArray = m_AuthorityComponent.GetLocalGroupArray();
		
		foreach (int i, string playerStringToSplit : groupArray) {
			
			array<string> playerSplitArray = {};
			playerStringToSplit.Split("╣", playerSplitArray, false);
			
			// Get all values we need to display this player.
			int playerID = playerSplitArray[1].ToInt();
			string colorTeam = playerSplitArray[2];
			string icon = playerSplitArray[3];

			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
			
			if (m_iRankVisible == 1) {
				string rank = SCR_CharacterRankComponent.GetCharacterRankNameShort(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
				playerName = string.Format("[%1] %2", rank, playerName);
			}

			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));
			
			// Check if we need to add ... to the end of players names.
			playerName = CheckEllipsis(110, playerDisplay, playerName);

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

	protected void ClearGroupDisplay(int positionToStartClearing, bool forceClear)
	{
		//Check if there's anything to clear
		ImageWidget displayCheck = ImageWidget.Cast(m_wRoot.FindAnyWidget("Status0"));
		int check = displayCheck.GetOpacity();
		if (check == 1 || forceClear) {
			for (int e = positionToStartClearing; e <= 24; e++)
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
		
	protected void TogglePlayerSelectionMenu()
	{	
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSelectionDialog);
	}
	
	protected void ToggleCSISettingsMenu()
	{	
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_SettingsDialog);
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
