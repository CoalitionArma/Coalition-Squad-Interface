class COA_Compass : SCR_InfoDisplay
{
	private TextWidget Bearing = null;
	private ImageWidget Compass = null;
	private bool CompassVisible = true;
	private string masterStringStored = "N/A";
	private int groupRefresh = 44;

	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleCompass", EActionTrigger.DOWN, ToggleCompass);
	}

	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);

		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());

		//Refresh base widgets if we can't get them
		if (!Bearing || !Compass) {
			Bearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			Compass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
		};

		// Can't run if these dont exist better exit out.
		if (!character || !Bearing || !Compass) return;

		// Sets Bearings text and the Compass direction
		SetBearingSetCompass(Bearing, Compass);

		// Update Group Display every 45 frames.
		if (groupRefresh >= 45) {
			groupRefresh = 0;

			SCR_GroupsManagerComponent vanillaGroupManager = SCR_GroupsManagerComponent.GetInstance();
			COA_GroupDisplayManagerComponent groupManagerCOA = COA_GroupDisplayManagerComponent.GetInstance();
			if (!vanillaGroupManager) return;
		 
			SCR_AIGroup playersGroup = vanillaGroupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

			// If player doesn't have a group we can't do much, better check then clear their group display, just in case.
			if (!playersGroup) {ClearGroupDisplay(0, false); return; };
			
			array<int> PlayerIDsArray = playersGroup.GetPlayerIDs();
			int groupCount = PlayerIDsArray.Count();
			
			if (groupCount <= 1) {ClearGroupDisplay(0, false); return; };
			
			array<string> PlayerGroupString = {};
			
			foreach (int PlayerID : PlayerIDsArray) {
				string PlayerStr = groupManagerCOA.ReturnMapValue("PlayerGroupValues", PlayerID);
				if (!PlayerStr || PlayerStr == "") continue;
				PlayerGroupString.Insert(PlayerStr);
			}
			
			PlayerGroupString.Sort(false);
			
			foreach (int i, string PlayerString : PlayerGroupString) {
				
				array<string> removeValueArray = {};
				PlayerString.Split("|", removeValueArray, false);
				PlayerString = removeValueArray[1];
				
				array<string> localPlayerStringSplit = {};
				PlayerString.Split(";", localPlayerStringSplit, false);

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
			  		for (int e = 0; sx > 138.5; e++)
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
						case "None"   : {statusDisplay.SetColorInt(ARGB(255, 255, 255, 255)); playerDisplay.SetColorInt(ARGB(255, 255, 255, 255)); break; };
					};
				};
			};
			ClearGroupDisplay(groupCount, true);
		} else {
			groupRefresh++;
		};
	}

	//------------------------------------------------------------------------------------------------

	// Group Display Additional Functions

	//------------------------------------------------------------------------------------------------

	protected void ClearGroupDisplay(int positionToStartClearing, bool forceClear)
	{
		//Check if there's anything to clear
		ImageWidget DisplayCheck = ImageWidget.Cast(m_wRoot.FindAnyWidget("Status0"));
		int Check = DisplayCheck.GetOpacity();
		if (Check == 1 || forceClear) {
			for (int e = positionToStartClearing; e<=12; e++)
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
	};

	//------------------------------------------------------------------------------------------------

	// Compass Functions

	//------------------------------------------------------------------------------------------------

	protected void ToggleCompass()
	{
		CompassVisible = !CompassVisible;
		if (CompassVisible)
		{
			Compass.SetOpacity(1);
			Bearing.SetOpacity(1);
		}
		else
		{
			Compass.SetOpacity(0);
			Bearing.SetOpacity(0);
		}
	}

	protected void SetBearingSetCompass(TextWidget BearingRef, ImageWidget CompassRef)
	{
		// Get current world
		World world = GetGame().GetWorld();
		if (!world) return;

		// Settup transform
		vector transform[4];
		// Get current vector camera is looking at
		world.GetCurrentCamera(transform);

		// Creates rotation matrix from angles (yaw, pitch, roll in degrees)
		int yaw = Math3D.MatrixToAngles(transform)[0];
		// Returns absolute value of yaw
		if (yaw < 0) { yaw = 360 - Math.AbsFloat(yaw); }

		// Rotate Compass texture to current bearing direction (yaw)
		CompassRef.SetRotation(-yaw);

		// Set Bearing so if yaw outputs 6 it'll read 006
		string bearingAdd = "";
		// I love this.
		if (yaw >= 0 & yaw < 10) { bearingAdd = "00"; }
		if (yaw >= 10 & yaw < 100) { bearingAdd = "0"; }
		string bearingSTR = bearingAdd + (yaw.ToString());

		// Set Bearing Text
		BearingRef.SetText(bearingSTR);
	}
}
