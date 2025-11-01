class CSI_PlayerControllerManagerClass : ScriptComponentClass {};

class CSI_PlayerControllerManager : ScriptComponent
{		
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected CSI_HUDManager m_HUDManager;

	protected int m_iUpdate;
	protected int m_iCurrentUpdateCycle = 20;
	protected CSI_EIcon m_iLocalyStoredSpecialtyIcon;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		m_HUDManager = CSI_HUDManager.GetInstance();

		if (RplSession.Mode() == RplMode.Dedicated) 
			return;
		
		SetEventMask(owner, EntityEvent.FIXEDFRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		super.EOnFixedFrame(owner, timeSlice);
		
		m_iUpdate++;
		m_HUDManager.UpdateLocalAimingYaw();
		
		if (!(m_iUpdate >= 20))
			return;
		else
			m_iUpdate = 0;
		
		UpdateAllLocalPlayerValues();
		m_HUDManager.UpdateLocalHUDValues();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllLocalPlayerValues()
	{
		int playerId = SCR_PlayerController.GetLocalPlayerId();
		
		// Get local player entity.
		IEntity localplayer = SCR_PlayerController.GetLocalMainEntity();

		if (!localplayer) 
			return;
		
		// Get base group manager component
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManagerComponent) 
			return;
		
		// Get players current group.
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(playerId);

		if (!playersGroup) 
			return;
		
		m_iCurrentUpdateCycle = m_iCurrentUpdateCycle + 1;
		CSI_EIcon displayIcon = CSI_EIcon.MAN;

		//------------------------------------------------------------------------------------------------
		// Vehicle Icons, they supercede any other Icon
		BaseCompartmentSlot compartment = CSI_ChararcterHelper.GetCharacterVehicleCompartment(localplayer);
		if (compartment)
		{
			// Check players current compartment type, then assign his Icon.
			ECompartmentType compartmentType = compartment.GetType();
			switch (compartmentType)
			{
				case ECompartmentType.CARGO  : displayIcon = CSI_EIcon.PASSANGER;  break;
				case ECompartmentType.PILOT  : displayIcon = CSI_EIcon.DRIVER; break;
				case ECompartmentType.TURRET : displayIcon = CSI_EIcon.GUNNER; break;
			};
		};

		//------------------------------------------------------------------------------------------------
		//	SL Icon
		if (displayIcon == CSI_EIcon.MAN && playersGroup.IsPlayerLeader(playerId))
			displayIcon = CSI_EIcon.SL;

		//------------------------------------------------------------------------------------------------
		//	TL Icon
		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerId);
		if (displayIcon == CSI_EIcon.MAN && playerData && playerData.GetIsTeamLeader())
			displayIcon = CSI_EIcon.TL;

		//------------------------------------------------------------------------------------------------
		//	Specialty Icons
		if (displayIcon == CSI_EIcon.MAN && m_iCurrentUpdateCycle >= 16) 
		{
			// Get players inventory component
			SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(localplayer.FindComponent(SCR_InventoryStorageManagerComponent));

			// Get all of players inventory items
			array<IEntity> allPlayerItems = {};
			characterInventory.GetAllRootItems(allPlayerItems);

			// Setup new arrays and variables
			array<EWeaponType> weaponTypeArray = {};
			array<IEntity> medicalTypeArray = {};
			array<IEntity> explosiveTypeArray = {};
			array<IEntity> engineerTypeArray = {};

			// Parse through players entire inventory.
			foreach (IEntity item : allPlayerItems)
			{
				// Check if item is explosives related
				SCR_DetonatorGadgetComponent detonator = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
				SCR_ExplosiveChargeComponent explosives = SCR_ExplosiveChargeComponent.Cast(item.FindComponent(SCR_ExplosiveChargeComponent));
				SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(item.FindComponent(SCR_MineWeaponComponent));
				if(detonator || explosives || mine)
				{
					explosiveTypeArray.Insert(item);
					continue;
				};
				
				// Check if item is enginner related
				SCR_RepairSupportStationComponent engTool = SCR_RepairSupportStationComponent.Cast(item.FindComponent(SCR_RepairSupportStationComponent));
				if(engTool)
				{
					engineerTypeArray.Insert(item);
					continue;
				};
				
				// Check if item is medical related
				SCR_HealSupportStationComponent medTool = SCR_HealSupportStationComponent.Cast(item.FindComponent(SCR_HealSupportStationComponent));
				if(medTool)
				{
					medicalTypeArray.Insert(item);
					continue;
				};
				
				// Check if item is a weapon.
				WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
				if (weaponComp) 
				{
					// Get the weapons type and insert it into the weapon array so we can read it later.
					weaponTypeArray.Insert(weaponComp.GetWeaponType());
					
					array<BaseMuzzleComponent> muzzles = {};
					
					// Get muzzle types (so we can detect something like a underslung grenade launcher)
					for (int m = 0, mCount = weaponComp.GetMuzzlesList(muzzles); m < mCount; m++)
					{
						// Convert muzzle types to weapon types and insert it into the weapon array so we can read it later. (ToDo: Not hardcoded?)
						switch (muzzles[m].GetMuzzleType())
						{
							case EMuzzleType.MT_RPGMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_ROCKETLAUNCHER); break;};
							case EMuzzleType.MT_UGLMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_GRENADELAUNCHER); break;};
						};
					};
					continue;
				};
			};
			// Take all the data we just collected and assign players a Icon based on if it exists in the weapon/medical arrays.
			switch (true)
			{
				case (weaponTypeArray.Contains(EWeaponType.WT_MACHINEGUN))      : displayIcon = CSI_EIcon.MG;  break;
				case (weaponTypeArray.Contains(EWeaponType.WT_ROCKETLAUNCHER))  : displayIcon = CSI_EIcon.AT;  break;
				case (weaponTypeArray.Contains(EWeaponType.WT_SNIPERRIFLE))     : displayIcon = CSI_EIcon.SNIPER; break;
				case (medicalTypeArray.Count() != 0)                            : displayIcon = CSI_EIcon.MEDIC; break;
				case (explosiveTypeArray.Count() != 0)                          : displayIcon = CSI_EIcon.DEMO; break;
				case (engineerTypeArray.Count() != 0)                           : displayIcon = CSI_EIcon.ENG; break;
				case (weaponTypeArray.Contains(EWeaponType.WT_GRENADELAUNCHER)) : displayIcon = CSI_EIcon.GREN; break;
				default                                                         : displayIcon = CSI_EIcon.MAN;
			};
			
			m_iCurrentUpdateCycle = 0;
			m_iLocalyStoredSpecialtyIcon = displayIcon;
		}
		
		if (displayIcon == CSI_EIcon.MAN)
			displayIcon = m_iLocalyStoredSpecialtyIcon;
	
		m_RplToAuthorityManager.Owner_UpdatePlayerData(playerId, displayIcon, SCR_CharacterRankComponent.GetCharacterRank(localplayer));
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
