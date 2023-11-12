modded class SCR_GroupsManagerComponentClass : SCR_BaseGameModeComponentClass
{
	[RplProp(onRplName: "RpcAsk_Authority_Method")]
	protected ref map<string,string> m_mStoredPlayerValues = new map <string,string>;
	
	void UpdateUIClassValue(int playerIDUI, string writeUpdate, string valueUpdate)
	{
		string key = string.Format("%1 %2", playerIDUI, writeUpdate);
		m_mStoredPlayerValues.Remove(key);
		m_mStoredPlayerValues.Insert(key, valueUpdate);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Authority_Method()
	{
		m_mStoredPlayerValues = m_mStoredPlayerValues;
		Replication.BumpMe();
	}

	map<string,string> ReturnStoredPlayerValues()
	{
		return m_mStoredPlayerValues;
	}
};

modded class SCR_GroupsManagerComponent : SCR_BaseGameModeComponent
{
	protected ref map<string,string> m_mLocallyStoredPlayerValues = new map <string,string>;
	protected ref map<string,string> m_mGroupMasterMap = new map<string,string>;

	protected string m_sCargo = "{B910A93F355F168C}Layouts\UI\Textures\Icons\imagecargo_ca.edds";
	protected string m_sDriver = "{C2B2F451FB157A89}Layouts\UI\Textures\Icons\imagedriver_ca.edds";
	protected string m_sGunner = "{3DAAB773C8C29812}Layouts\UI\Textures\Icons\imagegunner_ca.edds";
	protected string m_sSquadLeader = "{5ECE094ED4662B33}Layouts\UI\Textures\Icons\iconmanleader_ca.edds";
	protected string m_sTeamLeader = "{6D45BA2CCC322312}Layouts\UI\Textures\Icons\iconmanteamleader_ca.edds";
	protected string m_sMedic = "{01F2523A4EE5C48B}Layouts\UI\Textures\Icons\iconmanmedic_ca.edds";
	protected string m_sSniper = "{318B797C57BE3C29}Layouts\UI\Textures\Icons\iconmansniper_ca.edds";
	protected string m_sMachineGunner = "{CCF40410BDB53870}Layouts\UI\Textures\Icons\iconmanmg_ca.edds";
	protected string m_sAntiTank = "{DC86195B44F5A345}Layouts\UI\Textures\Icons\iconmanat_ca.edds";
	protected string m_sGrenadier = "{B7757F2024A3DC87}Layouts\UI\Textures\Icons\iconmangrenadier_ca.edds";
	protected string m_sMan = "{71ED761DF5BA041C}Layouts\UI\Textures\Icons\iconman_ca.edds";
	
	void UpdateUIvalue(int playerIDUI, string writeUpdate, string valueUpdate)
	{
		SCR_GroupsManagerComponentClass classData = SCR_GroupsManagerComponentClass.Cast(GetComponentData(GetOwner()));
		classData.UpdateUIClassValue(playerIDUI, writeUpdate, valueUpdate);
	}

	string ReturnUIValue(int playerIDUIReturn, string check)
	{	
		string checkKey = string.Format("%1 %2", playerIDUIReturn, check);
		return m_mLocallyStoredPlayerValues.Get(checkKey);
	}
	
	void UpdateGroupMapValue(string GrpString, string masterString) 
	{
		m_mGroupMasterMap.Remove(GrpString);
		m_mGroupMasterMap.Insert(GrpString, masterString);
	}

	string ReturnGroupMapValue(string groupReturn)
	{
		string localGroupString = m_mGroupMasterMap.Get(groupReturn);
		return localGroupString;
	}

	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		GetGame().GetCallqueue().CallLater(UpdateGroupValues, 65, true, owner);
	}
	
	void UpdateGroupValues(IEntity owner)
	{	
		SCR_GroupsManagerComponentClass classData = SCR_GroupsManagerComponentClass.Cast(GetComponentData(owner));
		m_mLocallyStoredPlayerValues = classData.ReturnStoredPlayerValues();
		
		array<SCR_AIGroup> outAllGroups;

		GetAllPlayableGroups(outAllGroups);

		foreach (SCR_AIGroup playersGroup : outAllGroups)
		{
			if (!playersGroup) continue;
			string GrpString = playersGroup.ToString();

			array<string> groupStringArray = {};

			// Get list of all the players we have to parse through.
			array<int> groupPlayersIDs = playersGroup.GetPlayerIDs();
			//groupPlayersIDs = {1, 1}; //Debugging

			if (groupPlayersIDs.Count() == 1 || groupPlayersIDs.Count() == 0) {
				m_mGroupMasterMap.Remove(GrpString);
				m_mGroupMasterMap.Insert(GrpString, "");
				continue;
			};

			int groupLeaderID = playersGroup.GetLeaderID();
	
			// Parse through current group array.
			foreach (int localPlayerID : groupPlayersIDs)
			{
				// Get target players entity and player name.
				IEntity localplayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerID);
				string playerName = GetGame().GetPlayerManager().GetPlayerName(localPlayerID);
				array<string> iconArray = {};

				// Skip ahead to next for-loop iteration if any of these are false.
				if (!playerName || !localplayer) continue;

				//------------------------------------------------------------------------------------------------
				// Color Teams
				//------------------------------------------------------------------------------------------------

				string playerColorTeam = ReturnUIValue(localPlayerID, "ColorTeam");
				if (!playerColorTeam || playerColorTeam == "") {playerColorTeam = "None"};

				//------------------------------------------------------------------------------------------------
				// Vehicle Icons, they supercede any other icon
				//------------------------------------------------------------------------------------------------

				// Check if target player is in a vehicle.
				CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(localplayer.FindComponent(CompartmentAccessComponent));
				if (compartmentAccess)
				{
					// Check target players current compartment.
					BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
					if (compartment)
					{
						// Check target players current compartment type, then assign his icon.
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
				
				// Check if current target player is the current squad leader.
				if (localPlayerID == groupLeaderID && iconArray.IsEmpty())
				{
					// Set Squad Leader Icon
					iconArray.Insert(m_sSquadLeader);
					//playerColorTeam = "None";
				};

				//------------------------------------------------------------------------------------------------
				// Override regular Icons If Set By SL
				//------------------------------------------------------------------------------------------------

				string playerOverideIcon = ReturnUIValue(localPlayerID, "OverrideIcon");

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

				// Check if current target player is the current squad leader.
				if (localPlayerID == groupLeaderID && iconArray.IsEmpty())
				{
					// Set Squad Leader Icon
					iconArray.Insert(m_sSquadLeader);
					//playerColorTeam = "None";
				};

				if (iconArray.IsEmpty()) {
					// Get target players inventory component
					SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(localplayer.FindComponent(SCR_InventoryStorageManagerComponent));

					// Get all of target players inventory items
					array<IEntity> allPlayerItems = {};
					characterInventory.GetAllRootItems(allPlayerItems);

					// Settup new arrays and variables
					//
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
									case EMuzzleType.MT_RPGMuzzle : {
										WeaponTypeArray.Insert(EWeaponType.WT_ROCKETLAUNCHER);
										break;
									};
									case EMuzzleType.MT_UGLMuzzle : {
										WeaponTypeArray.Insert(EWeaponType.WT_GRENADELAUNCHER);
										break;
									};
								};
							};
						};
					};
					// Take all the data we just collected and assign players a icon based on if it exists in the weapon/medical arrays.
					switch (true)
					{
						// Medic
						case (MedicalTypeArray.Find(SCR_EConsumableType.SALINE) != -1)    : { iconArray.Insert(m_sMedic);         break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_SNIPERRIFLE) != -1)     : { iconArray.Insert(m_sSniper);        break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_MACHINEGUN) != -1)      : { iconArray.Insert(m_sMachineGunner); break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_ROCKETLAUNCHER) != -1)  : { iconArray.Insert(m_sAntiTank);      break; };
						case (WeaponTypeArray.Find(EWeaponType.WT_GRENADELAUNCHER) != -1) : { iconArray.Insert(m_sGrenadier);     break; };
						default : { iconArray.Insert(m_sMan); };
					};
				};
				string playerString = string.Format("%1;%2;%3;%4;", playerName, playerColorTeam, iconArray[0], localPlayerID);
				groupStringArray.Insert(playerString);

				if (iconArray[0] != m_sCargo && iconArray[0] != m_sDriver && iconArray[0] != m_sGunner) {
					string playerStoredIcon = ReturnUIValue(localPlayerID, "StoredIcon");
					if (playerStoredIcon != iconArray[0]) {
						UpdateUIvalue(localPlayerID, "StoredIcon", iconArray[0]);
					};
				}
			};

			array<string> arrStr = {};

			foreach (int i, string playerGroupString : groupStringArray)
			{
				array<string> pLocStrArr = {};
				playerGroupString.Split(";", pLocStrArr, true);
				string colorTeam = pLocStrArr[1];
				string icon = pLocStrArr[2];
				string locPlayerID = pLocStrArr[3];
				int locPlayerIDInt = locPlayerID.ToInt();
				int value = 0;

				switch (colorTeam) {
					case "Red"    : { value = 2;  break; };
					case "Blue"   : { value = 4;  break; };
					case "Yellow" : { value = 6;  break; };
					case "Green"  : { value = 8;  break; };
					case "None"   : { value = -3; break; };
				};


				if (icon == m_sCargo || icon == m_sDriver || icon == m_sGunner) {
					string playerSI = ReturnUIValue(locPlayerIDInt, "StoredIcon");
					if (playerSI) {icon = playerSI};
				};

				switch (true) {
					case (icon == m_sSquadLeader) : {value = -1; break; };
					case (icon == m_sTeamLeader && colorTeam == "None") : {value++; break; };
					case (icon == m_sTeamLeader && colorTeam != "None") : {value--; break; };
				};

				string playerValueString = string.Format("%1|%2|", value, playerGroupString);
				arrStr.Insert(playerValueString);
			};

			arrStr.Sort(false);
			string masterString = "";

			foreach (string pStr : arrStr)
			{
				array<string> pStrArr = {};
				pStr.Split("|", pStrArr, true);
				masterString = masterString + pStrArr[1];
			};

			string localGroupStringCheck = m_mGroupMasterMap.Get(GrpString);
			if (localGroupStringCheck && localGroupStringCheck == masterString) continue;
			UpdateGroupMapValue(GrpString, masterString);
		};
	}
}
