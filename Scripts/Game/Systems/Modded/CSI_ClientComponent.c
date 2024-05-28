[ComponentEditorProps(category: "GameScripted/UI", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_ClientComponentClass : ScriptComponentClass {};

class CSI_ClientComponent : ScriptComponent
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Starting color team", category: "Default Player Settings", enums: { ParamEnum("None", "0"), ParamEnum("Red", "1"), ParamEnum("Blue", "2"), ParamEnum("Yellow", "3"), ParamEnum("Green", "4")})]
	protected int m_iStartingColorTeam;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Starting icon, especially usefull if you want to set teamleads automatically", category: "Default Player Settings", enums: { ParamEnum("None", "0"), ParamEnum("Team Lead", "1"), ParamEnum("Medic", "2"), ParamEnum("Marksman", "3"), ParamEnum("Machine Gunner", "4"), ParamEnum("Anti-Tank", "5"), ParamEnum("Grenadier", "6"), ParamEnum("Man", "7")})]
	protected int m_iOverrideIcon;
	
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
	protected int m_iCTRed    = ARGB(255, 200, 65, 65);
	protected int m_iCTBlue   = ARGB(255, 0, 92, 255);
	protected int m_iCTYellow = ARGB(255, 230, 230, 0);
	protected int m_iCTGreen  = ARGB(255, 0, 190, 85);
	protected int m_iCTNone   = ARGB(255, 215, 215, 215);
	
	// A hashmap that is modified only on the local user.
	protected ref map<string, string> m_mUpdateClientSettingsMap = new map<string, string>;
	
	// A array where we hold all local user settings.
	protected ref array<string> m_aLocalCSISettingsArray = new array<string>;

	// A array where we keep the local clients current group stored and sorted by the value determined for each player.
	protected ref array<string> m_aLocalGroupArray = new array<string>;
	
	protected CSI_AuthorityComponent m_AuthorityComponent;
	
	protected int m_iLocalPlayersGroupID = 1;
	
	protected int m_iCurrentUpdateCycle = 16;
	
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

		if (!GetGame().InPlayMode() || RplSession.Mode() == RplMode.Dedicated) 
			return;

		GetGame().GetInputManager().AddActionListener("CSISettingsMenu", EActionTrigger.DOWN, ToggleCSISettingsMenu);
		GetGame().GetInputManager().AddActionListener("PlayerSelectionMenu", EActionTrigger.DOWN, TogglePlayerSelectionMenu);
		
		GetGame().GetCallqueue().CallLater(WaitUntilWeSetDefaults, 1000, true);
		GetGame().GetCallqueue().CallLater(UpdateAllLocalPlayerValues, 625, true);
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

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	void UpdateLocalGroupArray()
	{
		array<string> tempLocalGroupArray = {};
		
		string groupString = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, -1, "GS"); // GS = GroupString

		if (groupString.IsEmpty()) return;

		array<string> outGroupStrArray = {};
		groupString.Split("|", outGroupStrArray, false);

		foreach (string playerString : outGroupStrArray) 
		{
			tempLocalGroupArray.Insert(playerString);
		};

		m_aLocalGroupArray = tempLocalGroupArray;
	}
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	string SwitchStringToIcon(string inputString)
	{
		string icon;
		switch (inputString) 
		{
			case "PAX" : {icon = m_sCargo;         break;};
			case "DRV" : {icon = m_sDriver;        break;};
			case "GNR" : {icon = m_sGunner;        break;};
			case "SL"  : {icon = m_sSquadLeader;   break;};
			case "FTL" : {icon = m_sTeamLeader;    break;};
			case "MED" : {icon = m_sMedic;         break;};
			case "MRK" : {icon = m_sMarksman;      break;};
			case "MG"  : {icon = m_sMachineGunner; break;};
			case "AT"  : {icon = m_sAntiTank;      break;};
			case "GRN" : {icon = m_sGrenadier;     break;};
			case "MAN" : {icon = m_sMan;           break;};
			default    : {icon = m_sMan;           break;};
		};
		
		return icon;
	}
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	int SwitchStringToColorTeam(string inputString)
	{
		int colorTeam;
		switch (inputString) 
		{
			case "R" : {colorTeam = m_iCTRed;    break;};
			case "B" : {colorTeam = m_iCTBlue;   break;};
			case "Y" : {colorTeam = m_iCTYellow; break;};
			case "G" : {colorTeam = m_iCTGreen;  break;};
			default  : {colorTeam = m_iCTNone;   break;};
		};
		
		return colorTeam;
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions for updating the local players icon.

	//------------------------------------------------------------------------------------------------
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void WaitUntilWeSetDefaults()
	{
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (!groupsManagerComponent) return;
		
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(playerID);
		
		if (!playersGroup) return;
		
		int groupID = playersGroup.GetGroupID();
		
		if (playerID != 0 && GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID) && groupID != -1) // Check if player has a ID, is in a valid entity, and has a valid group
		{
			GetGame().GetCallqueue().Remove(WaitUntilWeSetDefaults);
			
			switch (m_iStartingColorTeam) 
			{
				case 1 : {Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "R"); break;}; // CT = ColorTeam
				case 2 : {Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "B"); break;}; // CT = ColorTeam
				case 3 : {Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "Y"); break;}; // CT = ColorTeam
				case 4 : {Owner_UpdatePlayerMapValue(groupID, playerID, "CT", "G"); break;}; // CT = ColorTeam
			};

			switch (m_iOverrideIcon) 
			{
				case 1 : {Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "FTL"); break;}; // OI = OverrideIcon
				case 2 : {Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MED"); break;}; // OI = OverrideIcon
				case 3 : {Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MRK"); break;}; // OI = OverrideIcon
				case 4 : {Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MG");  break;}; // OI = OverrideIcon
				case 5 : {Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "AT");  break;}; // OI = OverrideIcon
				case 6 : {Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "GRN"); break;}; // OI = OverrideIcon
				case 7 : {Owner_UpdatePlayerMapValue(groupID, playerID, "OI", "MAN"); break;}; // OI = OverrideIcon
			};
		};
	}
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllLocalPlayerValues()
	{
		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		
		// Get local player entity.
		IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerID);

		if (!localplayer) return;
		
		if (m_AuthorityComponent.ReturnAuthoritySettings()[5]) 
			// Update PlayerRank
			Owner_UpdatePlayerMapValue(-1, localPlayerID, "PR", SCR_CharacterRankComponent.GetCharacterRankNameShort(localplayer)); // PR = PlayerRank
		
		if (!m_AuthorityComponent.ReturnAuthoritySettings()[1] && !m_AuthorityComponent.ReturnAuthoritySettings()[2] && !m_AuthorityComponent.ReturnAuthoritySettings()[7]) return;
		
		// Get base group manager component
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManagerComponent) return;
		
		// Get players current group.
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(localPlayerID);

		if (!playersGroup) return;
		
		m_iCurrentUpdateCycle = m_iCurrentUpdateCycle + 1;
		
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
					case ECompartmentType.Cargo  : {vehicleIcon = "PAX";  break;};
					case ECompartmentType.Pilot  : {vehicleIcon = "DRV"; break;};
					case ECompartmentType.Turret : {vehicleIcon = "GNR"; break;};
				};
			};
		};
		
		//------------------------------------------------------------------------------------------------
		// Set SL Icon
		//------------------------------------------------------------------------------------------------

		// Check if current player is the current squad leader.
		if (playersGroup.IsPlayerLeader(localPlayerID))
			// Set Squad Leader Icon
			specialtyIcon = "SL";

		//------------------------------------------------------------------------------------------------
		// Override regular Icons If Needed
		//------------------------------------------------------------------------------------------------

		string playerOverideIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "OI"); // OI = OverrideIcon

		if (!playerOverideIcon.IsEmpty() && playerOverideIcon != "N/A" && specialtyIcon.IsEmpty() && vehicleIcon.IsEmpty()) 
			specialtyIcon = playerOverideIcon;

		//------------------------------------------------------------------------------------------------
		//	Specialty Icons
		//------------------------------------------------------------------------------------------------

		if (specialtyIcon.IsEmpty() && vehicleIcon.IsEmpty() && m_iCurrentUpdateCycle >= 16) 
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
							case EMuzzleType.MT_RPGMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_ROCKETLAUNCHER); break;};
							case EMuzzleType.MT_UGLMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_GRENADELAUNCHER); break;};
						};
					};
				};
			};
			// Take all the data we just collected and assign players a Icon based on if it exists in the weapon/medical arrays.
			switch (true)
			{
				case (weaponTypeArray.Contains(EWeaponType.WT_MACHINEGUN))      : {specialtyIcon = "MG";  break;};
				case (weaponTypeArray.Contains(EWeaponType.WT_ROCKETLAUNCHER))  : {specialtyIcon = "AT";  break;};
				case (weaponTypeArray.Contains(EWeaponType.WT_SNIPERRIFLE))     : {specialtyIcon = "MRK"; break;};
				case (medicalTypeArray.Contains(SCR_EConsumableType.SALINE))    : {specialtyIcon = "MED"; break;};
				case (weaponTypeArray.Contains(EWeaponType.WT_GRENADELAUNCHER)) : {specialtyIcon = "GRN"; break;};
				default                                                         : {specialtyIcon = "MAN";       };
			};
			
			m_iCurrentUpdateCycle = 0;
		};

		if (!vehicleIcon.IsEmpty()) 
			displayIcon = vehicleIcon;
		else
			displayIcon = specialtyIcon;
		
		// Update the Icon we show on players screens.
		Owner_UpdatePlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "DI", displayIcon); // DI = DisplayIcon

		// Update StoredSpecialtyIcon.
		Owner_UpdatePlayerMapValue(m_iLocalPlayersGroupID, localPlayerID, "SSI", specialtyIcon); // SSI = StoredSpecialtyIcon
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions for updating the authority map which houses all player data

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	void Owner_UpdatePlayerMapValue(int groupID, int playerID, string write, string value)
	{
		string storedValue = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, playerID, write);
		if (storedValue == value || value.IsEmpty())
			return;
		
		Rpc(RpcAsk_UpdatePlayerMapValue, groupID, playerID, write, value);
	}

	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerMapValue(int groupID, int playerID, string write, string value)
	{
		if (m_AuthorityComponent)
			m_AuthorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, write, value);
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

	// Functions for Menus

	//------------------------------------------------------------------------------------------------

	protected void TogglePlayerSelectionMenu()
	{
		string storedSpecialtyIcon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iLocalPlayersGroupID, SCR_PlayerController.GetLocalPlayerId(), "SSI"); // SSI = StoredSpecialtyIcon
		
		if (ReturnLocalCSISettings()[8] == "false" || storedSpecialtyIcon == "SL" || storedSpecialtyIcon == "FTL")
		{
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSelectionDialog);
			return;
		} else {
			MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSettingsDialog, 0, true);
			
			CSI_PlayerSettingsDialog.Cast(menu).SetPlayerStr(string.Format("PlayerID:%1", SCR_PlayerController.GetLocalPlayerId()));
			return;
		};
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleCSISettingsMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_SettingsDialog);
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
			"compassVisible",						 //0
			"squadRadarVisible",				 //1
			"groupDisplayVisible",			 //2
			"staminaBarVisible",				 //3
			"nametagsVisible",					 //4
			"rankVisible",							 //5
			"nametagsRange",						 //6
			"roleNametagVisible",				 //7
			"personalColorTeamMenu",		 //8

			// Settings that are purely local to each client
			"squadRadarIconSize",        //9
			"squadRadarSelfIconVisible", //10
			"nametagsPosition",					 //11
			"compassTexture",					   //12
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
				case(!(settingServerOverride.IsEmpty() || settingServerOverride == "N/A")) : {settingValue = settingServerOverride; break;};
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
}
