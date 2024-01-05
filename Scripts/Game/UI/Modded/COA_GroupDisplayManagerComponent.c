[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class COA_GroupDisplayManagerComponentClass: SCR_BaseGameModeComponentClass {};

class COA_GroupDisplayManagerComponent : SCR_BaseGameModeComponent
{	
	protected ref map<string,string> m_mAuthorityPlayerMap = new map<string,string>;
	
	protected ref map<string,string> m_mLocalPlayerMap = new map<string,string>;
	
	[RplProp(onRplName: "UpdateLocalPlayerMap")]
	protected ref array<string> PlayerArray = new array<string>;
	
	protected string m_sCargo         = "{B910A93F355F168C}Layouts\UI\Textures\Icons\imagecargo_ca.edds";
	protected string m_sDriver        = "{C2B2F451FB157A89}Layouts\UI\Textures\Icons\imagedriver_ca.edds";
	protected string m_sGunner        = "{3DAAB773C8C29812}Layouts\UI\Textures\Icons\imagegunner_ca.edds";
	protected string m_sSquadLeader   = "{5ECE094ED4662B33}Layouts\UI\Textures\Icons\iconmanleader_ca.edds";
	protected string m_sTeamLeader    = "{6D45BA2CCC322312}Layouts\UI\Textures\Icons\iconmanteamleader_ca.edds";
	protected string m_sMedic         = "{01F2523A4EE5C48B}Layouts\UI\Textures\Icons\iconmanmedic_ca.edds";
	protected string m_sSniper        = "{318B797C57BE3C29}Layouts\UI\Textures\Icons\iconmansniper_ca.edds";
	protected string m_sMachineGunner = "{CCF40410BDB53870}Layouts\UI\Textures\Icons\iconmanmg_ca.edds";
	protected string m_sAntiTank      = "{DC86195B44F5A345}Layouts\UI\Textures\Icons\iconmanat_ca.edds";
	protected string m_sGrenadier     = "{B7757F2024A3DC87}Layouts\UI\Textures\Icons\iconmangrenadier_ca.edds";
	protected string m_sMan           = "{71ED761DF5BA041C}Layouts\UI\Textures\Icons\iconman_ca.edds";
	
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
	
	override protected void OnPostInit(IEntity owner)
	{	
		if (!Replication.IsServer()) return;
		
		GetGame().GetCallqueue().CallLater(UpdateGroupInfoInAuthorityPlayerMap, 85, true);
		
		GetGame().GetCallqueue().CallLater(UpdatePlayerArray, 125, true);
	}
	
	//------------------------------------------------------------------------------------------------

	// Functions to store and replicate a map in which we store custom player values

	//------------------------------------------------------------------------------------------------
	
	//- Clients
	
	protected void UpdatePlayerArray()
	{
		protected ref array<string> TempPlayerArray = new array<string>;
		for (int i = 0; i<m_mAuthorityPlayerMap.Count(); i++)
		{
			string key = m_mAuthorityPlayerMap.GetKey(i);
			string value = m_mAuthorityPlayerMap.Get(key);
			
			string mapValueString = string.Format("%1╢%2", key, value);
			TempPlayerArray.Insert(mapValueString);
		};
		PlayerArray = TempPlayerArray;
	}
	
	protected void UpdateLocalPlayerMap()
	{
	 	foreach (string PlayerKeyAndValue : PlayerArray)
		{
			array<string> PlayerKeyAndValueSplit = {};
			PlayerKeyAndValue.Split("╢", PlayerKeyAndValueSplit, false);
			
			m_mLocalPlayerMap.Set(PlayerKeyAndValueSplit[0], PlayerKeyAndValueSplit[1]);
		};
	}
	
	string ReturnLocalPlayerMapValue(string key, int playerID)
	{	
		string checkKey = string.Format("%1 %2", playerID, key);
		return m_mLocalPlayerMap.Get(checkKey);
	}
	
	//- Server
	
	void UpdateAuthorityPlayerMap(int playerID, string write, string value)
	{						
		string key = string.Format("%1 %2", playerID, write);
							
		m_mAuthorityPlayerMap.Set(key, value);
	}
	
	protected void UpdateGroupInfoInAuthorityPlayerMap()
	{	
		array<SCR_AIGroup> outAllGroups;

		SCR_GroupsManagerComponent vanillaGroupManager = SCR_GroupsManagerComponent.GetInstance();
		
		if (!vanillaGroupManager) return;
		
		vanillaGroupManager.GetAllPlayableGroups(outAllGroups);
		
		foreach (SCR_AIGroup playersGroup : outAllGroups)
		{
			if (!playersGroup) continue;
			
			array<string> groupStringArray = {};
	
			// Get list of all the players we have to parse through.
			array<int> groupPlayersIDs = playersGroup.GetPlayerIDs();
			
			if (groupPlayersIDs.Count() == 1 || groupPlayersIDs.Count() == 0) continue;
	
			// Parse through current group array.
			foreach(int localPlayerID : groupPlayersIDs) 
			{
				IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerID);
				string playerName = GetGame().GetPlayerManager().GetPlayerName(localPlayerID);
	
				// Skip ahead to next for-loop iteration if any of these are false.
				if (!playerName || !localplayer) return;
				
				array<string> iconArray = {};

				//------------------------------------------------------------------------------------------------
				// Color Teams
				//------------------------------------------------------------------------------------------------

				string playerColorTeam = ReturnLocalPlayerMapValue("ColorTeam", localPlayerID);
				if (!playerColorTeam || playerColorTeam == "") {playerColorTeam = "None"};

				//------------------------------------------------------------------------------------------------
				// Vehicle Icons, they supercede any other icon
				//------------------------------------------------------------------------------------------------

				// Check if player is in a vehicle.
				CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(localplayer.FindComponent(CompartmentAccessComponent));
				if (compartmentAccess)
				{
					// Check players current compartment.
					BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
					if (compartment)
					{
						// Check players current compartment type, then assign his icon.
						ECompartmentType compartmentType = SCR_CompartmentAccessComponent.GetCompartmentType(compartment);
						switch (compartmentType)
						{
							case ECompartmentType.Cargo  : { iconArray.Insert(m_sCargo);  break; };
							case ECompartmentType.Pilot  : { iconArray.Insert(m_sDriver); break; };
							case ECompartmentType.Turret : { iconArray.Insert(m_sGunner); break; };
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
					//playerColorTeam = "None";
				};

				//------------------------------------------------------------------------------------------------
				// Override regular Icons If Set By SL
				//------------------------------------------------------------------------------------------------

				string playerOverideIcon = ReturnLocalPlayerMapValue("OverrideIcon", localPlayerID);
	
				if (playerOverideIcon && playerColorTeam != "" && iconArray.IsEmpty()) {
					switch (playerOverideIcon)
						{
						case "Team Lead"      : { iconArray.Insert(m_sTeamLeader);    break; };
						case "Medic"          : { iconArray.Insert(m_sMedic);         break; };
						case "Sniper"         : { iconArray.Insert(m_sSniper);        break; };
						case "Machine Gunner" : { iconArray.Insert(m_sMachineGunner); break; };
						case "Anti-Tank"      : { iconArray.Insert(m_sAntiTank);      break; };
						case "Grenadier"      : { iconArray.Insert(m_sGrenadier);     break; };
						case "Man"            : { iconArray.Insert(m_sMan);           break; };
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
					array<EWeaponType> WeaponTypeArray = {};
					array<SCR_EConsumableType> MedicalTypeArray = {};
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
								MedicalTypeArray.Insert(medicalType);
							};
						};
						// Check if item is a Weapon.
						WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
						if (weaponComp) {
							// Get the weapons type and insert it into the weapon array so we can read it later.
							WeaponTypeArray.Insert(weaponComp.GetWeaponType());
							// Get muzzle types (so we can detect something like a underslung grenade launcher)
							for (int m = 0, mCount = weaponComp.GetMuzzlesList(muzzles); m < mCount; m++)
							{
								// Convert muzzle types to weapon types and insert it into the weapon array so we can read it later. (ToDo: Not hardcoded?)
								switch (muzzles[m].GetMuzzleType())
								{
									case EMuzzleType.MT_RPGMuzzle : { WeaponTypeArray.Insert(EWeaponType.WT_ROCKETLAUNCHER);  break; };
									case EMuzzleType.MT_UGLMuzzle : { WeaponTypeArray.Insert(EWeaponType.WT_GRENADELAUNCHER); break; };
								};
							};
						};
					};
					// Take all the data we just collected and assign players a icon based on if it exists in the weapon/medical arrays.
					switch (true)
					{
						case (MedicalTypeArray.Find(SCR_EConsumableType.SALINE) != -1)    : { iconArray.Insert(m_sMedic);         break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_SNIPERRIFLE) != -1)     : { iconArray.Insert(m_sSniper);        break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_MACHINEGUN) != -1)      : { iconArray.Insert(m_sMachineGunner); break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_ROCKETLAUNCHER) != -1)  : { iconArray.Insert(m_sAntiTank);      break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_GRENADELAUNCHER) != -1) : { iconArray.Insert(m_sGrenadier);     break; };
						default                                                           : { iconArray.Insert(m_sMan);                  };
					};
				};
		
				string icon = iconArray[0];
				
				string localPlayerIDStr = localPlayerID.ToString();
				string playerString = string.Format("%1║%2║%3║%4║%5", playerName, playerColorTeam, icon, localPlayerID, playerOverideIcon);

				if (icon != m_sCargo && icon != m_sDriver && icon != m_sGunner) {
					string playerStoredIcon = ReturnLocalPlayerMapValue("StoredIcon", localPlayerID);
		
						if (playerStoredIcon != icon) {
						UpdateAuthorityPlayerMap(localPlayerID, "StoredIcon", icon);
					};
				}

				int value = 0;

				switch (playerColorTeam) {
					case "Red"    : { value = -3;  break; };
					case "Blue"   : { value = -5;  break; };
					case "Yellow" : { value = -7;  break; };
					case "Green"  : { value = -9;  break; };
					case "None"   : { value = 2;   break; };
				};

				if (icon == m_sCargo || icon == m_sDriver || icon == m_sGunner) {
					string playerSI = ReturnLocalPlayerMapValue("StoredIcon", localPlayerID);
					if (playerSI) {icon = playerSI};
				};

				switch (true) {
					case (icon == m_sSquadLeader) : {value = -1; break; };
					case (icon == m_sTeamLeader && playerColorTeam == "None") : {value--; break; };
					case (icon == m_sTeamLeader && playerColorTeam != "None") : {value++; break; };
				};

				string playerValueString = string.Format("%1╣%2", value, playerString);
				UpdateAuthorityPlayerMap(localPlayerID, "PlayerGroupValues", playerValueString);
			};
		};
	};
}
