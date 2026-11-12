//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Client world system: every UPDATE_INTERVAL registers the local player's CSI data and reports their display
//! 				icon, rank and squad leader status to the server. Specialty icons come from an inventory scan that reruns
//! 				when the local character's inventory changes or the player gets a new character.
//! 
//------------------------------------------------------------------------------------------------
class CSI_ClientSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;

	// The specialty icon inventory scan only reruns after the local character's inventory reports a change,
	// the way vanilla SCR_AvailableActionsConditionData refetches its inventory data (m_bCanFetchInventory)
	protected SCR_InventoryStorageManagerComponent m_StorageManager;	//!< local character's inventory, listened to for changes
	protected bool m_bInventoryChanged = true;							//!< rescan on the next update cycle
	protected CSI_EIcon m_iLocallyStoredSpecialtyIcon;					//!< result of the last inventory scan
	protected int m_iLocallyStoredGroupID = -1;							//!< used to detect group changes

	protected ref CSI_SettingsJson m_SettingsJson;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INIT METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.FixedFrame);
	}

	//------------------------------------------------------------------------------------------------
	//! The inventory listeners are added from OnUpdatePoint, which only runs between OnStarted and OnStopped.
	override protected void OnStopped()
	{
		RemoveInventoryListeners();
		super.OnStopped();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INVENTORY EVENTS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Keeps the inventory listeners on the local character's storage manager: added for the first character (initial
	//! spawn) and moved to the new one when the character changes (respawn). Same pattern as vanilla
	//! SCR_AvailableActionsConditionData.
	//! \param[in] localPlayer
	protected void UpdateInventoryListeners(notnull IEntity localPlayer)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(localPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManager == m_StorageManager)
			return;

		RemoveInventoryListeners();

		// Scan what the new character already carries, as vanilla SCR_GadgetManagerComponent.RegisterInitialGadgets does
		m_StorageManager = storageManager;
		m_bInventoryChanged = true;

		if (!m_StorageManager)
			return;

		m_StorageManager.m_OnItemAddedInvoker.Insert(OnInventoryChanged);
		m_StorageManager.m_OnItemRemovedInvoker.Insert(OnInventoryChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveInventoryListeners()
	{
		if (!m_StorageManager)
			return;

		m_StorageManager.m_OnItemAddedInvoker.Remove(OnInventoryChanged);
		m_StorageManager.m_OnItemRemovedInvoker.Remove(OnInventoryChanged);
		m_StorageManager = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Fires for every item moved in or out (very often during spawn), so only flag it; the scan runs on the next update cycle.
	//! \param[in] item
	//! \param[in] storageOwner
	protected void OnInventoryChanged(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		m_bInventoryChanged = true;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected static const float UPDATE_INTERVAL = 0.5;	//!< seconds between update cycles
	protected float m_fTimeSinceUpdate;					//!< seconds since the last update cycle
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		// Managers live on the game mode, which may not exist yet
		if (!m_RplToAuthorityManager || !m_PlayerDataManager)
		{
			m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
			return;
		}

		// Time based like vanilla fixed frame systems (SCR_AIEnemyMarkingSystem) rather than a count of fixed frames
		m_fTimeSinceUpdate += args.GetTimeSliceSeconds();
		if (m_fTimeSinceUpdate < UPDATE_INTERVAL)
			return;

		m_fTimeSinceUpdate = 0;
		UpdateAllLocalPlayerValues();
	}

	//------------------------------------------------------------------------------------------------
	//! Works out the local player's display icon and reports it, their rank and squad leader status to the server.
	//! Icon priority: vehicle seat > officer > SL > TL > leader's override > specialty from inventory > MAN.
	protected void UpdateAllLocalPlayerValues()
	{
		int playerID = SCR_PlayerController.GetLocalPlayerId();

		// Main entity: the player's own character, also while a Game Master possesses another one. Checked every cycle, so a
		// first spawn or respawn is picked up on the next one (UpdateInventoryListeners then rescans the new character).
		IEntity localplayer = SCR_PlayerController.GetLocalMainEntity();

		if (!localplayer)
			return;

		// Re-resolve each cycle; on a listen server the instance cached before the host's controller existed may belong to another player
		CSI_RplToAuthorityManager localRplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		if (localRplToAuthorityManager)
			m_RplToAuthorityManager = localRplToAuthorityManager;

		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManagerComponent)
			return;

		// CSI data is only registered once the player is in a group
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(playerID);

		if (!playersGroup)
			return;

		// Group changed: clear the old group's color team, override and TL flag, then continue next cycle
		int playersGroupID = playersGroup.GetGroupID();
		if (m_iLocallyStoredGroupID != playersGroupID)
		{
			m_iLocallyStoredGroupID = playersGroupID;
			m_RplToAuthorityManager.Owner_ClearGroupSpecificData(playerID);
			return;
		};

		m_RplToAuthorityManager.Owner_RegisterPlayerData(playerID);
		UpdateInventoryListeners(localplayer);
		CSI_EIcon displayIcon = CSI_EIcon.MAN;

		//------------------------------------------------------------------------------------------------
		// Vehicle Icons, they supercede any other Icon
		BaseCompartmentSlot compartment = CSI_EntityHelper.GetCharacterVehicleCompartment(localplayer);
		if (compartment)
		{
			VehicleHelicopterSimulation heloSim = VehicleHelicopterSimulation.Cast(compartment.GetVehicle().FindComponent(VehicleHelicopterSimulation));
			VehicleFixedWingSimulation planeSim = VehicleFixedWingSimulation.Cast(compartment.GetVehicle().FindComponent(VehicleFixedWingSimulation));

			ECompartmentType compartmentType = compartment.GetType();
			string comartmentName = compartment.GetCompartmentName();

			UIInfo uiInfo = compartment.GetUIInfo();

			string name;
			if (uiInfo)
				name = uiInfo.GetName();

			// Commander and copilot seats are matched by name; everything else by compartment type
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

		CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(playerID);

		//------------------------------------------------------------------------------------------------
		//	OFFICER Icon
		if (displayIcon == CSI_EIcon.MAN && SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			displayIcon = CSI_EIcon.OFFICER;

		// Leader icons only in groups with more than one player
		if (playersGroup.GetPlayerCount() > 1)
		{
			//------------------------------------------------------------------------------------------------
			//	SL Icon
			if (displayIcon == CSI_EIcon.MAN && playersGroup.IsPlayerLeader(playerID))
				displayIcon = CSI_EIcon.SL;

			//------------------------------------------------------------------------------------------------
			//	TL Icon
			if (displayIcon == CSI_EIcon.MAN && playerData && playerData.GetIsTeamLeader())
				displayIcon = CSI_EIcon.TL;
		};

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
		if (displayIcon == CSI_EIcon.MAN && m_bInventoryChanged)
		{
			array<CSI_EIcon> validDisplayIcons = {};

			array<IEntity> allPlayerItems = {};
			if (m_StorageManager)
				m_StorageManager.GetItems(allPlayerItems);

			array<BaseMuzzleComponent> muzzles = {};
			// Parse through all items. Only RTO (the highest priority) ends the scan early, every other match
			// keeps scanning so a higher-priority item later in the inventory still wins in the switch below.
			foreach (IEntity item : allPlayerItems)
			{
				// RTO: backpack radio or deployable spawn point
				SCR_RadioComponent radioComp = SCR_RadioComponent.Cast(item.FindComponent(SCR_RadioComponent));
				SCR_RestrictedDeployableSpawnPointComponent spawnPoint = SCR_RestrictedDeployableSpawnPointComponent.Cast(item.FindComponent(SCR_RestrictedDeployableSpawnPointComponent));
				if(spawnPoint || (radioComp && radioComp.GetType() == EGadgetType.RADIO_BACKPACK))
				{
					validDisplayIcons.Insert(CSI_EIcon.RTO);
					break;
				};

				// Medic: healing support station (medical kit)
				SCR_HealSupportStationComponent medTool = SCR_HealSupportStationComponent.Cast(item.FindComponent(SCR_HealSupportStationComponent));
				if(medTool)
				{
					validDisplayIcons.Insert(CSI_EIcon.MEDIC);
					continue;
				};

				// Engineer: repair support station (repair kit)
				SCR_RepairSupportStationComponent engTool = SCR_RepairSupportStationComponent.Cast(item.FindComponent(SCR_RepairSupportStationComponent));
				if(engTool)
				{
					validDisplayIcons.Insert(CSI_EIcon.ENG);
					continue;
				};

				// Demo: detonator, explosive charge or mine
				SCR_DetonatorGadgetComponent detonator = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
				SCR_ExplosiveChargeComponent explosives = SCR_ExplosiveChargeComponent.Cast(item.FindComponent(SCR_ExplosiveChargeComponent));
				SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(item.FindComponent(SCR_MineWeaponComponent));
				if(detonator || explosives || mine)
				{
					validDisplayIcons.Insert(CSI_EIcon.DEMO);
					continue;
				};

				// Weapons: MG, sniper, launcher, or an underslung grenade launcher muzzle
				WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
				if (weaponComp)
				{
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

					// MG is the highest-priority weapon icon; no need to check this weapon's muzzles
					if (weaponType == EWeaponType.WT_MACHINEGUN)
						continue;

					muzzles.Clear();
					for (int m = 0, mCount = weaponComp .GetMuzzlesList(muzzles); m < mCount; m++)
					{
						// TODO: map muzzle types without hardcoding
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

			// Priority order of specialty icons, independent of inventory order
			switch (true)
			{
				case (validDisplayIcons.Contains(CSI_EIcon.RTO)) 		: displayIcon = CSI_EIcon.RTO; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.MEDIC)) 	: displayIcon = CSI_EIcon.MEDIC; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.ENG)) 		: displayIcon = CSI_EIcon.ENG; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.DEMO)) 		: displayIcon = CSI_EIcon.DEMO; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.MG)) 		: displayIcon = CSI_EIcon.MG; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.SNIPER)) 	: displayIcon = CSI_EIcon.SNIPER; 	break;
				case (validDisplayIcons.Contains(CSI_EIcon.AT)) 		: displayIcon = CSI_EIcon.AT; 		break;
				case (validDisplayIcons.Contains(CSI_EIcon.GREN)) 		: displayIcon = CSI_EIcon.GREN; 		break;
				default : displayIcon = CSI_EIcon.MAN;
			}

			m_bInventoryChanged = false;
			m_iLocallyStoredSpecialtyIcon = displayIcon;
		}

		// No scan this cycle: reuse the last scan's result
		if (displayIcon == CSI_EIcon.MAN)
			displayIcon = m_iLocallyStoredSpecialtyIcon;

		// Each Owner_* call only sends an RPC when its value changed
		m_RplToAuthorityManager.Owner_UpdatePlayerSquadLeader(playerID, playersGroup.IsPlayerLeader(playerID));
		m_RplToAuthorityManager.Owner_UpdatePlayerDisplayIcon(playerID, displayIcon);
		m_RplToAuthorityManager.Owner_UpdatePlayerRank(playerID, SCR_CharacterRankComponent.GetCharacterRank(localplayer));
	}
}
