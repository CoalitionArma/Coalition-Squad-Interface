class CSI_SettingsManagerClass : ScriptComponentClass {};

class CSI_SettingsManager : ScriptComponent
{	
	const static int SERVER_OVERRIDE_OFFSET = 1;
	const static int SERVER_OVERRIDE_FALSE = -1;
	const static int SERVER_OVERRIDE_TRUE = -2;

	protected ref ScriptInvoker m_OnSettingsUpdate;
	protected UserSettings m_UserSettigs;
    protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	
	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aSettingsAuthorityValues = {};
	
	protected ref map<string, int> m_mSettingsLocalValues = new map<string, int>;

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
	
	//------------------------------------------------------------------------------------------------
	/*!
	 * Retrieves an boolean value for the specified setting name.
	 * @param setting: The name of the setting to retrieve
	 * @return True or false based on setting value
	 */
	bool GetSettingBool(string setting)
	{
		int index = CSI_GameSettings.m_aSettingsArray.Find(setting);
		if (index != -1 && index <= INDEX_WHERE_BOOL_SETTINGS_STOP)
			return GetSettingInt(setting);
		
		return false;
	};	
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Retrieves an integer value for the specified setting name.
	 * @param setting: The name of the setting to retrieve
	 * @return The integer value of the setting
	 */
	int GetSettingInt(string setting) 
	{
		return m_mSettingsLocalValues.Get(setting);
	}
	
	//------------------------------------------------------------------------------------------------
	static TStringArray GetSettingsArray() 
	{
		return CSI_GameSettings.m_aSettingsArray;
	}
	
	//------------------------------------------------------------------------------------------------
	TIntArray GetServerSettingsArray() 
	{
		return m_aSettingsAuthorityValues;
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestSettingsUpdate()
	{
		SettingsUpdate();
	};
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates the specified server setting to the provided integer value and optionally enforces a server-side override.
	 * @param setting: Setting key to update
	 * @param value: New integer value for the setting
	 * @param serverOverrideEnabled: If true, enforce server-side override
	 */
	void UpdateServerSetting(string setting, int value, bool serverOverrideEnabled)
	{
		if (RplSession.Mode() != RplMode.Dedicated) 
			return;
		
		int index = CSI_GameSettings.m_aSettingsArray.Find(setting);
		if (index != -1)
		{
			if(serverOverrideEnabled)
			{		
				if (index <= INDEX_WHERE_BOOL_SETTINGS_STOP)
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
			GetGame().UserSettingsChanged();
			UpdateAuthorityValueArray();
			return;
		};
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates the array of authority values used for settings.
	 */
	protected void UpdateAuthorityValueArray()
	{
		m_aSettingsAuthorityValues.Clear();
		
		foreach (string setting : CSI_GameSettings.m_aSettingsArray)
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
	/**
	 * Updates CSI game settings all players pull from based on current server and/or client configuration
	 */
	protected void SettingsUpdate()
	{
		foreach (int i, string setting : CSI_GameSettings.m_aSettingsArray)
		{
			int settingValue;
			bool IsBool = i <= INDEX_WHERE_BOOL_SETTINGS_STOP;
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

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnSettingsUpdate()
	{
		if (!m_OnSettingsUpdate)
			m_OnSettingsUpdate = new ScriptInvoker();

		return m_OnSettingsUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the SettingsManager
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