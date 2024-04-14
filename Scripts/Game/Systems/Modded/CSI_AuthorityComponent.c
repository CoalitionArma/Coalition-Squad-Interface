[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class CSI_AuthorityComponentClass : SCR_BaseGameModeComponentClass {};

class CSI_AuthorityComponent : SCR_BaseGameModeComponent
{
	// A hashmap that is modified only on the authority.
	protected ref map<string, string> m_mUpdateAuthoritySettingsMap = new map<string, string>;
	
	// A hashmap that is modified only on the local user.
	protected ref map<string, string> m_mUpdateClientSettingsMap = new map<string, string>;
	
	// A array where we hold all local user settings.
	protected ref array<string> m_aLocalCSISettingsArray = new array<string>;

	// A array we use to broadcast whenever a change happens to any of the server overrides.
	[RplProp()]
	ref array<string> m_aServerOverridesArray = new array<string>;

	// A hashmap that is modified only on the authority.
	protected ref map<string, string> m_mAuthorityPlayerMap = new map<string, string>;

	// A hashmap that is modified only on each client by a .BumpMe from the authority.
	protected ref map<string, string> m_mLocalPlayerMap = new map<string, string>;

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

		if (Replication.IsServer()) 
		{
			CheckAuthoritySettings();
			
			GetGame().GetCallqueue().CallLater(UpdateAllGroupStrings, 850, true);
			GetGame().GetCallqueue().CallLater(CheckAuthoritySettings, 5000, true);
			GetGame().GetCallqueue().CallLater(CleanUpAuthorityPlayerMap, 480000, true); // Updates every 8min (480000ms)
		};
		
		CheckClientCSISettings();
		GetGame().GetCallqueue().CallLater(CheckClientCSISettings, 2850, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnGameEnd()
	{
		super.OnGameEnd();
		
		if (Replication.IsServer()) 
		{
			GetGame().GetCallqueue().Remove(UpdateAllGroupStrings);
			GetGame().GetCallqueue().Remove(CheckAuthoritySettings);
			GetGame().GetCallqueue().Remove(CleanUpAuthorityPlayerMap);
		};
		
		GetGame().GetCallqueue().Remove(CheckClientCSISettings);
	}
	
	//------------------------------------------------------------------------------------------------

	// Function for getting/setting local settings

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	TStringArray ReturnLocalCSISettings() {
		return m_aLocalCSISettingsArray;
	}

	//------------------------------------------------------------------------------------------------
	void ChangeLocalCSISetting(string setting, string value)
	{
		m_mUpdateClientSettingsMap.Set(setting, value);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckClientCSISettings()
	{
		if (m_mUpdateClientSettingsMap.Count() > 0) {
			for (int i = 0; i < m_mUpdateClientSettingsMap.Count(); i++)
			{
				string setting = m_mUpdateClientSettingsMap.GetKey(i);
				string value = m_mUpdateClientSettingsMap.Get(setting);

				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);
			};
			m_mUpdateClientSettingsMap.Clear();
			GetGame().UserSettingsChanged();
			GetGame().SaveUserSettings();
		};
		
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

		foreach (int i, string setting : settingsToCheck)
		{
			string settingValue = "";
			string settingServerOverride = "";
			if (i < 9 && !ReturnAuthoritySettings().IsEmpty()) {
				settingServerOverride = ReturnAuthoritySettings()[i];
			};
			switch (true)
			{
				case(!(settingServerOverride.IsEmpty() || settingServerOverride == "N/A")) : {settingValue = settingServerOverride; break; };
				default : {
					GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(setting, settingValue); 
					if (i < 9 && settingValue.IsEmpty() && ReturnAuthoritySettings()[9] == "true") 
					{
						 settingValue = ReturnAuthoritySettings()[i+10]; 
					}; 
					break; 
				};
			};
			tempLocalCSISettingsArray.Insert(settingValue);
		};
		m_aLocalCSISettingsArray = tempLocalCSISettingsArray;
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
		array<string> serverOverridesArray =
		{
			// Server Overrides
			"compassVisibleServerOverride",
			"squadRadarVisibleServerOverride",
			"groupDisplayVisibleServerOverride",
			"staminaBarVisibleServerOverride",
			"nametagsVisibleServerOverride",
			"rankVisibleServerOverride",
			"nametagsRangeServerOverride",
			"roleNametagVisibleServerOverride",
			"personalColorTeamMenuServerOverride",
			
			// Server Defaults
			"serverDefaultsActive",
			"compassVisibleServerDefault",
			"squadRadarVisibleServerDefault",
			"groupDisplayVisibleServerDefault",
			"staminaBarVisibleServerDefault",
			"nametagsVisibleServerDefault",
			"rankVisibleServerDefault",
			"nametagsRangeServerDefault",
			"roleNametagVisibleServerDefault",
			"personalColorTeamMenuServerDefault"
		};
		foreach (string serverOverride : serverOverridesArray)
		{
			string value = "";
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(serverOverride, value);
			m_aServerOverridesArray.Insert(value);
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

			tempPlayerArray.Insert(string.Format("%1╪%2", key, value));
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
			playerKeyAndValueToSplit.Split("╪", playerKeyAndValueArray, false);
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
		map<string, string> tempMap = new map<string, string>;
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
	protected void UpdateAllGroupStrings()
	{
		if ((!ReturnAuthoritySettings()[0] || !ReturnAuthoritySettings()[1]) && !ReturnAuthoritySettings()[2] && !ReturnAuthoritySettings()[7]) return;
		
		// Get base group manager component
		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (!m_GroupsManagerComponent) return;

		array<SCR_AIGroup> outAllGroups;

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

			array<string> tempLocalGroupArray = {};
			string groupString = "";

			// Parse through current group array.
			foreach (int localPlayerID : groupPlayersIDs)
			{
				string playerValue = ReturnAuthorityPlayerMapValue(groupID, localPlayerID, "PlayerValue");
				
				if (playerValue.IsEmpty()) continue;
				
				// Format a string with what we need for displaying/sorting a player.
				string playerStr = string.Format("%1╣%2", playerValue, localPlayerID);

				tempLocalGroupArray.Insert(playerStr);
			};

			tempLocalGroupArray.Sort(false);

			foreach (string playerStr : tempLocalGroupArray) {
				if (groupString.IsEmpty()) 
					groupString = playerStr;
				else
					groupString = string.Format("%1║%2", groupString, playerStr);
			}

			// Update GroupString.
			UpdateAuthorityPlayerMapValue(groupID, -1, "GroupString", groupString);
		};

		//Once we've updated all values, propagate them to all clients with UpdatePlayerArray.
		UpdatePlayerArray();
	}
}
