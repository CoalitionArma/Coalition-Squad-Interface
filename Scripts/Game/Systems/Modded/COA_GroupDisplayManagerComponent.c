[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class COA_GroupDisplayManagerComponentClass: SCR_BaseGameModeComponentClass {};

class COA_GroupDisplayManagerComponent : SCR_BaseGameModeComponent
{	
	// All Icons we could possibly want to give the player and/or to use for other functions.
	protected string m_sCargo         = "{B910A93F355F168C}UI\Textures\HUD\Modded\Icons\imagecargo_ca.edds";
	protected string m_sDriver        = "{C2B2F451FB157A89}UI\Textures\HUD\Modded\Icons\imagedriver_ca.edds";
	protected string m_sGunner        = "{3DAAB773C8C29812}UI\Textures\HUD\Modded\Icons\imagegunner_ca.edds";
	protected string m_sSquadLeader   = "{5ECE094ED4662B33}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds";
	protected string m_sTeamLeader    = "{6D45BA2CCC322312}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds";
	protected string m_sMedic         = "{01F2523A4EE5C48B}UI\Textures\HUD\Modded\Icons\Iconmanmedic_ca.edds";
	protected string m_sSniper        = "{318B797C57BE3C29}UI\Textures\HUD\Modded\Icons\Iconmansniper_ca.edds";
	protected string m_sMachineGunner = "{CCF40410BDB53870}UI\Textures\HUD\Modded\Icons\Iconmanmg_ca.edds";
	protected string m_sAntiTank      = "{DC86195B44F5A345}UI\Textures\HUD\Modded\Icons\Iconmanat_ca.edds";
	protected string m_sGrenadier     = "{B7757F2024A3DC87}UI\Textures\HUD\Modded\Icons\Iconmangrenadier_ca.edds";
	protected string m_sMan           = "{71ED761DF5BA041C}UI\Textures\HUD\Modded\Icons\Iconman_ca.edds";
	
	// A hashmap that is modified only on the authority.
	protected ref map<string,string> m_mAuthorityPlayerMap = new map<string,string>;
	
	// A hashmap that is modified only on each client by a .BumpMe from the authority.
	protected ref map<string,string> m_mLocalPlayerMap = new map<string,string>;
	
	// A array where we keep the local clients current group stored and sorted by the value determined for each player.
	protected ref array<string> m_aLocalGroupArray = new array<string>;
	
	// A array we use primarily for replication of m_mAuthorityPlayerMap to m_mLocalPlayerMap.
	[RplProp(onRplName: "UpdateLocalPlayerMapAndGroupArray")]
	protected ref array<string> m_aPlayerArray = new array<string>;
	
	// The vanilla group manager.
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent = null;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------

	static COA_GroupDisplayManagerComponent GetInstance() 
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return COA_GroupDisplayManagerComponent.Cast(gameMode.FindComponent(COA_GroupDisplayManagerComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		if (Replication.IsClient()) {
			GetGame().GetCallqueue().CallLater(UpdateLocalGroupArray, 450, true);
		};
		
		if (Replication.IsServer()) {
			GetGame().GetCallqueue().CallLater(UpdateGroupInfoInAuthorityPlayerMap, 435, true);
			GetGame().GetCallqueue().CallLater(UpdatePlayerArray, 425, true);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnGameEnd()
	{	
		super.OnGameEnd();
		
		if (Replication.IsClient()) {
			GetGame().GetCallqueue().Remove(UpdateLocalGroupArray);
		};
		
		if (Replication.IsServer()) {
			GetGame().GetCallqueue().Remove(UpdateGroupInfoInAuthorityPlayerMap);
			GetGame().GetCallqueue().Remove(UpdatePlayerArray);
		};
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to replicate and store values to each clients m_mLocalPlayerMap

	//------------------------------------------------------------------------------------------------
	
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
	protected void UpdateLocalPlayerMapAndGroupArray()
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
	string ReturnLocalPlayerMapValue(int groupID, int playerID, string key)
	{	
		// Get the players key 
		key = string.Format("%1 %2 %3", groupID, playerID, key);
		return m_mLocalPlayerMap.Get(key);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to sort and store the current group array we want to show on players screens.

	//------------------------------------------------------------------------------------------------
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdateLocalGroupArray()
	{
		if (!m_GroupsManagerComponent)
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		// Get players current group.
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (!playersGroup) return;
		
		array<int> playerIDsArray = playersGroup.GetPlayerIDs();
		int groupCount = playerIDsArray.Count();
		
		int groupID = playersGroup.GetGroupID();
		
		// If group count is less than what we need, better clear the array so players clear their displays.
		if (groupCount <= 1) {m_aLocalGroupArray.Clear(); return;};
		
		// Settup a temp array we'll use to keep all changes local so if a player calls for m_aLocalGroupArray it isn't in the middle of being updated.
		array<string> tempLocalGroupArray = {};
			
		foreach (int playerID : playerIDsArray) {
			IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!localplayer) continue;
			
			string colorTeam   = ReturnLocalPlayerMapValue(groupID, playerID, "ColorTeam");
			string icon        = ReturnLocalPlayerMapValue(groupID, playerID, "DisplayIcon");
			string playerValue = ReturnLocalPlayerMapValue(groupID, playerID, "PlayerValue");
			
			// If any of the above variables don't exist, better continue onto the next loop.
			if (!colorTeam || colorTeam == "" || !icon || icon == "" || !playerValue || playerValue == "") continue;
			
			// Format a string with what we need for displaying a player.
			string playerStr = string.Format("%1╣%2╣%3╣%4", playerValue, playerID, colorTeam, icon);
			tempLocalGroupArray.Insert(playerStr);
		}
		
		if (tempLocalGroupArray.Count() <= 1) {m_aLocalGroupArray.Clear(); return;};
			
		tempLocalGroupArray.Sort(false);
		m_aLocalGroupArray = tempLocalGroupArray;
	};
	
	//------------------------------------------------------------------------------------------------
	TStringArray GetLocalGroupArray()
	{	
		return m_aLocalGroupArray;
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to update m_mAuthorityPlayerMap.

	//------------------------------------------------------------------------------------------------
	
	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void UpdateAuthorityPlayerMap(int groupID, int playerID, string write, string value)
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

	// Functions for the primary loop that determines a players icon and value.

	//------------------------------------------------------------------------------------------------
	
	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdateGroupInfoInAuthorityPlayerMap()
	{	
		array<SCR_AIGroup> outAllGroups;

		// Get base group manager component
		if (!m_GroupsManagerComponent)
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (!m_GroupsManagerComponent) return;
		
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
				if (!playerColorTeam || playerColorTeam == "") UpdateAuthorityPlayerMap(groupID, localPlayerID, "ColorTeam", "None");

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
		
					// Settup new arrays and variables
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
				UpdateAuthorityPlayerMap(groupID, localPlayerID, "DisplayIcon", icon);
		
				// If player is not in a vehicle seat, make sure to store their Icon so we can use it for other menus/sorting of the players.
				if (icon != m_sCargo && icon != m_sDriver && icon != m_sGunner) {
					string playerStoredSpecialtyIcon = ReturnAuthorityPlayerMapValue(groupID, localPlayerID, "StoredSpecialtyIcon");
					
					// In case player spawns in a vehicle/state where we can't read his inventory, use m_sMan to keep all menus/functions happy.
					if (!playerStoredSpecialtyIcon || playerStoredSpecialtyIcon == "") UpdateAuthorityPlayerMap(groupID, localPlayerID, "StoredSpecialtyIcon", m_sMan);
					
					// Update StoredSpecialtyIcon.
					UpdateAuthorityPlayerMap(groupID, localPlayerID, "StoredSpecialtyIcon", icon);
				}
				
				// Determine players value by their color team and icon so we can sort players from most to least valuable in the group display (definitely not racist).
				int playerValue = DeterminePlayerValue(groupID, localPlayerID, playerColorTeam);
				
				// Update PlayerValue
				UpdateAuthorityPlayerMap(groupID, localPlayerID, "PlayerValue", playerValue.ToString());
			};
		};
	};
	
	//------------------------------------------------------------------------------------------------
	int DeterminePlayerValue(int groupID, int playerID, string colorTeam)
	{
		// Settup value variable.
		int value = 0;
		
		// Get player StoredSpecialtyIcon so we aren't influenced if the player is in a vehicle.
		string icon = ReturnAuthorityPlayerMapValue(groupID, playerID, "StoredSpecialtyIcon");

		// Sort player by their color so we can group color teams together (a lil bit racist).
		switch (colorTeam) {
			case "Red"    : {value = -3;  break;};
			case "Blue"   : {value = -5;  break;};
			case "Yellow" : {value = -7;  break;};
			case "Green"  : {value = -9;  break;};
			case "None"   : {value = 2;   break;};
		};
				
		switch (true) {
			// If the players is currently the SL, make him the most valuable player in the list
			case (icon == m_sSquadLeader)                       : {value = -1; break;};
			// Add/Remove value from a player if they're a Team Lead
			case (icon == m_sTeamLeader && colorTeam == "None") : {value--;    break;};
			case (icon == m_sTeamLeader && colorTeam != "None") : {value++;    break;};
		};
		
		// Return how valuable the player is
		return value;
	}
}
