class COAL_GUI: SCR_InfoDisplay
{
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Variables used on/for UI Components
	
	//---------------------------------------------------------------------------------------------------------------------------------
	protected TextWidget m_wBearing;
	protected TextWidget Player0;
	protected TextWidget Player1;
	protected TextWidget Player2;
	protected TextWidget Player3;
	protected TextWidget Player4;
	protected TextWidget Player5;
	protected TextWidget Player6;
	protected TextWidget Player7;
	protected TextWidget Player8;
	protected TextWidget Player9;
	protected TextWidget Player10;
	protected TextWidget Player11;
	protected ImageWidget m_wCompass;
	protected ImageWidget Status0;
	protected ImageWidget Status1;
	protected ImageWidget Status2;
	protected ImageWidget Status3;
	protected ImageWidget Status4;
	protected ImageWidget Status5;
	protected ImageWidget Status6;
	protected ImageWidget Status7;
	protected ImageWidget Status8;
	protected ImageWidget Status9;
	protected ImageWidget Status10;
	protected ImageWidget Status11;
	TextWidget tArray[] = {Player1,Player2,Player3,Player4,Player5,Player6,Player7,Player8,Player9,Player10,Player11};
	ImageWidget iArray[] = {Status1,Status2,Status3,Status4,Status5,Status6,Status7,Status8,Status9,Status10,Status11};
	private ref array<EEditableEntityLabel> m_RoleLabels = {};
	private ProgressBarWidget StamBar = null;
	private SCR_CharacterControllerComponent m_cCharacterController = null;
	private bool stamBarVisible = false;
	static Widget s_wDebugLayout;
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// When player is created
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		// Re-aquire UI widgets if they dont exist at the moment.
		if (!m_wBearing) m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
		if (!m_wCompass) m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
		if (!StamBar) StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
    }
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Main loop that repeats every frame
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		// Get local enity the player is controlling at the moment.
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		// Can't run shit if these dont exist better exit out quick.
		if (!character || !m_wBearing || !m_wCompass) return;
	
		// Passes m_wBearing & m_wCompass onto the custom function SetBearingSetCompass().
		SetBearingSetCompass(m_wBearing, m_wCompass);
		
		// Get Charachter Controller for locally played entity.
		 m_cCharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		
		if (!m_cCharacterController ) return;
		// Use local Charachter Controller to get the current entity stamina, then use custom function OnStaminaChange() to show current stamina on players screen.
        OnStaminaChange(m_cCharacterController.GetStamina());
		
		// Get local played entitys PlayerID.
		//int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		
		//GroupDisplay(owner, playerID);

	}
	//---------------------------------------------------------------------------------------------------------------------------------
	
	// Functions used for the UI
	
	//---------------------------------------------------------------------------------------------------------------------------------

	void GroupDisplay(IEntity owner, int playerID)
	{
		bool playerInGroup = SCR_AIGroup.Cast(owner).IsPlayerInGroup(playerID);
		if (!playerInGroup) return;
		
		int groupCount = SCR_AIGroup.Cast(owner).GetAgentsCount();
		array<int> groupPlayersID = SCR_AIGroup.Cast(owner).GetPlayerIDs();
		int groupLeaderID = SCR_AIGroup.Cast(owner).GetLeaderID();
		
		PlayerManager pMan = GetGame().GetPlayerManager();
		
		if (!Player0) Player0 = TextWidget.Cast(m_wRoot.FindAnyWidget("Player0"));
		if (!Status0) Status0 = ImageWidget.Cast(m_wRoot.FindAnyWidget("Status0"));
		string groupLeaderName = pMan.GetPlayerName(groupLeaderID);
		Player0.SetText(groupLeaderName);
		
		for(int i = 0; i<=groupCount; i++) 
		{
			int localGroupPlayerID = groupPlayersID[i];
			
			ImageWidget statusDisplay = iArray[i];
			TextWidget playerNameDisplay = tArray[i];
			
			string playerName = pMan.GetPlayerName(localGroupPlayerID);
			IEntity playerEntity = pMan.GetPlayerControlledEntity(localGroupPlayerID); 
			
			playerNameDisplay.SetText(playerName);
			
			m_ItemResource = Resource.Load(playerEntity.GetItemResourceName());
            IEntitySource entitySource = m_ItemResource.GetResource().ToEntitySource();
			
			//IEntitySource entitySource = playerEntity.GetResource().ToEntitySource();
			
			EWeaponType playerWeaponType = CheckWeapons(entitySource);
			
			switch (playerWeaponType)
			{
				case EWeaponType.WT_RIFLE : 
				{
					m_RoleLabels.Insert(EEditableEntityLabel.ROLE_RIFLEMAN);
					break;
				}
				case EWeaponType.WT_MACHINEGUN : 
				{
					m_RoleLabels.Insert(EEditableEntityLabel.ROLE_MACHINEGUNNER);
					break;
				}
				case EWeaponType.WT_ROCKETLAUNCHER : 
				{
					m_RoleLabels.Insert(EEditableEntityLabel.ROLE_ANTITANK);
					break;
				}
				case EWeaponType.WT_GRENADELAUNCHER : 
				{
					m_RoleLabels.Insert(EEditableEntityLabel.ROLE_GRENADIER);
					break;
				}
				case EWeaponType.WT_SNIPERRIFLE : 
				{
					m_RoleLabels.Insert(EEditableEntityLabel.ROLE_SHARPSHOOTER);
					break;
				}
				default: 
				{
					break;
				}
			}
		}
	}
	
	EWeaponType CheckWeapons(IEntitySource entitySource)
	{
		array<ref array<IEntityComponentSource>> weaponSlotComponents = {};
		array<string> componentTypeArray = {"CharacterWeaponSlotComponent"};
		int weaponSlotCount = SCR_BaseContainerTools.FindComponentSources(entitySource, componentTypeArray, weaponSlotComponents);
		
		array<IEntityComponentSource> weaponSlotComponentSources = weaponSlotComponents.Get(0);
		
		if (!weaponSlotComponentSources)
		{
			return;
		}
		
		foreach	(IEntityComponentSource weaponSlotComponent : weaponSlotComponentSources)
		{
			ResourceName weaponPrefab;
			if (weaponSlotComponent.Get("WeaponTemplate", weaponPrefab))
			{
				if (!weaponPrefab)
				{
					continue;
				}
				
				IEntitySource weaponSource = SCR_BaseContainerTools.FindEntitySource(Resource.Load(weaponPrefab));
				if (!weaponSource)
				{
					continue;
				}
				
				IEntityComponentSource weaponComponentSource = SCR_BaseContainerTools.FindComponentSource(weaponSource, "WeaponComponent");
				if (!weaponComponentSource)
				{
					continue;
				}
				
				EWeaponType weaponType = null;
				if (weaponComponentSource.Get("WeaponType", weaponType))
				{
					return weaponType;
				}
				return weaponType;
				
			}
		}
	}

	protected void SetBearingSetCompass(TextWidget Bearing, ImageWidget Compass)
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
		Compass.SetRotation(-yaw);
		
		// Set Bearing so if yaw outputs 6 it'll read 006
		string bearingAdd = "";
		// I love this.
		if (yaw >= 0 & yaw < 10 ) { bearingAdd = "00"; }
		if (yaw >= 10 & yaw < 100) { bearingAdd = "0"; }
		string bearingSTR = bearingAdd + (yaw.ToString());
		
		// Set Bearing Text
		Bearing.SetText(bearingSTR);
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
		//fadeBar(StamBar, 0.5, 2);
	}
	
	protected void hideBar(ProgressBarWidget widget) 
	{
		stamBarVisible = false;
		widget.SetOpacity(0);
		//fadeBar(StamBar, 0, 2);
	}
	
	void OnStaminaChange(float value)
    {
		StamBar = ProgressBarWidget.Cast(m_wRoot.FindWidget("StamBar"));
		StamBar.SetCurrent(value);
		//Print("Stamina: " + value);
		//Print("stamBarVisible: " + stamBarVisible);
		
		// Color
		// I couldnt get the switch statement to work. I hate this.
		if (value < 0.7 && value > 0.3)
		{
			StamBar.SetColor(new Color(0.091997,0.083009,0.035996,1.000000));
		} else if (value < 0.3) 
		{
			StamBar.SetColor(new Color(0.188724,0.046860,0.037476,1.000000));
		} else // Aka above 70%
		{
			StamBar.SetColor(new Color(0.035996,0.091997,0.050004,1.000000));
		}
		
		// Opacity
		if (value < 1 && !stamBarVisible) 
		{
			revealBar(StamBar);
		} else if (value == 1 && stamBarVisible) 
		{
			hideBar(StamBar);
		}
    }
};