class COAL_GUI: SCR_InfoDisplay
{
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Variables used on/for UI Components
	
	//---------------------------------------------------------------------------------------------------------------------------------
	private TextWidget Bearing = null;
	private ImageWidget Compass = null;
	private ProgressBarWidget StamBar = null;
	private bool stamBarVisible = false;
	private int groupRefresh = 45;
	private int widgetRefresh = 360;
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Main loop that repeats every frame
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		
		super.UpdateValues(owner, timeSlice);
		
		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		//Refresh base widgets every 360 frames
		if (widgetRefresh >= 360) {
			widgetRefresh = 0;
			Bearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			Compass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
			StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
		} else {
			widgetRefresh++;
		};
		
		// Can't run if these dont exist better exit out.
		if (!character || !Bearing || !Compass || !StamBar) return;
	
		// Sets Bearings text and the Compass direction
		SetBearingSetCompass(Bearing, Compass);
		
		// Get Charachter Controller of the locally played entity.
		SCR_CharacterControllerComponent m_cCharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		
		if (!m_cCharacterController ) return;
		
		// Use local Charachter Controller to get the current players stamina, then use custom function OnStaminaChange() to show current stamina on players screen.
        OnStaminaChange(m_cCharacterController.GetStamina(), StamBar);
		
		// Update Group Display every 45 frames. (May lower to 30 depending on feedback from players with lower end PCs)
		if (groupRefresh >= 45) {
				
			groupRefresh = 0;
			
			// Get Global Player Controller and Group Manager.
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
			
			// If for some reason the global player controller or group manager dont exist, better exit out. (and start praying to whatever god you beleve in)
			if (!playerController || !groupManager) return;
			
			SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerController.GetPlayerId());
			
			// If player doesn't have a group we can't do much, better check then clear their group display, just in case.
			if (!playersGroup) {
				ClearGroupDisplay(0, false);
				return;
			};
			
			// Get list of all the players we have to parse through.
			array<int> groupPlayersIDs = playersGroup.GetPlayerIDs();
			//array<int> groupPlayersIDs = {1,1,1,1,1,1}; //Debugging
			
			// Count how many players we plan on parsing through.
			int groupCount = groupPlayersIDs.Count();
			
			//If person is only one in group, check then clear the group display.
			if (groupCount == 1 || groupCount == 0) {
				ClearGroupDisplay(0, false);
				return;
			};
			
			// Get current group leader.
			int groupLeaderID = playersGroup.GetLeaderID();
			
