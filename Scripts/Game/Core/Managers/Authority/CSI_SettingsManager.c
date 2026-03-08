class CSI_SettingsManagerClass : ScriptComponentClass {};

class CSI_SettingsManager : ScriptComponent
{	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	const static int SERVER_OVERRIDE_OFFSET = 1;
	const static int SERVER_OVERRIDE_FALSE = -1;
	const static int SERVER_OVERRIDE_TRUE = -2;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected bool m_bAuthorityIsSavingSettings;
	protected ref ScriptInvoker m_OnSettingsUpdate;
	protected UserSettings m_UserSettigs;
    protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	
	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aSettingsAuthorityValues = {};
	
	protected ref map<string, int> m_mSettingsLocalValues = new map<string, int>;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SYSTEM INITILIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		m_UserSettigs = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		
		if (RplSession.Mode() != RplMode.Client) 
			UpdateAuthorityValueArray();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	TIntArray GetServerSettingsArray() 
	{
		return m_aSettingsAuthorityValues;
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves an boolean value for the specified setting name.
	//! \param[in] setting: The name of the setting to retrieve
	//! \return True or false based on setting value
	bool GetSettingBool(string setting)
	{
		int index = CSI_GameSettings.GetSettingsArray().Find(setting);
		if (index != -1 && index <= CSI_GameSettings.INDEX_WHERE_BOOL_SETTINGS_STOP)
			return GetSettingInt(setting);
		
		return false;
	};	
	
	//------------------------------------------------------------------------------------------------
	//! Retrieves an integer value for the specified setting name.
	//! \param[in] setting: The name of the setting to retrieve
	//! \return The integer value of the setting
	int GetSettingInt(string setting) 
	{
		return m_mSettingsLocalValues.Get(setting);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnSettingsUpdate()
	{
		if (!m_OnSettingsUpdate)
			m_OnSettingsUpdate = new ScriptInvoker();

		return m_OnSettingsUpdate;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE SETTING METHODS
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	void RequestSettingsUpdate()
	{
		SettingsUpdate();
	};

	//------------------------------------------------------------------------------------------------
	//! Updates the specified server setting to the provided integer value and optionally enforces a server-side override.
	//! \param[in] setting: Setting key to update
	//! \param[in] value: New integer value for the setting
	//! \param[in] serverOverrideEnabled: If true, enforce server-side override
	void UpdateAuthoritySetting(string setting, int value, bool serverOverrideEnabled)
	{
		if (RplSession.Mode() != RplMode.Dedicated) 
			return;
		
		int index = CSI_GameSettings.GetSettingsArray().Find(setting);
		if (index != -1)
		{
			if(serverOverrideEnabled)
			{		
				if (index <= CSI_GameSettings.INDEX_WHERE_BOOL_SETTINGS_STOP)
				{
					if (value == 0)
						value = SERVER_OVERRIDE_FALSE;
					else
						value = SERVER_OVERRIDE_TRUE;
				} else {
					value = value + SERVER_OVERRIDE_OFFSET;
					value = -value;
				}
			};
			
			m_UserSettigs.Set(setting, value);
			
			if(!m_bAuthorityIsSavingSettings)
			{
				m_bAuthorityIsSavingSettings = true;
				GetGame().GetCallqueue().CallLater(SaveAuthoritySettingsDelay, 150, false);
			}

			UpdateAuthorityValueArray();
			return;
		};
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates the array of authority values used for settings.
	protected void UpdateAuthorityValueArray()
	{
		m_aSettingsAuthorityValues.Clear();
		
		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			int settingValue;
			m_UserSettigs.Get(setting, settingValue); 
			
			m_aSettingsAuthorityValues.Insert(settingValue);
		}
		
		Replication.BumpMe();
		
		#ifdef WORKBENCH
			SettingsUpdate();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Simple delay for saving authority settings so we dont jam up the queue if a server admin changed more than one setting at a time
	void SaveAuthoritySettingsDelay()
	{
		m_bAuthorityIsSavingSettings = false;
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 REPLICATION UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Updates CSI game settings all players pull from based on current server and/or client configuration
	protected void SettingsUpdate()
	{
		foreach (int i, string setting : CSI_GameSettings.GetSettingsArray())
		{
			int settingValue;
			bool IsBool = i <= CSI_GameSettings.INDEX_WHERE_BOOL_SETTINGS_STOP;
			int serverSetting = m_aSettingsAuthorityValues.Get(i);
	
			switch (true)
			{
				case (IsBool && serverSetting == SERVER_OVERRIDE_FALSE) : settingValue = 0; break;
				case (IsBool && serverSetting == SERVER_OVERRIDE_TRUE) : settingValue = 1; break;
				case (serverSetting < 0) : settingValue = Math.AbsInt((serverSetting + SERVER_OVERRIDE_OFFSET)); break;
				default : m_UserSettigs.Get(setting, settingValue);
			}
			
			m_mSettingsLocalValues.Set(setting, settingValue);
		}
		
		if (m_OnSettingsUpdate)
			m_OnSettingsUpdate.Invoke();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	protected static CSI_SettingsManager m_sInstance;
	static CSI_SettingsManager GetInstance()
	{
		return m_sInstance;
	}

    //------------------------------------------------------------------------------------------------
	void CSI_SettingsManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}