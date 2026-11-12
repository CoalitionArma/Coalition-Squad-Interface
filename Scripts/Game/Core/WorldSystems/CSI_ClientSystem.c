//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Client world system: every 35 fixed frames registers the local player's CSI data and reports their
//! 				display icon, rank and squad leader status to the server. Specialty icons come from an inventory scan.
//! 
//------------------------------------------------------------------------------------------------
class CSI_ClientSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;

	protected int m_iCurrentUpdateCycle = 12;
	protected CSI_EIcon m_iLocallyStoredSpecialtyIcon;
	protected int m_iLocallyStoredGroupID = -1;

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

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	protected int m_iUpdate;	//!< fixed frames since the last update cycle
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

		m_iUpdate++;

		// only update every 35 frames
		if (!(m_iUpdate >= 35))
			return;
		else
			m_iUpdate = 0;

		UpdateAllLocalPlayerValues();
	}

	//------------------------------------------------------------------------------------------------
	//! Works out the local player's display icon and reports it, their rank and squad leader status to the server.
	//! Icon priority: vehicle seat > officer > SL > TL > leader's override > specialty from inventory > MAN.
	protected void UpdateAllLocalPlayerValues()
	{
		int playerID = SCR_PlayerController.GetLocalPlayerId();

		IEntity localplayer = SCR_PlayerController.GetLocalMainEntity();

		if (!localplayer)
			return;

		// Get base group manager component
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
		m_iCurrentUpdateCycle = m_iCurrentUpdateCycle + 1;
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
		if (displayIcon == CSI_EIcon.MAN && m_iCurrentUpdateCycle >= 12) 
		{
			array<CSI_EIcon> validDisplayIcons = {};

			SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(localplayer.FindComponent(SCR_InventoryStorageManagerComponent));

			array<IEntity> allPlayerItems = {};
				characterInventory.GetItems(allPlayerItems);

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
					break;
				};

				// Engineer: repair support station (repair kit)
				SCR_RepairSupportStationComponent engTool = SCR_RepairSupportStationComponent.Cast(item.FindComponent(SCR_RepairSupportStationComponent));
				if(engTool)
				{
					validDisplayIcons.Insert(CSI_EIcon.ENG);
					break;
				};

				// Demo: detonator, explosive charge or mine
				SCR_DetonatorGadgetComponent detonator = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
				SCR_ExplosiveChargeComponent explosives = SCR_ExplosiveChargeComponent.Cast(item.FindComponent(SCR_ExplosiveChargeComponent));
				SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(item.FindComponent(SCR_MineWeaponComponent));
				if(detonator || explosives || mine)
				{
					validDisplayIcons.Insert(CSI_EIcon.DEMO);
					break;
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

					// MG is the highest-priority weapon icon; no need to scan remaining items
					if (validDisplayIcons.Contains(CSI_EIcon.MG))
						break;

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

			m_iCurrentUpdateCycle = 0;
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
