class CSI_PlayerControllerManagerClass : ScriptComponentClass {};

class CSI_PlayerControllerManager : ScriptComponent
{		
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_HUDManager m_HUDManager;

	protected int m_iUpdate;
	protected int m_iCurrentUpdateCycle = 12;
	protected CSI_EIcon m_iLocalyStoredSpecialtyIcon;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_HUDManager = CSI_HUDManager.GetInstance();

		if (RplSession.Mode() != RplMode.Dedicated) 
			SetEventMask(owner, EntityEvent.FRAME);
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
		
		m_iCurrentUpdateCycle = m_iCurrentUpdateCycle + 1;
		CSI_EIcon displayIcon = CSI_EIcon.MAN;

		//------------------------------------------------------------------------------------------------
		// Vehicle Icons, they supercede any other Icon
		BaseCompartmentSlot compartment = CSI_ChararcterHelper.GetCharacterVehicleCompartment(localplayer);
		if (compartment)
		{
			VehicleHelicopterSimulation heloSim = VehicleHelicopterSimulation.Cast(compartment.GetVehicle().FindComponent(VehicleHelicopterSimulation));
			
			// Check players current compartment type, then assign his Icon.
			ECompartmentType compartmentType = compartment.GetType();
			
			switch (compartmentType)
			{
				case ECompartmentType.CARGO  : displayIcon = CSI_EIcon.PASSANGER; break;
				case ECompartmentType.TURRET : displayIcon = CSI_EIcon.GUNNER; break;
				
				case ECompartmentType.PILOT  : {
					if (heloSim)
					{
						UIInfo uiInfo = compartment.GetUIInfo();
						if (uiInfo.GetName() == "#AR-VehiclePosition_Copilot")
							displayIcon = CSI_EIcon.HELICREW;
						else
							displayIcon = CSI_EIcon.HELIPILOT;
					} else
						displayIcon = CSI_EIcon.DRIVER; 
					break;
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
				if(spawnPoint || (radioComp && radioComp.GetRadioCategory() == ERadioCategory.MANPACK))
				{
					displayIcon = CSI_EIcon.RTO;
					break;
				};
				
				// Check if item is medical related
				SCR_HealSupportStationComponent medTool = SCR_HealSupportStationComponent.Cast(item.FindComponent(SCR_HealSupportStationComponent));
				if(medTool)
				{
					displayIcon = CSI_EIcon.MEDIC;
					break;
				};
				
				// Check if item is enginner related
				SCR_RepairSupportStationComponent engTool = SCR_RepairSupportStationComponent.Cast(item.FindComponent(SCR_RepairSupportStationComponent));
				if(engTool)
				{
					displayIcon = CSI_EIcon.ENG;
					break;
				};
					
				// Check if item is a weapon.
				WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
				if (weaponComp) 
				{
					//--- Get weapon type
					EWeaponType weaponType = weaponComp.GetWeaponType();
					if (weaponType == EWeaponType.WT_NONE)
						continue;
					
					switch (weaponType)
					{
						case EWeaponType.WT_MACHINEGUN : 
							displayIcon = CSI_EIcon.MG; 
							break;
						case EWeaponType.WT_SNIPERRIFLE : 
							displayIcon = CSI_EIcon.SNIPER; 
							break;
						case EWeaponType.WT_ROCKETLAUNCHER : 
							displayIcon = CSI_EIcon.AT;
							break;
					};

					array<BaseMuzzleComponent> muzzles = {};						
					//-- Get muzzle types (e.g., underslung grenade launcher)
					for (int m = 0, mCount = weaponComp .GetMuzzlesList(muzzles); m < mCount; m++)
					{
						//--- Convert muzzle types to weapon types (ToDo: Not hardcoded?)
						EWeaponType muzzleWeaponType = -1;
						switch (muzzles[m].GetMuzzleType())
						{
							case EMuzzleType.MT_UGLMuzzle: 
								displayIcon = CSI_EIcon.GREN;
								break;
						}
					}
					
					if (displayIcon != CSI_EIcon.MAN)
						break;
					else
						continue;
				};
				
				// Check if item is explosives related
				SCR_DetonatorGadgetComponent detonator = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
				SCR_ExplosiveChargeComponent explosives = SCR_ExplosiveChargeComponent.Cast(item.FindComponent(SCR_ExplosiveChargeComponent));
				SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(item.FindComponent(SCR_MineWeaponComponent));
				if(detonator || explosives || mine)
				{
					displayIcon = CSI_EIcon.DEMO;
					break;
				};
			}
			
			m_iCurrentUpdateCycle = 0;
			m_iLocalyStoredSpecialtyIcon = displayIcon;
		}
		
		if (displayIcon == CSI_EIcon.MAN)
			displayIcon = m_iLocalyStoredSpecialtyIcon;
	
		m_RplToAuthorityManager.Owner_UpdatePlayerData(playerID, playersGroup.IsPlayerLeader(playerID), displayIcon, SCR_CharacterRankComponent.GetCharacterRank(localplayer));
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
