class CSI_PlayerControllerManagerClass : ScriptComponentClass {};

class CSI_PlayerControllerManager : ScriptComponent
{		
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_HUDManager m_HUDManager;

	protected int m_iUpdate;
	protected int m_iCurrentUpdateCycle = 12;
	protected CSI_EIcon m_iLocallyStoredSpecialtyIcon;
	protected int m_iLocallyStoredGroupID = -1;
	
	protected ref CSI_SettingsJson m_SettingsJson;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_HUDManager = CSI_HUDManager.GetInstance();
		
		GetGame().GetInputManager().AddActionListener("CSI_PlayerSettingsMenu", EActionTrigger.DOWN, OpenLocalPlayerSettingsMenu);

		if (RplSession.Mode() != RplMode.Dedicated) 
		{
			m_SettingsJson = new CSI_SettingsJson;
			m_SettingsJson.LoadFromFile();
			
			SetEventMask(owner, EntityEvent.FRAME);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		m_iUpdate++;
		m_HUDManager.UpdateLocalAimingYaw();
		
		if (!(m_iUpdate >= 25))
			return;
		else
			m_iUpdate = 0;
		
		UpdateAllLocalPlayerValues();
		m_HUDManager.UpdateLocalHUDValues();
	}
	
	//------------------------------------------------------------------------------------------------
	CSI_SettingsJson GetLocalSettingsJson()
	{
		return m_SettingsJson;
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates all local player values to their current state within the CSI system.
	 */
	protected void UpdateAllLocalPlayerValues()
	{
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		
		// Get local player entity.
		IEntity localplayer = SCR_PlayerController.GetLocalMainEntity();

		if (!localplayer) 
			return;
		
		// Get base group manager component
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManagerComponent) 
			return;
		
		// Get players current group.
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(playerID);

		if (!playersGroup)
			return;
		
		int playersGroupID = playersGroup.GetGroupID();
		if (m_iLocallyStoredGroupID != playersGroupID) 
		{
			m_iLocallyStoredGroupID = playersGroupID;
			m_RplToAuthorityManager.Owner_ClearGroupSpecificData(playerID);
			return;
		};
		
		m_RplToAuthorityManager.Owner_RegisterPlayerData(playerID);
		m_iCurrentUpdateCycle = m_iCurrentUpdateCycle + 1;
		CSI_EIcon displayIcon = CSI_EIcon.MAN;

		//------------------------------------------------------------------------------------------------
		// Vehicle Icons, they supercede any other Icon
		BaseCompartmentSlot compartment = CSI_ChararcterHelper.GetCharacterVehicleCompartment(localplayer);
		if (compartment)
		{
			VehicleHelicopterSimulation heloSim = VehicleHelicopterSimulation.Cast(compartment.GetVehicle().FindComponent(VehicleHelicopterSimulation));
			VehicleFixedWingSimulation planeSim = VehicleFixedWingSimulation.Cast(compartment.GetVehicle().FindComponent(VehicleFixedWingSimulation));
			
			// Check players current compartment type, then assign his Icon.
			ECompartmentType compartmentType = compartment.GetType();
			string comartmentName = compartment.GetCompartmentName();
			
			UIInfo uiInfo = compartment.GetUIInfo();
			
			string name;
			if (uiInfo)
				name = uiInfo.GetName();
			
			if (name == "#AR-VehiclePosition_Commander" || comartmentName == "Commander")
				displayIcon = CSI_EIcon.COMMANDER;
			else {
				switch (compartmentType)
				{
					case ECompartmentType.CARGO  : displayIcon = CSI_EIcon.PASSANGER; break;
					case ECompartmentType.TURRET : displayIcon = CSI_EIcon.GUNNER; break;
					case ECompartmentType.PILOT  : {
						if (heloSim || planeSim)
							if (name == "#AR-VehiclePosition_Copilot" || comartmentName == "CopilotCompartment")
								displayIcon = CSI_EIcon.HELICREW;
							else
								displayIcon = CSI_EIcon.HELIPILOT;
						else
							displayIcon = CSI_EIcon.DRIVER;
					};
				};
			};
		};
		
		//------------------------------------------------------------------------------------------------
		//	OFFICER Icon
		if (displayIcon == CSI_EIcon.MAN && SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			displayIcon = CSI_EIcon.OFFICER;

		//------------------------------------------------------------------------------------------------
		//	SL Icon
		if (displayIcon == CSI_EIcon.MAN && playersGroup.IsPlayerLeader(playerID))
			displayIcon = CSI_EIcon.SL;

		//------------------------------------------------------------------------------------------------
		//	TL Icon
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);
		if (displayIcon == CSI_EIcon.MAN && playerData && playerData.GetIsTeamLeader())
			displayIcon = CSI_EIcon.TL;
		
		//------------------------------------------------------------------------------------------------
		//	Override Icon
		if (playerData)
		{
			CSI_EOverrideIcon overrideIcon = playerData.GetOverrideIcon();
			if (displayIcon == CSI_EIcon.MAN && overrideIcon != CSI_EOverrideIcon.AUTO)
				displayIcon = overrideIcon;
		};

		//------------------------------------------------------------------------------------------------
		//	Specialty Icons
		if (displayIcon == CSI_EIcon.MAN && m_iCurrentUpdateCycle >= 12) 
		{
			array<CSI_EIcon> validDisplayIcons = {};
			
			// Get players inventory component
			SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(localplayer.FindComponent(SCR_InventoryStorageManagerComponent));
			
			array<IEntity> allPlayerItems = {};
			characterInventory.GetItems(allPlayerItems);
			// Parse through all items.
			foreach (IEntity item : allPlayerItems)
			{	
				// Check if item is RTO related
				SCR_RadioComponent radioComp = SCR_RadioComponent.Cast(item.FindComponent(SCR_RadioComponent));
				SCR_RestrictedDeployableSpawnPointComponent spawnPoint = SCR_RestrictedDeployableSpawnPointComponent.Cast(item.FindComponent(SCR_RestrictedDeployableSpawnPointComponent));
				if(spawnPoint || (radioComp && radioComp.GetType() == EGadgetType.RADIO_BACKPACK))
				{
					validDisplayIcons.Insert(CSI_EIcon.RTO);
					break;
				};
				
				// Check if item is medical related
				SCR_HealSupportStationComponent medTool = SCR_HealSupportStationComponent.Cast(item.FindComponent(SCR_HealSupportStationComponent));
				if(medTool)
				{
					validDisplayIcons.Insert(CSI_EIcon.MEDIC);
					break;
				};
				
				// Check if item is enginner related
				SCR_RepairSupportStationComponent engTool = SCR_RepairSupportStationComponent.Cast(item.FindComponent(SCR_RepairSupportStationComponent));
				if(engTool)
				{
					validDisplayIcons.Insert(CSI_EIcon.ENG);
					break;
				};
				
				// Check if item is explosives related
				SCR_DetonatorGadgetComponent detonator = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
				SCR_ExplosiveChargeComponent explosives = SCR_ExplosiveChargeComponent.Cast(item.FindComponent(SCR_ExplosiveChargeComponent));
				SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(item.FindComponent(SCR_MineWeaponComponent));
				if(detonator || explosives || mine)
				{
					validDisplayIcons.Insert(CSI_EIcon.DEMO);
					break;
				};
					
				// Check if item is a weapon.
				WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
				if (weaponComp) 
				{
					// Get weapon type
					EWeaponType weaponType = weaponComp.GetWeaponType();
					if (weaponType == EWeaponType.WT_NONE)
						continue;
					
					switch (weaponType)
					{
						case EWeaponType.WT_MACHINEGUN : 
							validDisplayIcons.Insert(CSI_EIcon.MG); 
							break;
						case EWeaponType.WT_SNIPERRIFLE : 
							validDisplayIcons.Insert(CSI_EIcon.SNIPER);
							break;
						case EWeaponType.WT_ROCKETLAUNCHER : 
							validDisplayIcons.Insert(CSI_EIcon.AT);
							break;
					};

					array<BaseMuzzleComponent> muzzles = {};						
					// Get muzzle types (e.g., underslung grenade launcher)
					for (int m = 0, mCount = weaponComp .GetMuzzlesList(muzzles); m < mCount; m++)
					{
						// Convert muzzle types to weapon types (ToDo: Not hardcoded?)
						EWeaponType muzzleWeaponType = -1;
						switch (muzzles[m].GetMuzzleType())
						{
							case EMuzzleType.MT_UGLMuzzle: 
								validDisplayIcons.Insert(CSI_EIcon.GREN);
								break;
						}
					}
				};
			}
			
			// We use an array and switch to set priority on what to show other players
			switch (true)
			{
				case (validDisplayIcons.Contains(CSI_EIcon.RTO)) 		: displayIcon = CSI_EIcon.RTO; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.MEDIC)) 	: displayIcon = CSI_EIcon.MEDIC; 	break; 
				case (validDisplayIcons.Contains(CSI_EIcon.ENG)) 		: displayIcon = CSI_EIcon.ENG; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.DEMO)) 		: displayIcon = CSI_EIcon.DEMO; 	break;
				case (validDisplayIcons.Contains(CSI_EIcon.MG)) 		: displayIcon = CSI_EIcon.MG; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.SNIPER)) 	: displayIcon = CSI_EIcon.SNIPER; 	break;
				case (validDisplayIcons.Contains(CSI_EIcon.AT)) 		: displayIcon = CSI_EIcon.AT; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.GREN)) 		: displayIcon = CSI_EIcon.GREN; 	break;
				default : displayIcon = CSI_EIcon.MAN;
			}
			
			m_iCurrentUpdateCycle = 0;
			m_iLocallyStoredSpecialtyIcon = displayIcon;
		}
		
		if (displayIcon == CSI_EIcon.MAN)
			displayIcon = m_iLocallyStoredSpecialtyIcon;
		
		m_RplToAuthorityManager.Owner_UpdatePlayerSquadLead(playerID, playersGroup.IsPlayerLeader(playerID));
		m_RplToAuthorityManager.Owner_UpdatePlayerDisplayIcon(playerID, displayIcon);
		m_RplToAuthorityManager.Owner_UpdatePlayerRank(playerID, SCR_CharacterRankComponent.GetCharacterRank(localplayer));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenLocalPlayerSettingsMenu()
	{
		
		SCR_AIGroup group = SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (group && group.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()))
			GetGame().OpenGroupMenu();
		else
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSettingsMenu, 0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the PlayerControllerManager
	protected static CSI_PlayerControllerManager m_sInstance;
	static CSI_PlayerControllerManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	void CSI_PlayerControllerManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}
