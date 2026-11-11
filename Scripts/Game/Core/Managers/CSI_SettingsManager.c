class CSI_SettingsManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Effective CSI settings for the local player: their own CSI_GameSettings values with server overrides applied.
//! 				The server replicates one value per setting in m_aSettingsAuthorityValues: >= 0 means "use the client's value",
//! 				a negative value is a forced override (encoding below).
//! 
//------------------------------------------------------------------------------------------------
class CSI_SettingsManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	const static int SERVER_OVERRIDE_OFFSET = 1;	//!< non-boolean override is stored as -(value + SERVER_OVERRIDE_OFFSET)
	const static int SERVER_OVERRIDE_FALSE = -1;	//!< boolean override: forced off
	const static int SERVER_OVERRIDE_TRUE = -2;		//!< boolean override: forced on

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected bool m_bAuthorityIsSavingSettings;
	protected ref ScriptInvoker m_OnSettingsUpdate;

	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aSettingsAuthorityValues = {};	//!< server values by CSI_GameSettings.GetSettingsArray() index

	protected ref map<string, int> m_mSettingsLocalValues = new map<string, int>;	//!< effective values, read by GetSetting*

	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;

	protected UserSettings m_UserSettigs;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 MANAGER INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_UserSettigs = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");

		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();

		// Server (dedicated or listen host) publishes its settings as the override array
		if (RplSession.Mode() != RplMode.Client && GetGame().InPlayMode())
			UpdateAuthorityValueArray();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \return effective value; false if the setting isn't boolean
	bool GetSettingBool(string setting)
	{
		if (CSI_GameSettings.SETTINGS_THAT_ARE_BOOLEAN.Contains(setting))
			return GetSettingInt(setting);

		return false;
	};

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \return effective value (server override if set, else the client's own)
	int GetSettingInt(string setting)
	{
		return m_mSettingsLocalValues.Get(setting);
	}

	//------------------------------------------------------------------------------------------------
	//! \return replicated server values (encoded, see class description); empty until replicated
	TIntArray GetServerSettingsArray()
	{
		return m_aSettingsAuthorityValues;
	}

	//------------------------------------------------------------------------------------------------
	//! \return invoker fired after the effective settings are recalculated (created on first use)
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
	//! Recalculates the effective settings, e.g. after the local settings changed.
	void RequestSettingsUpdate()
	{
		SettingsUpdate();
	};

	//------------------------------------------------------------------------------------------------
	//! Server: stores a setting, encoded as an override if enabled, and replicates the updated array.
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value
	//! \param[in] serverOverrideEnabled false stores the plain value, so clients use their own
	void UpdateAuthoritySetting(string setting, int value, bool serverOverrideEnabled)
	{
		// Dedicated servers only (a listen server host can't change server settings)
		if (RplSession.Mode() != RplMode.Dedicated)
			return;

		int index = CSI_GameSettings.GetSettingsArray().Find(setting);
		if (index != -1)
		{
			if(serverOverrideEnabled)
			{
				// Encode as negative, see SERVER_OVERRIDE_* constants
				if (CSI_GameSettings.SETTINGS_THAT_ARE_BOOLEAN.Contains(setting))
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

			// Batch saves when an admin changes several settings at once
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
	//! Server: rebuilds the replicated array from the server's user settings.
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

		// The onRplName callback only runs on clients; apply directly when playing in Workbench
		#ifdef WORKBENCH
			SettingsUpdate();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Saves the server's user settings; delayed so several admin changes in a row are saved once.
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
	//! Recalculates the effective settings from the server array and the local values, then invokes GetOnSettingsUpdate().
	//! Also the onRplName callback of m_aSettingsAuthorityValues.
	protected void SettingsUpdate()
	{
		if (m_aSettingsAuthorityValues.IsEmpty())
			return;

		foreach (int i, string setting : CSI_GameSettings.GetSettingsArray())
		{
			int settingValue;
			bool IsBool = CSI_GameSettings.SETTINGS_THAT_ARE_BOOLEAN.Contains(setting);
			int serverSetting = m_aSettingsAuthorityValues.Get(i);

			// Decode the server value; >= 0 means no override, so use the client's own setting
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