			// Parse through groupPlayersIDs and show players names and icons on the group display.
			GroupDisplay(groupLeaderID, groupCount, groupPlayersIDs);
		} else {
			groupRefresh++;
		};
	}
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Functions used for the UI
	
	//---------------------------------------------------------------------------------------------------------------------------------
	protected void GroupDisplay(int groupLeaderID, int groupCount, array<int> groupPlayersIDs)
	{	
		// Cannot guarantee that the player at 0 in groupPlayersIDs will always be the squad leader, so we find him in the group array and force him there.
		int leaderIndex = groupPlayersIDs.Find(groupLeaderID);
		groupPlayersIDs.Remove(leaderIndex);
		
		// Sort the array from player IDs. (typically comes out as the first to last players on the squad to connect)
		groupPlayersIDs.Sort(true);
		
		// Push the Squad leader to the first Icon.
		groupPlayersIDs.InsertAt(groupLeaderID, 0);
		
		// Parse through current group array.
		for(int i = 0; i<groupCount && i < 18; i++) 
		{
			// Get Target Players ID.
			int localPlayerID = groupPlayersIDs[i];
			
			// Get target players entity and player name.
			IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerID);
			string playerName = GetGame().GetPlayerManager().GetPlayerName(localPlayerID);
			
			// Get group display widgets.
			TextWidget playerDisplay = TextWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Player%1", i)));
			ImageWidget statusDisplay = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("Status%1", i)));
			
			// Skip ahead to next for-loop iteration if any of whese are false.
			if (!statusDisplay || !playerDisplay || !playerName || !localplayer) continue;
			
			// Set player text.
			playerDisplay.SetText(playerName);
			
			//---------------------------------------------------------------------------------------------------------------------------------
			// Color Teams
			//---------------------------------------------------------------------------------------------------------------------------------
			
			//statusDisplay.SetColorInt(ARGB(255, 255, 160, 160))
			
			//---------------------------------------------------------------------------------------------------------------------------------
			// Vehicle Icons, they supercede any specialty icons
			//---------------------------------------------------------------------------------------------------------------------------------
			
			// Check if target player is even in a vehicle.
			CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(localplayer.FindComponent(CompartmentAccessComponent));
			if (compartmentAccess)
			{
				// Check target players current compartment.
			   BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();  
			    if (compartment)
			    {
					// Check target players current compartment type, then assign his icon.
			        ECompartmentType compartmentType = SCR_CompartmentAccessComponent.GetCompartmentType(compartment);
					switch (compartmentType)
					{
						// Passanger/Commander 
						// ToDo: impliment Commander compartment detection and icon when/if it's added to the game
						case ECompartmentType.Cargo : {
							statusDisplay.SetOpacity(1);
							statusDisplay.LoadImageTexture(0, "{B910A93F355F168C}Layouts\UI\Textures\Icons\imagecargo_ca.edds");
							break;
						}
						// Driver
						case ECompartmentType.Pilot : {
							statusDisplay.SetOpacity(1);
							statusDisplay.LoadImageTexture(0, "{C2B2F451FB157A89}Layouts\UI\Textures\Icons\imagedriver_ca.edds");
							break;
						}
						// Gunner
						case ECompartmentType.Turret : {
							statusDisplay.SetOpacity(1);
							statusDisplay.LoadImageTexture(0, "{3DAAB773C8C29812}Layouts\UI\Textures\Icons\imagegunner_ca.edds");
							break;
						}
					}
					// Skip ahead to next for-loop iteration since we've done our job of setting the target players icon.
					continue;
			    };
			};

			//---------------------------------------------------------------------------------------------------------------------------------
			//	Specialty Icons
			//---------------------------------------------------------------------------------------------------------------------------------
			
			// Check if current target player is the current squad leader AND we're at the correct position.
			if (localPlayerID == groupLeaderID && i == 0) 
			{
				// Set Squad Leader Icon
				statusDisplay.SetOpacity(1);
				statusDisplay.LoadImageTexture(0, "{5ECE094ED4662B33}Layouts\UI\Textures\Icons\iconmanleader_ca.edds");
				// Skip ahead to next for-loop iteration since we've done our job of setting the target players icon.
				continue;
			};
			
			// Get target players inventory component
			SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(localplayer.FindComponent(SCR_InventoryStorageManagerComponent));
			
			// Get all of target players inventory items
			array<IEntity> allPlayerItems = {};
			characterInventory.GetAllRootItems(allPlayerItems);
			
			// Settup new arrays and variables
			//
			array<EWeaponType> WeaponTypeArray = new array<EWeaponType>();
			array<SCR_EConsumableType> MedicalTypeArray = new array<SCR_EConsumableType>();
			array<BaseMuzzleComponent> muzzles = {};
			EntityPrefabData prefabData;
			string resourceName;
			
			// Parse through players entire inventory.
			foreach (IEntity item : allPlayerItems)
			{			
				prefabData = item.GetPrefabData();
					
				// Ignore if no prefab data.
				if (!prefabData) continue;
					
				resourceName = prefabData.GetPrefabName();
				
				// Ignore if no prefab resource name.
				if (resourceName.IsEmpty()) continue;
				
				// Check if item is consumable (nearly always a medical item).
				SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
				if (consumable)
				{
					// Check items type.
					SCR_EConsumableType medicalType = SCR_ConsumableItemComponent.Cast(consumable).GetConsumableType();
					if (medicalType == SCR_EConsumableType.SALINE)
					{
						// Insert the valid item into the medical array so we can read it later.
						MedicalTypeArray.Insert(medicalType);
					};
				};
				// Check if item is a Weapon.
				WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
				if (weaponComp) {
					
					// Get the weapons type and insert it into the weapon array so we can read it later.
					WeaponTypeArray.Insert(weaponComp.GetWeaponType());
					
					// Get muzzle types (so we can detect something like a underslung grenade launcher)
					for (int m = 0, mCount = weaponComp.GetMuzzlesList(muzzles); m < mCount; m++)
					{
						// Convert muzzle types to weapon types and insert it into the weapon array so we can read it later. (ToDo: Not hardcoded?)
						switch (muzzles[m].GetMuzzleType())
						{
							case EMuzzleType.MT_RPGMuzzle : {
								WeaponTypeArray.Insert(EWeaponType.WT_ROCKETLAUNCHER);
								break;
							};
							case EMuzzleType.MT_UGLMuzzle : {
								WeaponTypeArray.Insert(EWeaponType.WT_GRENADELAUNCHER);
								break;
							};
						}
					}
				};
			};
			// Take all the data we just collected and assign players a icon based on if it exists in the weapon/medical arrays.
			switch (true) 
			{
				// Medic
				case (MedicalTypeArray.Find(SCR_EConsumableType.SALINE) != -1) : {
					statusDisplay.SetOpacity(1);
					statusDisplay.LoadImageTexture(0, "{01F2523A4EE5C48B}Layouts\UI\Textures\Icons\iconmanmedic_ca.edds");
					break;
				};
				// Sniper
				case (WeaponTypeArray.Find(EWeaponType.WT_SNIPERRIFLE) != -1) : {
					statusDisplay.SetOpacity(1);
					statusDisplay.LoadImageTexture(0, "{318B797C57BE3C29}Layouts\UI\Textures\Icons\iconmansniper_ca.edds");
					break;
				};
				// MG	
				case (WeaponTypeArray.Find(EWeaponType.WT_MACHINEGUN) != -1) : {
					statusDisplay.SetOpacity(1);
					statusDisplay.LoadImageTexture(0, "{CCF40410BDB53870}Layouts\UI\Textures\Icons\iconmanmg_ca.edds");
					break;
				};
				// RAT/MAT			
				case (WeaponTypeArray.Find(EWeaponType.WT_ROCKETLAUNCHER) != -1) : {
					statusDisplay.SetOpacity(1);
					statusDisplay.LoadImageTexture(0, "{DC86195B44F5A345}Layouts\UI\Textures\Icons\iconmanat_ca.edds");
					break;
				};
				// GL
				case (WeaponTypeArray.Find(EWeaponType.WT_GRENADELAUNCHER) != -1) : {
					statusDisplay.SetOpacity(1);
					statusDisplay.LoadImageTexture(0, "{B7757F2024A3DC87}Layouts\UI\Textures\Icons\iconmangrenadier_ca.edds");
					break;
				};
				// No One Special (Loser)
				default : {
					statusDisplay.SetOpacity(1);
					statusDisplay.LoadImageTexture(0, "{71ED761DF5BA041C}Layouts\UI\Textures\Icons\iconman_ca.edds");
				};
			};
		}
		// Clear the group display bellow what we just populated, getting rid of any defunct player names/icons.
		ClearGroupDisplay(groupCount, true);
	}
	
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
	
	// Slowly fades the opacity of the bar
	// TO-DO: Implement this
	void fadeBar(ProgressBarWidget widget, float goToOpacity, int time) 
	{
		float currentOpacity = widget.GetOpacity();
		float change = (currentOpacity + goToOpacity) / time; //fix
		
		for(int i = 0; i < time; i++) 
		{
			widget.SetOpacity(change);
			//Print("Opacity: " + widget.GetOpacity());
			Sleep(1000); //sleep 1s
		}
	}
	
	void revealBar(ProgressBarWidget widget) 
	{
		stamBarVisible = true;
		widget.SetOpacity(0.5);
		//fadeBar(StamBarRef, 0.5, 2);
	}
	
	protected void hideBar(ProgressBarWidget widget) 
	{
		stamBarVisible = false;
		widget.SetOpacity(0);
		//fadeBar(StamBarRef, 0, 2);
	}
	
	void OnStaminaChange(float value, ProgressBarWidget StamBarRef)
    {
		StamBarRef.SetCurrent(value);
		//Print("Stamina: " + value);
		//Print("stamBarVisible: " + stamBarVisible);
		
		// Color
		// I couldnt get the switch statement to work. I hate this.
		if (value < 0.7 && value > 0.3)
		{
			StamBarRef.SetColor(new Color(0.091997,0.083009,0.035996,1.000000));
		} else if (value < 0.3) 
		{
			StamBarRef.SetColor(new Color(0.188724,0.046860,0.037476,1.000000));
		} else // Aka above 70%
		{
			StamBarRef.SetColor(new Color(0.035996,0.091997,0.050004,1.000000));
		}
		
		// Opacity
		if (value < 1 && !stamBarVisible) 
		{
			revealBar(StamBarRef);
		} else if (value == 1 && stamBarVisible) 
		{
			hideBar(StamBarRef);
		}
    }
};

/*
modded enum ChimeraMenuPreset {
    ColorTeamSelection
}


class COAL_GUI_ColorTeamSelection: ChimeraMenuBase
{
	
	
};
*/



