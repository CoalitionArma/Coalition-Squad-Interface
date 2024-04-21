[ComponentEditorProps(category: "GameScripted/UI", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_ClientComponentClass : ScriptComponentClass {};

class CSI_ClientComponent : ScriptComponent
{
	// All Icons we could possibly want to give the player and/or to use for other functions.
	protected string m_sCargo         = "{05CAA2D974A461ED}UI\Textures\HUD\Modded\Icons\imagecargo_ca.edds";
	protected string m_sDriver        = "{9F51D41FDEB5D414}UI\Textures\HUD\Modded\Icons\imagedriver_ca.edds";
	protected string m_sGunner        = "{6049973DED62368F}UI\Textures\HUD\Modded\Icons\imagegunner_ca.edds";
	protected string m_sSquadLeader   = "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds";
	protected string m_sTeamLeader    = "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds";
	protected string m_sMedic         = "{C74F2DD12FEBFEB9}UI\Textures\HUD\Modded\Icons\Iconmanmedic_ca.edds";
	protected string m_sMarksman      = "{6CD9D05A934CDA32}UI\Textures\HUD\Modded\Icons\Iconmansniper_ca.edds";
	protected string m_sMachineGunner = "{C0938BB194E60432}UI\Textures\HUD\Modded\Icons\Iconmanmg_ca.edds";
	protected string m_sAntiTank      = "{D0E196FA6DA69F07}UI\Textures\HUD\Modded\Icons\Iconmanat_ca.edds";
	protected string m_sGrenadier     = "{FBC8C841728649FC}UI\Textures\HUD\Modded\Icons\Iconmangrenadier_ca.edds";
	protected string m_sMan           = "{25A0BFBD75253292}UI\Textures\HUD\Modded\Icons\Iconman_ca.edds";
	
	// All Color Teams
	protected string m_sCTRed    = ARGB(255, 200, 65, 65).ToString();
	protected string m_sCTBlue   = ARGB(255, 0, 92, 255).ToString();
	protected string m_sCTYellow = ARGB(255, 230, 230, 0).ToString();
	protected string m_sCTGreen  = ARGB(255, 0, 190, 85).ToString();
	protected string m_sCTNone   = ARGB(255, 215, 215, 215).ToString();
	
	// A hashmap that is modified only on the local user.
	protected ref map<string, string> m_mUpdateClientSettingsMap = new map<string, string>;
	
	// A array where we hold all local user settings.
	protected ref array<string> m_aLocalCSISettingsArray = new array<string>;

	// A array where we keep the local clients current group stored and sorted by the value determined for each player.
	protected ref array<string> m_aLocalGroupArray = new array<string>;
	
	protected CSI_AuthorityComponent m_AuthorityComponent;
	
	protected int m_iLocalPlayersGroupID = null;
	
	protected int m_iCurrentUpdateCycle = 26;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	static CSI_ClientComponent GetInstance()
	{
		if (GetGame().GetPlayerController())
			return CSI_ClientComponent.Cast(GetGame().GetPlayerController().FindComponent(CSI_ClientComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();

		if (!GetGame().InPlayMode() || Replication.IsServer()) return;

		GetGame().GetInputManager().AddActionListener("CSISettingsMenu", EActionTrigger.DOWN, ToggleCSISettingsMenu);
		GetGame().GetInputManager().AddActionListener("PlayerSelectionMenu", EActionTrigger.DOWN, TogglePlayerSelectionMenu);
		
		GetGame().GetCallqueue().CallLater(UpdateAllLocalPlayerValues, 525, true);
		UpdateLocalCSISettingArray();
	}

	//------------------------------------------------------------------------------------------------

	// Functions to sort and store the current group array we want to show on players screens.

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	TStringArray GetLocalGroupArray()
	{
		return m_aLocalGroupArray;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateLocalGroupArray()
	{
		array<string> tempLocalGroupArray = {};
		
		string groupString = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, -1, "GroupString");

		if (groupString.IsEmpty()) return;

		array<string> outGroupStrArray = {};
		groupString.Split("║", outGroupStrArray, false);

		foreach (string playerString : outGroupStrArray) 
		{
			tempLocalGroupArray.Insert(playerString);
		};

		m_aLocalGroupArray = tempLocalGroupArray;
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions for updating the local players icon.

	//------------------------------------------------------------------------------------------------
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllLocalPlayerValues()
	{
		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		
		// Get local player entity.
		IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerID);

		if (!localplayer) return;
		
		if (m_AuthorityComponent.ReturnAuthoritySettings()[5]) 
		{
			// Update PlayerRank
			Owner_UpdatePlayerMapValue(-1, localPlayerID, "PlayerRank", SCR_CharacterRankComponent.GetCharacterRankNameShort(localplayer));
		};
		
		if (!m_AuthorityComponent.ReturnAuthoritySettings()[1] && !m_AuthorityComponent.ReturnAuthoritySettings()[2] && !m_AuthorityComponent.ReturnAuthoritySettings()[7]) return;
		
		// Get base group manager component
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManagerComponent) return;
		
		// Get players current group.
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(localPlayerID);

		if (!playersGroup) return;
		
		m_iCurrentUpdateCycle = m_iCurrentUpdateCycle++;
		
		m_iLocalPlayersGroupID = playersGroup.GetGroupID();

		string vehicleIcon = "";
		string specialtyIcon = "";
		string displayIcon = "";

		//------------------------------------------------------------------------------------------------
		// Vehicle Icons, they supercede any other Icon
		//------------------------------------------------------------------------------------------------

		// Check if player is in a vehicle.
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(localplayer.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess.IsInCompartment())
		{
			// Check players current compartment.
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				// Check players current compartment type, then assign his Icon.
				ECompartmentType compartmentType = compartment.GetType();
				switch (compartmentType)
				{
					case ECompartmentType.Cargo  : {vehicleIcon = m_sCargo;  break; };
					case ECompartmentType.Pilot  : {vehicleIcon = m_sDriver; break; };
					case ECompartmentType.Turret : {vehicleIcon = m_sGunner; break; };
				};
			};
		};
		
		//------------------------------------------------------------------------------------------------
		// Set SL Icon
		//------------------------------------------------------------------------------------------------

		// Check if current player is the current squad leader.
		if (playersGroup.IsPlayerLeader(localPlayerID))
		{
			// Set Squad Leader Icon
			specialtyIcon = m_sSquadLeader;
		};

		//------------------------------------------------------------------------------------------------
		// Override regular Icons If Needed
		//------------------------------------------------------------------------------------------------

		string playerOverideIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "OverrideIcon");

		if (playerOverideIcon && specialtyIcon.IsEmpty() && vehicleIcon.IsEmpty()) 
		{
			switch (playerOverideIcon)
			{
				case "Team Lead"      : {specialtyIcon = m_sTeamLeader;    break; };
				case "Medic"          : {specialtyIcon = m_sMedic;         break; };
				case "Marksman"       : {specialtyIcon = m_sMarksman;      break; };
				case "Machine Gunner" : {specialtyIcon = m_sMachineGunner; break; };
				case "Anti-Tank"      : {specialtyIcon = m_sAntiTank;      break; };
				case "Grenadier"      : {specialtyIcon = m_sGrenadier;     break; };
				case "Man"            : {specialtyIcon = m_sMan;           break; };
			};
		};

		//------------------------------------------------------------------------------------------------
		//	Specialty Icons
		//------------------------------------------------------------------------------------------------

		if (specialtyIcon.IsEmpty()  && vehicleIcon.IsEmpty() && m_iCurrentUpdateCycle == 26) 
		{
			// Get players inventory component
			SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(localplayer.FindComponent(SCR_InventoryStorageManagerComponent));

			// Get all of players inventory items
			array<IEntity> allPlayerItems = {};
			characterInventory.GetAllRootItems(allPlayerItems);

			// Setup new arrays and variables
			array<EWeaponType> weaponTypeArray = {};
			array<SCR_EConsumableType> medicalTypeArray = {};

			// Parse through players entire inventory.
			foreach (IEntity item : allPlayerItems)
			{
				SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
				if (consumable)
				{
					// Check items type.
					SCR_EConsumableType medicalType = SCR_ConsumableItemComponent.Cast(consumable).GetConsumableType();
					if (medicalType == SCR_EConsumableType.SALINE)
					{
						medicalTypeArray.Insert(medicalType);
						
						// Get Saline Storage Component
						SCR_SalineStorageComponent salineStorageMan = SCR_SalineStorageComponent.Cast(localplayer.FindComponent(SCR_SalineStorageComponent));

						// Get all Saline bags attatched to this person, just so we dont accidentally assign cassualties the medic role.
						array<IEntity> items = {};
						salineStorageMan.GetAll(items);
						foreach (IEntity salineBag : items)
						{
							if (salineBag == item)
								// Insert the valid item into the medical array so we can read it later.
								medicalTypeArray.Clear();
						}
					};
				};
				// Check if item is a Weapon.
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
							case EMuzzleType.MT_RPGMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_ROCKETLAUNCHER); break; };
							case EMuzzleType.MT_UGLMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_GRENADELAUNCHER); break; };
						};
					};
				};
			};
			// Take all the data we just collected and assign players a Icon based on if it exists in the weapon/medical arrays.
			switch (true)
			{
				case (weaponTypeArray.Contains(EWeaponType.WT_MACHINEGUN))      : {specialtyIcon = m_sMachineGunner; break; };
				case (weaponTypeArray.Contains(EWeaponType.WT_ROCKETLAUNCHER))  : {specialtyIcon = m_sAntiTank;      break; };
				case (weaponTypeArray.Contains(EWeaponType.WT_SNIPERRIFLE))     : {specialtyIcon = m_sMarksman;      break; };
				case (medicalTypeArray.Contains(SCR_EConsumableType.SALINE))    : {specialtyIcon = m_sMedic;         break; };
				case (weaponTypeArray.Contains(EWeaponType.WT_GRENADELAUNCHER)) : {specialtyIcon = m_sGrenadier;     break; };
				default                                                         : {specialtyIcon = m_sMan;                  };
			};
			
			m_iCurrentUpdateCycle == 0;
		};
		
		if (specialtyIcon.IsEmpty()  && vehicleIcon.IsEmpty())
			specialtyIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "StoredSpecialtyIcon");
		
		if (specialtyIcon.IsEmpty())
			specialtyIcon = m_sMan;

		if (!vehicleIcon.IsEmpty()) 
			displayIcon = vehicleIcon;
		else 
			displayIcon = specialtyIcon;
		
		// Update the Icon we show on players screens.
		Owner_UpdatePlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "DisplayIcon", displayIcon);

		// Update StoredSpecialtyIcon.
		Owner_UpdatePlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "StoredSpecialtyIcon", specialtyIcon);
		
		// Get players Color Team so we can determine player value.
		string playerColorTeam = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "ColorTeam");
		if (!playerColorTeam || playerColorTeam.IsEmpty()) 
		{
			Owner_UpdatePlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "ColorTeam", m_sCTNone);
			playerColorTeam = m_sCTNone;
		}
		
		// Determine players value by their color team and icon so we can sort players from most to least valuable in the group display (definitely not racist).
		int playerValue = DetermineLocalPlayerValue(localPlayerID, playerColorTeam);

		// Update PlayerValue
		Owner_UpdatePlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "PlayerValue", playerValue.ToString());
	}
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	int DetermineLocalPlayerValue(int playerID, string colorTeam)
	{
		// Setup value variable.
		int value = 0;

		// Get player StoredSpecialtyIcon so we aren't influenced if the player is in a vehicle.
		string icon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, playerID, "StoredSpecialtyIcon");

		// Sort player by their color so we can group color teams together (a lil bit racist).
		switch (colorTeam) 
		{
			case m_sCTRed    : {value = -3; break; };
			case m_sCTBlue   : {value = -5; break; };
			case m_sCTYellow : {value = -7; break; };
			case m_sCTGreen  : {value = -9; break; };
			case m_sCTNone   : {value = 2;  break; };
		};

		switch (true) 
		{
			// If the players is currently the SL, make him the most valuable player in the list
			case (icon == m_sSquadLeader)                          : {value = -1; break; };

			// Add/Remove value from a player if they're a Team Lead
			case (icon == m_sTeamLeader && colorTeam == m_sCTNone) : {value--;    break; };
			case (icon == m_sTeamLeader && colorTeam != m_sCTNone) : {value++;    break; };
		};

		// Return how valuable the player is
		return value;
	}
	
	//------------------------------------------------------------------------------------------------

	// Function for getting/setting local settings

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	TStringArray ReturnLocalCSISettings() 
	{
		return m_aLocalCSISettingsArray;
	}
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	void Owner_ChangeLocalCSISetting(string setting, string value)
	{
		Rpc(RpcAsk_ChangeLocalCSISetting, setting, value);
	}

	//- Client Owner -\\
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcAsk_ChangeLocalCSISetting(string setting, string value)
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);

		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
		
		UpdateLocalCSISettingArray();
	};
		
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	void UpdateLocalCSISettingArray()
	{
		array<string> settingsToCheck = {
			// Settings that can be overriden by the server
			"compassVisible",						  //0
			"squadRadarVisible",					//1
			"groupDisplayVisible",				//2
			"staminaBarVisible",					//3
			"nametagsVisible",						//4
			"rankVisible",								//5
			"nametagsRange",							//6
			"roleNametagVisible",					//7
			"personalColorTeamMenu",			//8

			// Settings that are purely local to each client
			"squadRadarIconSize",					//9
			"squadRadarSelfIconVisible",	//10
			"nametagsPosition",						//11
			"compassTexture",						  //12
		};

		array<string> tempLocalCSISettingsArray = {};

		foreach (int i, string checkSetting : settingsToCheck)
		{
			string settingValue = "";
			string settingServerOverride = "";
			if (i < 9 && !m_AuthorityComponent.ReturnAuthoritySettings().IsEmpty()) 
			{
				settingServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[i];
			};
			switch (true)
			{
				case(!(settingServerOverride.IsEmpty() || settingServerOverride == "N/A")) : {settingValue = settingServerOverride; break; };
				default : {
					GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(checkSetting, settingValue); 
					if (i < 9 && settingValue.IsEmpty() && m_AuthorityComponent.ReturnAuthoritySettings()[9] == "true") 
					{
						 settingValue = m_AuthorityComponent.ReturnAuthoritySettings()[i+10]; 
					}; 
					break; 
				};
			};
			tempLocalCSISettingsArray.Insert(settingValue);
		};
		m_aLocalCSISettingsArray = tempLocalCSISettingsArray;
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to change Server Override Settings

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	void Owner_ChangeAuthoritySetting(string setting, string value)
	{
		Rpc(RpcAsk_ChangeAuthoritySetting, setting, value);
	}

	//- Server -\\
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ChangeAuthoritySetting(string setting, string value)
	{
		m_AuthorityComponent.UpdateAuthoritySetting(setting, value);
	}

	//------------------------------------------------------------------------------------------------

	// Functions for Group/Player Settings replication

	//------------------------------------------------------------------------------------------------

	//- Promote Player To SL -\\
	//------------------------------------------------------------------------------------------------
	void Owner_PromotePlayerToSL(int playerID)
	{
		Rpc(RpcAsk_PromotePlayerToSL, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_PromotePlayerToSL(int playerID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);
		playersGroup.SetGroupLeader(playerID);
	}

	//- Set Max Group Members -\\
	//------------------------------------------------------------------------------------------------
	void Owner_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		Rpc(RpcAsk_SetMaxGroupMembers, playerID, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxGroupMembers(int playerID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(playerID);

		if (maxMembers < playersGroup.GetPlayerCount()) maxMembers = playersGroup.GetPlayerCount();

		playersGroup.SetMaxMembers(maxMembers);
	}

	//- Remove Player From Group -\\
	//------------------------------------------------------------------------------------------------
	void Owner_RemovePlayerFromGroup(int playerID)
	{
		Rpc(RpcAsk_RemovePlayerFromGroup, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemovePlayerFromGroup(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerID);
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);

		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(group.GetFaction());

		if (!newGroup)
			return;
		playerGroupController.RequestJoinGroup(newGroup.GetGroupID());
	}

	//------------------------------------------------------------------------------------------------

	// Functions for Group Display replication

	//------------------------------------------------------------------------------------------------

	void Owner_UpdatePlayerMapValue(int groupID, int playerID, string write, string value)
	{
		string storedValue = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, playerID, write);
		if (storedValue == value || value.IsEmpty())
			return;
		
		Rpc(RpcAsk_UpdatePlayerMapValue, groupID, playerID, write, value);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerMapValue(int groupID, int playerID, string write, string value)
	{
		if (m_AuthorityComponent)
			m_AuthorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, write, value);
	}

	//------------------------------------------------------------------------------------------------

	// Functions for Menus

	//------------------------------------------------------------------------------------------------

	protected void TogglePlayerSelectionMenu()
	{
		string storedSpecialtyIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, SCR_PlayerController.GetLocalPlayerId(), "StoredSpecialtyIcon");
		
		if (ReturnLocalCSISettings()[8] == "false" || (SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId()).IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()) && storedSpecialtyIcon == "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds") || (storedSpecialtyIcon == "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds"))
		{
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSelectionDialog);
			return;
		} else {
			MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSettingsDialog, 0, true);
			CSI_PlayerSettingsDialog colorTeamMenu = CSI_PlayerSettingsDialog.Cast(menu);

			colorTeamMenu.SetPlayerStr(string.Format("PlayerID╣%1", SCR_PlayerController.GetLocalPlayerId()));
			return;
		};
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleCSISettingsMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_SettingsDialog);
	}
}
