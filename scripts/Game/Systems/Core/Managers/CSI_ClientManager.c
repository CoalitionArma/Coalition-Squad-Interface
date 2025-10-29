[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_ClientManagerClass : ScriptComponentClass {};

class CSI_ClientManager : ScriptComponent
{		
	protected CSI_AuthorityManager m_AuthorityComponent;
	protected int m_iCurrentUpdateCycle = 20;
	
	//------------------------------------------------------------------------------------------------
	// override/static functions
	//------------------------------------------------------------------------------------------------

	static CSI_ClientManager GetInstance()
	{
		if (GetGame().GetPlayerController())
			return CSI_ClientManager.Cast(GetGame().GetPlayerController().FindComponent(CSI_ClientManager));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_AuthorityComponent = CSI_AuthorityManager.GetInstance();

		if (!GetGame().InPlayMode() || RplSession.Mode() == RplMode.Dedicated) 
			return;

		GetGame().GetInputManager().AddActionListener("CSI_PlayerSelectionMenu", EActionTrigger.DOWN, TogglePlayerSelectionMenu);
		GetGame().GetInputManager().AddActionListener("CSI_SettingsMenu", EActionTrigger.DOWN, ToggleCSISettingsMenu);
		
		GetGame().GetCallqueue().CallLater(UpdateAllLocalPlayerValues, 250, true);
		UpdateLocalCSISettingArray();
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
		CSI_EIcon displayIcon;

		//------------------------------------------------------------------------------------------------
		// Vehicle Icons, they supercede any other Icon

		// Check players current compartment.
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

		if (playersGroup.IsPlayerLeader(playerId))
			displayIcon = CSI_EIcon.SL;

		//------------------------------------------------------------------------------------------------
		//	Specialty Icons

		if (displayIcon == CSI_EIcon.MAN && m_iCurrentUpdateCycle >= 20) 
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
		}
		
		Rpc(RpcAsk_UpdatePlayerData, playerId, displayIcon, SCR_CharacterRankComponent.GetCharacterRank(localplayer));
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_UpdatePlayerData(int playerId, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = m_AuthorityComponent.GetPlayerData(playerId);
		
		// Check if any data has updated
		if(playerData && (icon == playerData.GetDisplayIcon() && rank == playerData.GetRank()))
			return;
		
		m_AuthorityComponent.UpdatePlayerData(playerId, icon, rank);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerColorTeam(int playerId, CSI_EColorTeam colorTeam)
	{
		Rpc(RpcAsk_UpdatePlayerColorTeam, playerId, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerColorTeam(int playerId, CSI_EColorTeam colorTeam)
	{
		m_AuthorityComponent.UpdatePlayerColorTeam(playerId, colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerOverrideIcon(int playerId, CSI_EOverrideIcon overrideIcon)
	{
		Rpc(RpcAsk_UpdatePlayerOverrideIcon, playerId, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerOverrideIcon(int playerId, CSI_EOverrideIcon overrideIcon)
	{
		m_AuthorityComponent.UpdatePlayerOverrideIcon(playerId, overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_UpdatePlayerTeamLeader(int playerId, bool isTL)
	{
		Rpc(RpcAsk_UpdatePlayerTeamLeader, playerId, isTL);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerTeamLeader(int playerId, bool isTL)
	{
		m_AuthorityComponent.UpdatePlayerTeamLeader(playerId, isTL);
	}

	//------------------------------------------------------------------------------------------------
	// Functions for Group/Player Settings replication
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void Owner_PromotePlayerToSL(int playerId)
	{
		Rpc(RpcAsk_PromotePlayerToSL, playerId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerId)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerId);
		playersGroup.SetGroupLeader(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_SetMaxGroupMembers(int playerId, int maxMembers)
	{
		Rpc(RpcAsk_SetMaxGroupMembers, playerId, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerId, int maxMembers)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerId);

		if (maxMembers < playersGroup.GetPlayerCount()) 
			maxMembers = playersGroup.GetPlayerCount();

		playersGroup.SetMaxMembers(maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	void Owner_RemovePlayerFromGroup(int playerId)
	{
		Rpc(RpcAsk_RemovePlayerFromGroup, playerId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemovePlayerFromGroup(int playerId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerId);
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerId);

		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(group.GetFaction());

		if (!newGroup)
			return;
		playerGroupController.RequestJoinGroup(newGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	// Functions to change Server Override Settings
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void Owner_ChangeAuthoritySetting(string setting, string value)
	{
		Rpc(RpcAsk_ChangeAuthoritySetting, setting, value);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ChangeAuthoritySetting(string setting, string value)
	{
		//m_AuthorityComponent.UpdateAuthoritySetting(setting, value);
	}
	
	//------------------------------------------------------------------------------------------------

	// Function for getting/setting local settings

	//------------------------------------------------------------------------------------------------

	ref TStringArray m_aLocalCSISettingsArray = {};
	
	//------------------------------------------------------------------------------------------------
	TStringArray ReturnLocalCSISettings() 
	{
		return m_aLocalCSISettingsArray;
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeLocalCSISetting(string setting, string value)
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);
		
		UpdateLocalCSISettingArray();
	}
		
	//------------------------------------------------------------------------------------------------
	void UpdateLocalCSISettingArray()
	{
		/*
		array<string> settingsToCheck = {
			// Settings that can be overriden by the server
			"compassVisible",            //0
			"squadRadarVisible",         //1
			"groupDisplayVisible",       //2
			"staminaBarVisible",         //3
			"nametagsVisible",           //4
			"rankVisible",               //5
			"nametagsRange",             //6
			"roleNametagVisible",        //7
			"personalColorTeamMenu",     //8
			"groupNametagVisible",       //9
			"nametagLOSEnabled",         //10

			// Settings that are purely local to each client
			"squadRadarIconSize",        //11
			"squadRadarSelfIconVisible", //12
			"nametagsPosition",          //13
			"autoHideUI",                //14
			"compassTexture",            //15
		};

		array<string> tempLocalCSISettingsArray = {};

		foreach (int i, string checkSetting : settingsToCheck)
		{
			string settingValue = "";
			string settingServerOverride = "";
			if (i < 11 && !m_AuthorityComponent.ReturnAuthoritySettings().IsEmpty()) 
			{
				settingServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[i];
			};
			switch (true)
			{
				case(!(settingServerOverride.IsEmpty() || settingServerOverride == "N/A")) : {settingValue = settingServerOverride; break;};
				default : {
					GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(checkSetting, settingValue); 
					if (i < 11 && settingValue.IsEmpty() && (!m_AuthorityComponent.ReturnAuthoritySettings().IsEmpty() && m_AuthorityComponent.ReturnAuthoritySettings()[11] == "true")) 
					{
						 settingValue = m_AuthorityComponent.ReturnAuthoritySettings()[i+12]; 
					}; 
					break; 
				};
			};
			tempLocalCSISettingsArray.Insert(settingValue);
		};
		m_aLocalCSISettingsArray = tempLocalCSISettingsArray;
	
		*/
	}
}
