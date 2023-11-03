class COA_Compass : SCR_InfoDisplay
{
	private TextWidget Bearing = null;
	private ImageWidget Compass = null;
	private bool isVisible = true;
	private string groupStringStored = "N/A";
	private int groupRefresh = 30;
	
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
		
		// Get Charachter Controller of the locally played entity.
		SCR_CharacterControllerComponent m_cCharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		
		if (!m_cCharacterController ) return;

		// Update Group Display every 35 frames.
		if (groupRefresh >= 35) {	
			groupRefresh = 0;
			// Get Global Player Controller and Group Manager.
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
			
			// If for some reason the global player controller or group manager dont exist, better exit out. (and start praying to whatever god you beleve in)
			if (!playerController || !groupManager) return;
			
			SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerController.GetPlayerId());
			
			// If player doesn't have a group we can't do much, better check then clear their group display, just in case.
			if (!playersGroup) {ClearGroupDisplay(0, false); return;};
			
			string groupString = groupManager.ReturnGroupMapValue(playersGroup.ToString());
			
			if (!groupString || groupString == "") {ClearGroupDisplay(0, false); return;};
			
			if (groupString == groupStringStored) return;
			
			groupStringStored = groupString;
			
			array<string> localGroupSplitString = {};
			groupString.Split(";", localGroupSplitString, true);
			
			int groupCount = localGroupSplitString.Count();
			groupCount = groupCount/3;
			
			int playerPlace = 0;

			// Parse through current group array.
			for(int i = 0; i<groupCount && i < 12; i++) 
			{
				string playerName = localGroupSplitString[playerPlace];
				string colorTeam = localGroupSplitString[playerPlace + 1];
				string icon = localGroupSplitString[playerPlace + 2];
				
				playerPlace = playerPlace + 3;
			
				// Get group display widgets.
				TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
				ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));
				
				playerDisplay.SetText(playerName);
				statusDisplay.SetOpacity(1);
				statusDisplay.LoadImageTexture(0, icon);
				
				if (colorTeam) {
					switch (colorTeam) 
					{
						case "Red"    : {statusDisplay.SetColorInt(ARGB(255, 230, 0, 0));     playerDisplay.SetColorInt(ARGB(255, 230, 0, 0));     break;};
						case "Blue"   : {statusDisplay.SetColorInt(ARGB(255, 0, 92, 255));    playerDisplay.SetColorInt(ARGB(255, 0, 92, 255));    break;};
						case "Yellow" : {statusDisplay.SetColorInt(ARGB(255, 230, 230, 0));   playerDisplay.SetColorInt(ARGB(255, 230, 230, 0));   break;};
						case "Green"  : {statusDisplay.SetColorInt(ARGB(255, 0, 230, 0));     playerDisplay.SetColorInt(ARGB(255, 0, 230, 0));     break;};
						case "None"   : {statusDisplay.SetColorInt(ARGB(255, 255, 255, 255)); playerDisplay.SetColorInt(ARGB(255, 255, 255, 255)); break;};
					};
				};
			};	
			ClearGroupDisplay(groupCount, true);
		} else {
			groupRefresh++;
		};
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Group Display Additional Functions
	
	//---------------------------------------------------------------------------------------------------------------------------------
	
	protected void ClearGroupDisplay(int positionToStartClearing, bool forceClear)
	{
		//Check if there's anything to clear
		ImageWidget DisplayCheck = ImageWidget.Cast(m_wRoot.FindAnyWidget("Status0"));
		int Check = DisplayCheck.GetOpacity();
		if (Check == 1 || forceClear) {
			for(int e = positionToStartClearing; e<=18; e++)
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
	
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Compass Functions
	
	//---------------------------------------------------------------------------------------------------------------------------------
	
	protected void ToggleCompass()
	{
		isVisible = !isVisible;
		if(isVisible)
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
		if (yaw >= 0 & yaw < 10 ) { bearingAdd = "00"; }
		if (yaw >= 10 & yaw < 100) { bearingAdd = "0"; }
		string bearingSTR = bearingAdd + (yaw.ToString());
		
		// Set Bearing Text
		BearingRef.SetText(bearingSTR);
	}
}