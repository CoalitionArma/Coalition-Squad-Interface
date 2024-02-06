[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class CSI_AuthorityComponentClass: SCR_BaseGameModeComponentClass {};

class CSI_AuthorityComponent : SCR_BaseGameModeComponent
{	
	// All Icons we could possibly want to give the player and/or to use for other functions.
	protected string m_sCargo         = "{05CAA2D974A461ED}UI\Textures\HUD\Modded\Icons\imagecargo_ca.edds";
	protected string m_sDriver        = "{9F51D41FDEB5D414}UI\Textures\HUD\Modded\Icons\imagedriver_ca.edds";
	protected string m_sGunner        = "{6049973DED62368F}UI\Textures\HUD\Modded\Icons\imagegunner_ca.edds";
	protected string m_sSquadLeader   = "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds";
	protected string m_sTeamLeader    = "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds";
	protected string m_sMedic         = "{C74F2DD12FEBFEB9}UI\Textures\HUD\Modded\Icons\Iconmanmedic_ca.edds";
	protected string m_sSniper        = "{6CD9D05A934CDA32}UI\Textures\HUD\Modded\Icons\Iconmansniper_ca.edds";
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
	
	// A hashmap that is modified only on the authority.
	protected ref map<string,string> m_mUpdateAuthoritySettingsMap = new map<string,string>;
	
	// A array we use to broadcast whenever a change happens to any of the server overrides.
	[RplProp()]
	protected ref array<string> m_aServerOverridesArray = new array<string>;
	
	// A hashmap that is modified only on the authority.
	protected ref map<string,string> m_mAuthorityPlayerMap = new map<string,string>;
	
	// A hashmap that is modified only on each client by a .BumpMe from the authority.
	protected ref map<string,string> m_mLocalPlayerMap = new map<string,string>;
	
	// A array we use primarily for replication of m_mAuthorityPlayerMap to m_mLocalPlayerMap.
	[RplProp(onRplName: "UpdateLocalPlayerMap")]
	protected ref array<string> m_aPlayerArray = new array<string>;
	
	// The vanilla group manager.
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent = null;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	static CSI_AuthorityComponent GetInstance() 
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return CSI_AuthorityComponent.Cast(gameMode.FindComponent(CSI_AuthorityComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		if (Replication.IsServer()) {
			GetGame().GetCallqueue().CallLater(UpdateAllAuthorityPlayerMapValues, 500, true);
			GetGame().GetCallqueue().CallLater(CheckAuthoritySettings, 5000, true);
			GetGame().GetCallqueue().CallLater(CleanUpAuthorityPlayerMap, 21000000, true); // Updates every 35min (21000000ms)
		};
	} 
	
	//------------------------------------------------------------------------------------------------
	override protected void OnGameEnd()
	{	
		super.OnGameEnd();
		
		if (Replication.IsServer()) {
			GetGame().GetCallqueue().Remove(UpdateAllAuthorityPlayerMapValues);
			GetGame().GetCallqueue().Remove(CheckAuthoritySettings);
			GetGame().GetCallqueue().Remove(CleanUpAuthorityPlayerMap);
		};
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to change/get Server Override Settings
	
	//------------------------------------------------------------------------------------------------
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	TStringArray ReturnAuthoritySettings()
	{
		return m_aServerOverridesArray;
	}
	
	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void UpdateAuthoritySetting(string setting, string value)
	{
		m_mUpdateAuthoritySettingsMap.Set(setting, value);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckAuthoritySettings()
	{
		if (m_mUpdateAuthoritySettingsMap.Count() > 0) {
			for (int i = 0; i < m_mUpdateAuthoritySettingsMap.Count(); i++)
			{
				string setting = m_mUpdateAuthoritySettingsMap.GetKey(i);
				string value = m_mUpdateAuthoritySettingsMap.Get(setting);
			
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);
			};
			m_mUpdateAuthoritySettingsMap.Clear();
			GetGame().UserSettingsChanged();
			GetGame().SaveUserSettings();
		};

		m_aServerOverridesArray.Clear();
		array<string> serverOverridesArray = {"compassVisibleServerOverride", "squadRadarVisibleServerOverride", "groupDisplayVisibleServerOverride", "staminaBarVisibleServerOverride", "nametagsVisibleServerOverride", "rankVisibleServerOverride"};
		foreach (string serverOverride : serverOverridesArray)
		{
			string value = "";
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(serverOverride, value);
			if (value == "") value = "N/A";
			m_aServerOverridesArray.Insert(value)
		}
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to replicate and store values to each clients m_mLocalPlayerMap

	//------------------------------------------------------------------------------------------------
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	string ReturnLocalPlayerMapValue(int groupID, int playerID, string key)
	{	
		// Get the players key 
		key = string.Format("%1 %2 %3", groupID, playerID, key);
		return m_mLocalPlayerMap.Get(key);
	}
	
	
	//- Authority & Client -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerArray()
	{
		// Create a temp array so we arent broadcasting for each change to m_aPlayerArray.
		protected ref array<string> tempPlayerArray = new array<string>;
		
		// Fill tempPlayerArray with all keys and values in m_mAuthorityPlayerMap.
		for (int i = 0; i < m_mAuthorityPlayerMap.Count(); i++)
		{
			string key = m_mAuthorityPlayerMap.GetKey(i);
			string value = m_mAuthorityPlayerMap.Get(key);
			
			tempPlayerArray.Insert(string.Format("%1║%2", key, value));
		};
		
		// Replicate m_aPlayerArray to all clients.
		m_aPlayerArray = tempPlayerArray;
		Replication.BumpMe();
	}
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdateLocalPlayerMap()
	{
		// Fill m_mLocalPlayerMap with all keys and values from m_mAuthorityPlayerMap.
	 	foreach (string playerKeyAndValueToSplit : m_aPlayerArray)
		{		
			array<string> playerKeyAndValueArray = {};
			playerKeyAndValueToSplit.Split("║", playerKeyAndValueArray, false);
			m_mLocalPlayerMap.Set(playerKeyAndValueArray[0], playerKeyAndValueArray[1]);
		};
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to update m_mAuthorityPlayerMap.

	//------------------------------------------------------------------------------------------------
	
	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void UpdateAuthorityPlayerMapValue(int groupID, int playerID, string write, string value)
	{						
		// The key we are gonna use that keeps everything locallied to the group and player, so we don't get any cross-contamination between groups or players.
		string key = string.Format("%1 %2 %3", groupID, playerID, write);
		m_mAuthorityPlayerMap.Set(key, value);
	}
	
	//------------------------------------------------------------------------------------------------
	string ReturnAuthorityPlayerMapValue(int groupID, int playerID, string key)
	{	
		// Get the players key
		key = string.Format("%1 %2 %3", groupID, playerID, key);
		return m_mAuthorityPlayerMap.Get(key);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CleanUpAuthorityPlayerMap()
	{
		map<string,string> tempMap = new map<string,string>;
		array<int> outPlayers = new array<int>;
		
		GetGame().GetPlayerManager().GetPlayers(outPlayers);
		foreach (int playerID : outPlayers) {
			SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(playerID);
			if (!playersGroup) continue;
			
			int groupID = playersGroup.GetGroupID();
			
			string colorTeam = ReturnAuthorityPlayerMapValue(groupID, playerID, "ColorTeam");
			string key = string.Format("%1 %2 %3", groupID, playerID, "ColorTeam");
			tempMap.Set(key, colorTeam);
			
			string overrideIcon = ReturnAuthorityPlayerMapValue(groupID, playerID, "OverrideIcon");
			string key2 = string.Format("%1 %2 %3", groupID, playerID, "OverrideIcon");
			tempMap.Set(key2, overrideIcon);
		};
		
		m_mAuthorityPlayerMap.Clear();
		
		m_mAuthorityPlayerMap = tempMap;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAllAuthorityPlayerMapValues()
	{	
		array<SCR_AIGroup> outAllGroups;

		// Get base group manager component
		if (!m_GroupsManagerComponent) {
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			return;
		};
		
		// Get all groups
		m_GroupsManagerComponent.GetAllPlayableGroups(outAllGroups);
		
		foreach (SCR_AIGroup playersGroup : outAllGroups)
		{
			if (!playersGroup) continue;
			
			array<string> groupStringArray = {};
	
			// Get list of all the players we have to parse through.
			array<int> groupPlayersIDs = playersGroup.GetPlayerIDs();
			
			// Get Group ID
			int groupID = playersGroup.GetGroupID();
	
			// Parse through current group array.
			foreach(int localPlayerID : groupPlayersIDs) 
			{
				IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerID);
	
				// Skip ahead to next for-loop iteration if any of these are false.
				if (!localplayer) continue;
				
				array<string> iconArray = {};
				
				//------------------------------------------------------------------------------------------------
				// Color Teams
				//------------------------------------------------------------------------------------------------

				string playerColorTeam = ReturnAuthorityPlayerMapValue(groupID, localPlayerID, "ColorTeam");
				if (!playerColorTeam || playerColorTeam == "") UpdateAuthorityPlayerMapValue(groupID, localPlayerID, "ColorTeam", m_sCTNone);

				//------------------------------------------------------------------------------------------------
				// Vehicle Icons, they supercede any other Icon
				//------------------------------------------------------------------------------------------------

				// Check if player is in a vehicle.
				CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(localplayer.FindComponent(CompartmentAccessComponent));
				if (compartmentAccess)
				{
					// Check players current compartment.
					BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
					if (compartment)
					{
						// Check players current compartment type, then assign his Icon.
						ECompartmentType compartmentType = SCR_CompartmentAccessComponent.GetCompartmentType(compartment);
						switch (compartmentType)
						{
							case ECompartmentType.Cargo  : {iconArray.Insert(m_sCargo);  break;};
							case ECompartmentType.Pilot  : {iconArray.Insert(m_sDriver); break;};
							case ECompartmentType.Turret : {iconArray.Insert(m_sGunner); break;};
						};
					};
				};

				//------------------------------------------------------------------------------------------------
				// Set SL Icon
				//------------------------------------------------------------------------------------------------
		
				// Check if current player is the current squad leader.
				if (playersGroup.IsPlayerLeader(localPlayerID) && iconArray.IsEmpty())
				{
					// Set Squad Leader Icon
					iconArray.Insert(m_sSquadLeader);
				};

				//------------------------------------------------------------------------------------------------
				// Override regular Icons If Set By SL
				//------------------------------------------------------------------------------------------------

				string playerOverideIcon = ReturnAuthorityPlayerMapValue(groupID, localPlayerID, "OverrideIcon");
	
				if (playerOverideIcon && iconArray.IsEmpty()) {
					switch (playerOverideIcon)
					{
						case "Team Lead"      : {iconArray.Insert(m_sTeamLeader);    break;};
						case "Medic"          : {iconArray.Insert(m_sMedic);         break;};
						case "Sniper"         : {iconArray.Insert(m_sSniper);        break;};
						case "Machine Gunner" : {iconArray.Insert(m_sMachineGunner); break;};
						case "Anti-Tank"      : {iconArray.Insert(m_sAntiTank);      break;};
						case "Grenadier"      : {iconArray.Insert(m_sGrenadier);     break;};
						case "Man"            : {iconArray.Insert(m_sMan);           break;};
					};
				};

				//------------------------------------------------------------------------------------------------
				//	Specialty Icons
				//------------------------------------------------------------------------------------------------

				if (iconArray.IsEmpty()) {
					// Get players inventory component
					SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(localplayer.FindComponent(SCR_InventoryStorageManagerComponent));

					// Get all of players inventory items
					array<IEntity> allPlayerItems = {};
					characterInventory.GetAllRootItems(allPlayerItems);
		
					// Setup new arrays and variables
					array<EWeaponType> weaponTypeArray = {};
					array<SCR_EConsumableType> medicalTypeArray = {};
					array<BaseMuzzleComponent> muzzles = {};

					// Parse through players entire inventory.
					foreach (IEntity item : allPlayerItems)
					{
						// Check if item is consumable (nearly always a medical item).
						SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
						if (consumable)
						{
							// Check items type.
							SCR_EConsumableType medicalType = SCR_ConsumableItemComponent.Cast(consumable).GetConsumableType();
							if (medicalType == SCR_EConsumableType.SALINE)
							{
								// Insert the valid item into the medical array so we can read it later.
								medicalTypeArray.Insert(medicalType);
							};
						};
						// Check if item is a Weapon.
						WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
						if (weaponComp) {
							// Get the weapons type and insert it into the weapon array so we can read it later.
							weaponTypeArray.Insert(weaponComp.GetWeaponType());
							// Get muzzle types (so we can detect something like a underslung grenade launcher)
							for (int m = 0, mCount = weaponComp.GetMuzzlesList(muzzles); m < mCount; m++)
							{
								// Convert muzzle types to weapon types and insert it into the weapon array so we can read it later. (ToDo: Not hardcoded?)
								switch (muzzles[m].GetMuzzleType())
								{
									case EMuzzleType.MT_RPGMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_ROCKETLAUNCHER);  break;};
									case EMuzzleType.MT_UGLMuzzle : {weaponTypeArray.Insert(EWeaponType.WT_GRENADELAUNCHER); break;};
								};
							};
						};
					};
					// Take all the data we just collected and assign players a Icon based on if it exists in the weapon/medical arrays.
					switch (true)
					{
						case (medicalTypeArray.Contains(SCR_EConsumableType.SALINE))    : {iconArray.Insert(m_sMedic);         break;};
						case (weaponTypeArray.Contains(EWeaponType.WT_SNIPERRIFLE))     : {iconArray.Insert(m_sSniper);        break;};
						case (weaponTypeArray.Contains(EWeaponType.WT_MACHINEGUN))      : {iconArray.Insert(m_sMachineGunner); break;};
						case (weaponTypeArray.Contains(EWeaponType.WT_ROCKETLAUNCHER))  : {iconArray.Insert(m_sAntiTank);      break;};
						case (weaponTypeArray.Contains(EWeaponType.WT_GRENADELAUNCHER)) : {iconArray.Insert(m_sGrenadier);     break;};
						default                                                         : {iconArray.Insert(m_sMan);                 };
					};
				};
				
				// Get players icon from the iconArray.
				string icon = iconArray[0];
				
				// Update the Icon we show on players screens.
				UpdateAuthorityPlayerMapValue(groupID, localPlayerID, "DisplayIcon", icon);
		
				// If player is not in a vehicle seat, make sure to store their Icon so we can use it for other menus/sorting of the players.
				if (icon != m_sCargo && icon != m_sDriver && icon != m_sGunner) {
					string playerStoredSpecialtyIcon = ReturnAuthorityPlayerMapValue(groupID, localPlayerID, "StoredSpecialtyIcon");
					
					// In case player spawns in a vehicle/state where we can't read his inventory, use m_sMan to keep all menus/functions happy.
					if (!playerStoredSpecialtyIcon || playerStoredSpecialtyIcon == "") UpdateAuthorityPlayerMapValue(groupID, localPlayerID, "StoredSpecialtyIcon", m_sMan);
					
					// Update StoredSpecialtyIcon.
					UpdateAuthorityPlayerMapValue(groupID, localPlayerID, "StoredSpecialtyIcon", icon);
				}
				
				// Determine players value by their color team and icon so we can sort players from most to least valuable in the group display (definitely not racist).
				int playerValue = DeterminePlayerValue(groupID, localPlayerID, playerColorTeam);
				
				// Update PlayerValue
				UpdateAuthorityPlayerMapValue(groupID, localPlayerID, "PlayerValue", playerValue.ToString());
			};
		};
		
		//Once we've updated all values, propagate them to all clients with UpdatePlayerArray.
		UpdatePlayerArray();
	};
	
	//------------------------------------------------------------------------------------------------
	int DeterminePlayerValue(int groupID, int playerID, string colorTeam)
	{
		// Setup value variable.
		int value = 0;
		
		// Get player StoredSpecialtyIcon so we aren't influenced if the player is in a vehicle.
		string icon = ReturnAuthorityPlayerMapValue(groupID, playerID, "StoredSpecialtyIcon");

		// Sort player by their color so we can group color teams together (a lil bit racist).
		switch (colorTeam) {
			case m_sCTRed    : {value = -3;  break;};
			case m_sCTBlue   : {value = -5;  break;};
			case m_sCTYellow : {value = -7;  break;};
			case m_sCTGreen  : {value = -9;  break;};
			case m_sCTNone   : {value = 2;   break;};
		};
				
		switch (true) {
			// If the players is currently the SL, make him the most valuable player in the list
			case (icon == m_sSquadLeader)                          : {value = -1; break;};
			
			// Add/Remove value from a player if they're a Team Lead
			case (icon == m_sTeamLeader && colorTeam == m_sCTNone) : {value--;    break;};
			case (icon == m_sTeamLeader && colorTeam != m_sCTNone) : {value++;    break;};
		};
		
		// Return how valuable the player is
		return value;
	}
}
