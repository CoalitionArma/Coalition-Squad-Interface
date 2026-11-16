class CSI_SettingsManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Effective CSI settings for the local player: their own values with server overrides applied.
//! 				Also holds the admin-set server overrides, max group size and server default settings, saved
//! 				between server restarts.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingsManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	const static int SERVER_OVERRIDE_OFFSET = 1;	//!< non-boolean override stored as -(value + offset)
	const static int SERVER_OVERRIDE_FALSE = -1;	//!< boolean override: forced off
	const static int SERVER_OVERRIDE_TRUE = -2;		//!< boolean override: forced on
	const static int NO_SERVER_OVERRIDE = 0;		//!< setting has no server override (any value >= 0)

	const static int MAX_GROUP_SIZE_LIMIT = 99;		//!< highest max group size an admin can set
	const static int DEFAULT_MAX_GROUP_SIZE = 8;
	const static int NO_SERVER_DEFAULT = -1;		//!< setting has no server default
	const static int SERVER_DEFAULTS_SAVE_DELAY = 150;	//!< ms, so several changes are written once

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref ScriptInvokerVoid m_OnSettingsUpdate;

	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aSettingsAuthorityValues = {};	//!< encoded server values by settings index

	protected ref TIntArray m_aSettingsOverrideValues = {};	//!< server only: encoded overrides by settings index, saved; empty = none

	protected ref map<string, int> m_mSettingsLocalValues = new map<string, int>;	//!< effective values, read by GetSetting*

	// Slider limits of the settings that have them, read once (GetSettingLimits)
	protected ref map<string, int> m_mSettingMinValues;
	protected ref map<string, int> m_mSettingMaxValues;

	[RplProp()]
	protected int m_iMaxGroupSize = DEFAULT_MAX_GROUP_SIZE;	//!< top of the leaders' group size slider

	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aSettingsDefaultValues = {};	//!< server default by settings index; empty = no defaults

	protected ref CSI_ServerDefaultsJson m_ServerDefaultsJson;	//!< server only
	protected bool m_bServerDefaultsSavePending;

	protected UserSettings m_UserSettigs;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 MANAGER INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_UserSettigs = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");

		// Recalculate whenever the user settings change, e.g. in the settings menu
		GetGame().OnUserSettingsChangedInvoker().Insert(SettingsUpdate);

		// Server: load saved defaults and overrides and publish them before anything replicates
		if (GetGame().InPlayMode() && Replication.IsServer())
		{
			LoadServerDefaults();
			UpdateAuthorityValueArray();
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().OnUserSettingsChangedInvoker().Remove(SettingsUpdate);

		// Write a queued save now so it isn't lost
		if (m_bServerDefaultsSavePending)
		{
			if (GetGame())
				GetGame().GetCallqueue().Remove(SaveServerDefaults);

			SaveServerDefaults();
		}

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: loads the saved defaults, overrides and max group size.
	protected void LoadServerDefaults()
	{
		if (!m_ServerDefaultsJson)
			m_ServerDefaultsJson = new CSI_ServerDefaultsJson();

		TIntArray savedDefaults = {};
		int savedMaxGroupSize;
		bool hasSavedFile = m_ServerDefaultsJson.LoadFromProfile(savedDefaults, savedMaxGroupSize, m_aSettingsOverrideValues);

		// Older versions kept a dedicated server's overrides in its user settings
		if (m_aSettingsOverrideValues.IsEmpty() && RplSession.Mode() == RplMode.Dedicated)
			MigrateUserSettingsOverrides();

		if (!hasSavedFile)
			return;

		m_iMaxGroupSize = Math.ClampInt(savedMaxGroupSize, 1, MAX_GROUP_SIZE_LIMIT);

		// Negative values (only from an edited file) count as no default
		m_aSettingsDefaultValues.Clear();
		foreach (int savedDefault : savedDefaults)
		{
			int defaultValue = savedDefault;
			if (defaultValue < 0)
				defaultValue = NO_SERVER_DEFAULT;

			m_aSettingsDefaultValues.Insert(defaultValue);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Dedicated server: copies the overrides (negative values) from its user settings and saves them.
	protected void MigrateUserSettingsOverrides()
	{
		if (!m_UserSettigs)
			return;

		TIntArray overrideValues = {};
		bool hasOverrides;
		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			int value;
			m_UserSettigs.Get(setting, value);
			if (value < 0)
				hasOverrides = true;
			else
				value = NO_SERVER_OVERRIDE;

			overrideValues.Insert(value);
		}

		if (!hasOverrides)
			return;

		m_aSettingsOverrideValues = overrideValues;
		QueueServerDefaultsSave();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Read every frame by the HUD and nametags, so one map lookup.
	//! \param[in] setting an on/off CSI_GameSettings name (SETTINGS_THAT_ARE_BOOLEAN)
	//! \return effective value
	bool GetSettingBool(string setting)
	{
		return m_mSettingsLocalValues.Get(setting) != 0;
	};

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \return effective value (server overrides applied), within the setting's limits
	int GetSettingInt(string setting)
	{
		return m_mSettingsLocalValues.Get(setting);
	}

	//------------------------------------------------------------------------------------------------
	//! Slider range of a setting, read once from its attribute.
	//! \param[in] setting CSI_GameSettings name
	//! \param[out] min
	//! \param[out] max
	//! \return false if the setting has no min and max
	bool GetSettingLimits(string setting, out int min, out int max)
	{
		if (!m_mSettingMinValues)
			CacheSettingLimits();

		return m_mSettingMinValues.Find(setting, min) && m_mSettingMaxValues.Find(setting, max);
	}

	//------------------------------------------------------------------------------------------------
	protected void CacheSettingLimits()
	{
		m_mSettingMinValues = new map<string, int>();
		m_mSettingMaxValues = new map<string, int>();

		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			int min, max;
			if (!CSI_GameSettings.GetSettingLimits(setting, min, max))
				continue;

			m_mSettingMinValues.Set(setting, min);
			m_mSettingMaxValues.Set(setting, max);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return encoded server values; empty until replicated
	TIntArray GetServerSettingsArray()
	{
		return m_aSettingsAuthorityValues;
	}

	//------------------------------------------------------------------------------------------------
	//! \return highest group size leaders can set
	int GetMaxGroupSize()
	{
		return m_iMaxGroupSize;
	}

	//------------------------------------------------------------------------------------------------
	bool HasServerDefaults()
	{
		return !m_aSettingsDefaultValues.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[out] value the server's default, if it has one
	//! \return true if the server has a default for this setting
	bool GetServerDefault(string setting, out int value)
	{
		int index = CSI_GameSettings.GetSettingsArray().Find(setting);
		if (!m_aSettingsDefaultValues.IsIndexValid(index) || m_aSettingsDefaultValues[index] == NO_SERVER_DEFAULT)
			return false;

		value = m_aSettingsDefaultValues[index];
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return also true without a local player
	protected bool HasLocalSavedSettings()
	{
		CSI_PlayerControllerManager controllerManager = CSI_PlayerControllerManager.GetInstance();
		if (!controllerManager || !controllerManager.GetLocalSettingsJson())
			return true;

		return controllerManager.GetLocalSettingsJson().HasSavedSettings();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnSettingsUpdate()
	{
		if (!m_OnSettingsUpdate)
			m_OnSettingsUpdate = new ScriptInvokerVoid();

		return m_OnSettingsUpdate;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE SETTING METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Recalculates the effective settings.
	void RequestSettingsUpdate()
	{
		SettingsUpdate();
	};

	//------------------------------------------------------------------------------------------------
	//! Server: stores a setting's override, encoded, then saves and replicates it.
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value
	//! \param[in] serverOverrideEnabled false lets clients use their own value
	void UpdateAuthoritySetting(string setting, int value, bool serverOverrideEnabled)
	{
		if (!Replication.IsServer())
			return;

		int index = CSI_GameSettings.GetSettingsArray().Find(setting);
		if (index != -1)
		{
			// Personal settings are never stored as overrides
			if (CSI_GameSettings.SETTINGS_WITHOUT_SERVER_OVERRIDE.Contains(setting))
				serverOverrideEnabled = false;

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
			}
			else
			{
				value = NO_SERVER_OVERRIDE;
			}

			// First override: one entry per setting, none set yet
			while (m_aSettingsOverrideValues.Count() < CSI_GameSettings.GetSettingsArray().Count())
				m_aSettingsOverrideValues.Insert(NO_SERVER_OVERRIDE);

			m_aSettingsOverrideValues.Set(index, value);
			QueueServerDefaultsSave();

			UpdateAuthorityValueArray();
			return;
		};
	}

	//------------------------------------------------------------------------------------------------
	//! Server: rebuilds the replicated array from the stored overrides.
	protected void UpdateAuthorityValueArray()
	{
		m_aSettingsAuthorityValues.Clear();

		int settingsCount = CSI_GameSettings.GetSettingsArray().Count();
		for (int i = 0; i < settingsCount; i++)
		{
			int settingValue = NO_SERVER_OVERRIDE;
			if (m_aSettingsOverrideValues.IsIndexValid(i))
				settingValue = m_aSettingsOverrideValues[i];

			m_aSettingsAuthorityValues.Insert(settingValue);
		}

		Replication.BumpMe();

		// onRplName only runs on clients, so update here too
		SettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//! Server: sets the top of the leaders' group size slider.
	//! \param[in] maxGroupSize clamped to 1 - MAX_GROUP_SIZE_LIMIT
	void UpdateMaxGroupSize(int maxGroupSize)
	{
		if (!Replication.IsServer())
			return;

		m_iMaxGroupSize = Math.ClampInt(maxGroupSize, 1, MAX_GROUP_SIZE_LIMIT);
		Replication.BumpMe();

		QueueServerDefaultsSave();
	}
	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 SERVER DEFAULTS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Server: stores one setting's default for players without saved settings.
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value
	void UpdateServerDefault(string setting, int value)
	{
		if (!Replication.IsServer())
			return;

		int index = CSI_GameSettings.GetSettingsArray().Find(setting);
		if (index == -1 || value < 0)
			return;

		// First default: one entry per setting, none set yet
		while (m_aSettingsDefaultValues.Count() < CSI_GameSettings.GetSettingsArray().Count())
			m_aSettingsDefaultValues.Insert(NO_SERVER_DEFAULT);

		m_aSettingsDefaultValues.Set(index, value);
		Replication.BumpMe();

		// onRplName only runs on clients, so update here too
		SettingsUpdate();

		QueueServerDefaultsSave();
	}

	//------------------------------------------------------------------------------------------------
	void ClearServerDefaults()
	{
		if (!Replication.IsServer())
			return;

		m_aSettingsDefaultValues.Clear();
		Replication.BumpMe();

		SettingsUpdate();

		QueueServerDefaultsSave();
	}

	//------------------------------------------------------------------------------------------------
	protected void QueueServerDefaultsSave()
	{
		if (m_bServerDefaultsSavePending)
			return;

		m_bServerDefaultsSavePending = true;
		GetGame().GetCallqueue().CallLater(SaveServerDefaults, SERVER_DEFAULTS_SAVE_DELAY, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Writes the defaults, overrides and max group size to CSI_ServerDefaults.json.
	protected void SaveServerDefaults()
	{
		m_bServerDefaultsSavePending = false;

		if (!m_ServerDefaultsJson)
			m_ServerDefaultsJson = new CSI_ServerDefaultsJson();

		m_ServerDefaultsJson.SaveToProfile(m_aSettingsDefaultValues, m_iMaxGroupSize, m_aSettingsOverrideValues);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: copies the server's defaults into the local user settings for the settings tab.
	//! Only call for players without saved settings.
	void ApplyServerDefaultsToUserSettings()
	{
		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			int defaultValue;
			if (GetServerDefault(setting, defaultValue))
				m_UserSettigs.Set(setting, defaultValue);
		}
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 REPLICATION UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Recalculates the effective settings and invokes GetOnSettingsUpdate().
	//! Also the onRplName callback of both replicated setting arrays.
	protected void SettingsUpdate()
	{
		if (m_aSettingsAuthorityValues.IsEmpty())
			return;

		// Players without saved settings get the server's defaults (in memory only)
		bool useServerDefaults = HasServerDefaults() && !HasLocalSavedSettings();

		foreach (int i, string setting : CSI_GameSettings.GetSettingsArray())
		{
			int settingValue;
			bool IsBool = CSI_GameSettings.SETTINGS_THAT_ARE_BOOLEAN.Contains(setting);
			// No override (0) for personal settings and ones an older server doesn't know
			int serverSetting = 0;
			if (m_aSettingsAuthorityValues.IsIndexValid(i) && !CSI_GameSettings.SETTINGS_WITHOUT_SERVER_OVERRIDE.Contains(setting))
				serverSetting = m_aSettingsAuthorityValues[i];

			// Decode; >= 0 means no override: use the server default or the client's value
			switch (true)
			{
				case (IsBool && serverSetting == SERVER_OVERRIDE_FALSE) : settingValue = 0; break;
				case (IsBool && serverSetting == SERVER_OVERRIDE_TRUE) : settingValue = 1; break;
				case (serverSetting < 0) : settingValue = Math.AbsInt((serverSetting + SERVER_OVERRIDE_OFFSET)); break;
				default :
				{
					if (!useServerDefaults || !GetServerDefault(setting, settingValue))
						m_UserSettigs.Get(setting, settingValue);
				}
			}

			// Within the setting's slider range, wherever the value came from
			int min, max;
			if (GetSettingLimits(setting, min, max))
				settingValue = Math.ClampInt(settingValue, min, max);

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