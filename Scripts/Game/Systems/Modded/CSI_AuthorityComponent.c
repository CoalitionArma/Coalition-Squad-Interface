[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class CSI_AuthorityComponentClass : SCR_BaseGameModeComponentClass {};

class CSI_AuthorityComponent : SCR_BaseGameModeComponent
{
	// A hashmap that is modified only on the authority.
	protected ref map<string, string> m_mUpdateAuthoritySettingsMap = new map<string, string>;

	// A array we use to broadcast whenever a change happens to any of the server overrides.
	[RplProp(onRplName: "UpdateLocalSettings")]
	ref array<string> m_aServerOverridesArray = new array<string>;

	// A hashmap that is modified only on the authority.
	protected ref map<string, string> m_mAuthorityPlayerMap = new map<string, string>;

	// A hashmap that is modified only on each client by a .BumpMe from the authority.
	protected ref map<string, string> m_mLocalPlayerMap = new map<string, string>;

	// A array we use primarily for replication of m_mAuthorityPlayerMap to m_mLocalPlayerMap.
	[RplProp(onRplName: "UpdateLocalPlayerMap")]
	protected ref array<string> m_aPlayerArray = new array<string>;

	// The vanilla group manager.
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

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
			UpdateAuthoritySettingArray();
			
			GetGame().GetCallqueue().CallLater(UpdateAllGroupStrings, 685, true);
			GetGame().GetCallqueue().CallLater(CleanUpAuthorityPlayerMap, 480000, true); // Updates every 8min (480000ms)
		};
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnGameEnd()
	{
		super.OnGameEnd();
		
		if (Replication.IsServer())
		{
			GetGame().GetCallqueue().Remove(UpdateAllGroupStrings);
			GetGame().GetCallqueue().Remove(CleanUpAuthorityPlayerMap);
		};
	}

	//------------------------------------------------------------------------------------------------

	// Functions to replicate and store values to each clients m_mLocalPlayerMap

	//------------------------------------------------------------------------------------------------

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	string ReturnLocalPlayerMapValue(int groupID, int playerID, string key)
	{
		// Get the players key
		key = string.Format("%1%2%3", groupID, playerID, key);
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
			
			tempPlayerArray.Insert(string.Format("%1~%2", key, value));
		};

		// Replicate m_aPlayerArray to all clients.
		m_aPlayerArray = tempPlayerArray;
		Replication.BumpMe();
		
		//Adds support for listen servers
		UpdateLocalPlayerMap();
	}

	//- Client -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdateLocalPlayerMap()
	{
		// Fill m_mLocalPlayerMap with all keys and values from m_mAuthorityPlayerMap.
		foreach (string playerKeyAndValueToSplit : m_aPlayerArray)
		{
			array<string> playerKeyAndValueArray = {};
			playerKeyAndValueToSplit.Split("~", playerKeyAndValueArray, false);
			m_mLocalPlayerMap.Set(playerKeyAndValueArray[0], playerKeyAndValueArray[1]);
		};
		
		CSI_ClientComponent clientComponent = CSI_ClientComponent.GetInstance();
		if (!clientComponent) return;
		clientComponent.UpdateLocalGroupArray();
	}

	//------------------------------------------------------------------------------------------------

	// Functions to update m_mAuthorityPlayerMap.

	//------------------------------------------------------------------------------------------------
	
	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	string ReturnAuthorityPlayerMapValue(int groupID, int playerID, string key)
	{
		// Get the players key
		key = string.Format("%1%2%3", groupID, playerID, key);
		return m_mAuthorityPlayerMap.Get(key);
	}

	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void UpdateAuthorityPlayerMapValue(int groupID, int playerID, string write, string value)
	{
		// The key we are gonna use that keeps everything local to the group and player, so we don't get any cross-contamination between groups or players.
		string key = string.Format("%1%2%3", groupID, playerID, write);
		m_mAuthorityPlayerMap.Set(key, value);
	}

	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	protected void CleanUpAuthorityPlayerMap()
	{
		map<string, string> tempMap = new map<string, string>;
		array<int> outPlayers = new array<int>;

		GetGame().GetPlayerManager().GetPlayers(outPlayers);
		
		foreach (int playerID : outPlayers) 
		{
			SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(playerID);
			
			if (!playersGroup) continue;

			int groupID = playersGroup.GetGroupID();
			
			// CT = ColorTeam | OI = OverrideIcon | DI = DisplayIcon | SSI = StoredSpecialtyIcon | PV = PlayerValue | PR = PlayerRank
			array<string> playerValuesArray = {"CT", "OI", "DI", "SSI", "PV", "PR"};
			
			foreach (string value : playerValuesArray) 
			{
				string hashValue = ReturnAuthorityPlayerMapValue(groupID, playerID, value);
				string key = string.Format("%1%2%3", groupID, playerID, value);
				tempMap.Set(key, hashValue);
			}
		};

		m_mAuthorityPlayerMap.Clear();

		m_mAuthorityPlayerMap = tempMap;
	}

	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllGroupStrings()
	{	
		if (!ReturnAuthoritySettings()[1] && !ReturnAuthoritySettings()[2] && !ReturnAuthoritySettings()[7]) return;
		
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
				string playerValue = DetermineLocalPlayerValue(groupID, localPlayerID).ToString(); // Determine players value by their color team and icon so we can sort players from most to least valuable in the group display (definitely not racist).
				
				if (playerValue.IsEmpty() || playerValue == "0") continue;
				
				// Format a string with what we need for displaying/sorting a player.
				string playerStr = string.Format("%1:%2", playerValue, localPlayerID);

				if (playerValue != ReturnAuthorityPlayerMapValue(groupID, localPlayerID, "PV"))
					UpdateAuthorityPlayerMapValue(groupID, localPlayerID, "PV", playerValue);
				
				tempLocalGroupArray.Insert(playerStr);
			};

			tempLocalGroupArray.Sort(false);

			foreach (string playerStr : tempLocalGroupArray) 
			{
				if (groupString.IsEmpty()) 
					groupString = playerStr;
				else
					groupString = string.Format("%1|%2", groupString, playerStr);
			}

			// Update GroupString.
			if (groupString != ReturnAuthorityPlayerMapValue(groupID, -1, "GS"))
				UpdateAuthorityPlayerMapValue(groupID, -1, "GS", groupString); // GS = GroupString
		};

		//Once we've updated all values, propagate them to all clients with UpdatePlayerArray.
		UpdatePlayerArray();
	}
	
	//------------------------------------------------------------------------------------------------
	int DetermineLocalPlayerValue(int groupID, int localPlayerID)
	{
		// Setup value variable.
		int value = 0;
		
		string icon = ReturnAuthorityPlayerMapValue(groupID, localPlayerID, "SSI");
		string colorTeam = ReturnLocalPlayerMapValue(groupID, localPlayerID, "CT"); // CT = ColorTeam

		// Sort player by their color so we can group color teams together (a lil bit racist).
		switch (colorTeam) 
		{
			case "R" : {value = -3; break;};
			case "B" : {value = -5; break;};
			case "Y" : {value = -7; break;};
			case "G" : {value = -9; break;};
			default  : {value = 2;  break;};
		};

		switch (true) 
		{
			// If the players is currently the SL, make him the most valuable player in the list
			case (icon == "SL")                          : {value = -1; break;};

			// Add/Remove value from a player if they're a Team Lead
			case (icon == "FTL" && colorTeam == "N/A") : {value--;    break;};
			case (icon == "FTL" && colorTeam != "N/A") : {value++;    break;};
		};

		// Return how valuable the player is
		return value;
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
	
	//- Client -\\
	//------------------------------------------------------------------------------------------------
	void UpdateLocalSettings()
	{
		CSI_ClientComponent clientComponent = CSI_ClientComponent.GetInstance();
		if (!clientComponent) return;
		clientComponent.UpdateLocalCSISettingArray();
	};

	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void UpdateAuthoritySetting(string setting, string value)
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);
		
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
		
		UpdateAuthoritySettingArray();
	}

	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void UpdateAuthoritySettingArray()
	{
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
			string checkValue = "";
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(serverOverride, checkValue);
			m_aServerOverridesArray.Insert(checkValue);
		}
		Replication.BumpMe();
		
		UpdateLocalSettings();
	}
}
