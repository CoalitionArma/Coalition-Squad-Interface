class CSI_SettingsManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Effective CSI settings for the local player: their own CSI_GameSettings values with server overrides applied.
//! 				The server replicates one value per setting in m_aSettingsAuthorityValues: >= 0 means "use the client's value",
//! 				a negative value is a forced override (encoding below). Also holds the admin-set max group size and the
//! 				server's default settings, which apply to players without their own saved settings (see CSI_SettingsJson).
//! 				Both are set in game from the CSI settings tab; the server saves them to $profile:CSI_ServerDefaults.json
//! 				(CSI_ServerDefaultsJson) and loads them again when it starts.
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

	const static int MAX_GROUP_SIZE_LIMIT = 99;		//!< highest max group size an admin can set
	const static int DEFAULT_MAX_GROUP_SIZE = 8;	//!< max group size until an admin changes it
	const static int NO_SERVER_DEFAULT = -1;		//!< m_aSettingsDefaultValues entry for a setting without a server default
	const static int SERVER_DEFAULTS_SAVE_DELAY = 150;	//!< ms; "Save My Settings As Server Defaults" sends one request per setting, saved once

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected bool m_bAuthorityIsSavingSettings;
	protected ref ScriptInvoker m_OnSettingsUpdate;

	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aSettingsAuthorityValues = {};	//!< server values by CSI_GameSettings.GetSettingsArray() index

	protected ref map<string, int> m_mSettingsLocalValues = new map<string, int>;	//!< effective values, read by GetSetting*

	[RplProp()]
	protected int m_iMaxGroupSize = DEFAULT_MAX_GROUP_SIZE;	//!< top of the leaders' group size slider

	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aSettingsDefaultValues = {};	//!< server default by settings index (NO_SERVER_DEFAULT = none); empty = no defaults

	protected ref CSI_ServerDefaultsJson m_ServerDefaultsJson;	//!< server: file the defaults and max group size are saved to
	protected bool m_bServerDefaultsSavePending;				//!< server: a save of that file is queued

	protected UserSettings m_UserSettigs;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 MANAGER INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_UserSettigs = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");

		// Server (dedicated or listen host) restores the defaults and max group size saved in an earlier session, then
		// publishes its settings as the override array; both before anything replicates to clients
		if (RplSession.Mode() != RplMode.Client && GetGame().InPlayMode())
		{
			LoadServerDefaults();
			UpdateAuthorityValueArray();
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		// The server is shutting down with a save of the defaults still queued: write it now so it isn't lost
		if (m_bServerDefaultsSavePending)
		{
			if (GetGame())
				GetGame().GetCallqueue().Remove(SaveServerDefaults);

			SaveServerDefaults();
		}

		// Same for a queued save of the server's override settings
		if (m_bAuthorityIsSavingSettings && GetGame())
		{
			GetGame().GetCallqueue().Remove(SaveAuthoritySettingsDelay);
			SaveAuthoritySettingsDelay();
		}

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: restores the default settings and max group size from CSI_ServerDefaultsJson, if it has been saved before.
	protected void LoadServerDefaults()
	{
		if (!m_ServerDefaultsJson)
			m_ServerDefaultsJson = new CSI_ServerDefaultsJson();

		TIntArray savedDefaults = {};
		int savedMaxGroupSize;
		if (!m_ServerDefaultsJson.LoadFromProfile(savedDefaults, savedMaxGroupSize))
			return;

		m_iMaxGroupSize = Math.ClampInt(savedMaxGroupSize, 1, MAX_GROUP_SIZE_LIMIT);

		// Same rule as UpdateServerDefault: negative values (only from an edited file) count as no default
		m_aSettingsDefaultValues.Clear();
		foreach (int savedDefault : savedDefaults)
		{
			int defaultValue = savedDefault;
			if (defaultValue < 0)
				defaultValue = NO_SERVER_DEFAULT;

			m_aSettingsDefaultValues.Insert(defaultValue);
		}
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
	//! \return highest size leaders can set for their group (1 - MAX_GROUP_SIZE_LIMIT)
	int GetMaxGroupSize()
	{
		return m_iMaxGroupSize;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the server has default settings for players without saved settings
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
	//! \return true if the local player has their own saved settings (always true without a local player)
	protected bool HasLocalSavedSettings()
	{
		CSI_PlayerControllerManager controllerManager = CSI_PlayerControllerManager.GetInstance();
		if (!controllerManager || !controllerManager.GetLocalSettingsJson())
			return true;

		return controllerManager.GetLocalSettingsJson().HasSavedSettings();
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

		// onRplName only runs on clients, so call it on the server too, as vanilla does for its RplProp callbacks
		SettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//! Saves the server's user settings; delayed so several admin changes in a row are saved once.
	void SaveAuthoritySettingsDelay()
	{
		m_bAuthorityIsSavingSettings = false;
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! Server: sets the max group size (the top of the leaders' group size slider).
	//! \param[in] maxGroupSize kept within 1 - MAX_GROUP_SIZE_LIMIT
	void UpdateMaxGroupSize(int maxGroupSize)
	{
		if (!Replication.IsServer())
			return;

		m_iMaxGroupSize = Math.ClampInt(maxGroupSize, 1, MAX_GROUP_SIZE_LIMIT);
		Replication.BumpMe();

		QueueServerDefaultsSave();
	}

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

		// onRplName only runs on clients; update a listen host's own effective settings directly
		SettingsUpdate();

		QueueServerDefaultsSave();
	}

	//------------------------------------------------------------------------------------------------
	//! Server: removes all default settings.
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
	//! Server: saves the defaults and max group size shortly, so several changes in a row are written once.
	protected void QueueServerDefaultsSave()
	{
		if (m_bServerDefaultsSavePending)
			return;

		m_bServerDefaultsSavePending = true;
		GetGame().GetCallqueue().CallLater(SaveServerDefaults, SERVER_DEFAULTS_SAVE_DELAY, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: writes the defaults and max group size to CSI_ServerDefaultsJson for the next session.
	protected void SaveServerDefaults()
	{
		m_bServerDefaultsSavePending = false;

		if (!m_ServerDefaultsJson)
			m_ServerDefaultsJson = new CSI_ServerDefaultsJson();

		m_ServerDefaultsJson.SaveToProfile(m_aSettingsDefaultValues, m_iMaxGroupSize);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: copies the server's defaults into the local user settings, so the settings tab shows what the player
	//! currently gets. Only call for players without saved settings; closing the tab then saves them as their own.
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
	//! Recalculates the effective settings from the server array and the local values, then invokes GetOnSettingsUpdate().
	//! Also the onRplName callback of m_aSettingsAuthorityValues and m_aSettingsDefaultValues.
	protected void SettingsUpdate()
	{
		if (m_aSettingsAuthorityValues.IsEmpty())
			return;

		// Players without their own saved settings get the server's defaults (in memory only, never saved for them)
		bool useServerDefaults = HasServerDefaults() && !HasLocalSavedSettings();

		foreach (int i, string setting : CSI_GameSettings.GetSettingsArray())
		{
			int settingValue;
			bool IsBool = CSI_GameSettings.SETTINGS_THAT_ARE_BOOLEAN.Contains(setting);
			int serverSetting = m_aSettingsAuthorityValues.Get(i);

			// Decode the server value; >= 0 means no override, so use the server default or the client's own setting
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