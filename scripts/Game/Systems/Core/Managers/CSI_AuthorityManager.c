[ComponentEditorProps(category: "GameScripted/Authority", description: "")]
class CSI_AuthorityManagerClass : SCR_BaseGameModeComponentClass {};

class CSI_AuthorityManager : SCR_BaseGameModeComponent
{	
	// A array we use to broadcast whenever a change happens to any of the server overrides.
	[RplProp(onRplName: "UpdateLocalSettings")]
	ref array<string> m_aServerOverridesArray = new array<string>;
	
	protected ref map<int, CSI_PlayerData> m_mPlayerDataMap = new map<int, CSI_PlayerData>;
	
	// Replication arrays (maps cannot be directly replicated)
	[RplProp()]
	protected ref array<int> m_aPlayerIDs = {}; 
	
	[RplProp()]
	protected ref array<ref CSI_PlayerData> m_aPlayerData = {}; 
	
	// Replication property for slotting updates
	[RplProp(onRplName: "PlayerDataUpdate")]
	protected int m_PlayerDataUpdate;

	// The vanilla group manager.
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	//------------------------------------------------------------------------------------------------
	protected static CSI_AuthorityManager m_sInstance;
	void CSI_AuthorityManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	// INITIALIZATION
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static CSI_AuthorityManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		//--- Server only
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		UpdateAuthoritySettingArray();
	}
	
	//------------------------------------------------------------------------------------------------
	// DATA AND REPLICATION HANDLING
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	CSI_PlayerData GetPlayerData(int playerID)
	{
		return m_mPlayerDataMap.Get(playerID);
	}

	//------------------------------------------------------------------------------------------------
	void RequestDataUpdate()
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		m_aPlayerIDs.Clear();
		m_aPlayerData.Clear();

		// Fill arrays with all map data
		foreach (int playerID, ref CSI_PlayerData playerData : m_mPlayerDataMap)
		{
			m_aPlayerIDs.Insert(playerID);
			m_aPlayerData.Insert(playerData);
		}

		// Update replication properties
		m_PlayerDataUpdate++;
		Replication.BumpMe();
		
		#ifdef WORKBENCH
			PlayerDataUpdate();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayerDataUpdate()
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		// Update local map from replicated arrays
		for (int i = 0; i < m_aPlayerIDs.Count(); i++)
		{
			int playerID = m_aPlayerIDs.Get(i);
			
			CSI_PlayerData newPlayerData = m_aPlayerData.Get(i);
			CSI_PlayerData oldPlayerData = m_mPlayerDataMap.Get(playerID);
			
			if(!oldPlayerData)
				m_mPlayerDataMap.Set(playerID, newPlayerData);
			else
				oldPlayerData.DataUpdate(playerID, newPlayerData);
		}
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
		CSI_ClientManager clientComponent = CSI_ClientManager.GetInstance();
		if (!clientComponent) 
			return;
		clientComponent.UpdateLocalCSISettingArray();
	};

	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void UpdateAuthoritySetting(string setting, string value)
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);
		
		GetGame().UserSettingsChanged();
		
		GetGame().GetCallqueue().CallLater(SaveAuthoritySettingsDelay, 1, false);
		
		UpdateAuthoritySettingArray();
	}
	
	//- Authority -\\
	//------------------------------------------------------------------------------------------------
	void SaveAuthoritySettingsDelay()
	{
		GetGame().SaveUserSettings();
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
			"groupNametagVisibleServerOverride",
			"nametagLOSEnabledServerOverride",
			
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
			"personalColorTeamMenuServerDefault",
			"groupNametagVisibleServerDefault",
			"nametagLOSEnabledServerDefault"
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
