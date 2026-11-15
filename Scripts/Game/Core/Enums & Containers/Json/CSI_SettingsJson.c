//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Saves and loads the local player's CSI settings in $profile:CSI_Settings.json, by settings index.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingsJson
{
	protected static string m_sSettingsPath = "$profile:CSI_Settings.json";
    protected ref array<int> m_aSettingsValues = new array<int>;
	protected bool m_bHasSavedSettings;	//!< loaded from file or saved this session

	//------------------------------------------------------------------------------------------------
	//! Loads the saved values into the CSI_GameSettings user settings.
    void LoadFromFile()
	{
        if (FileIO.FileExists(m_sSettingsPath))
		{
            JsonLoadContext jsonLoad = new JsonLoadContext(false);
            if (!jsonLoad.LoadFromFile(m_sSettingsPath))
				return;

            jsonLoad.ReadValue("m_aSettingsValues", m_aSettingsValues);

			// An empty file counts as no saved settings
			m_bHasSavedSettings = !m_aSettingsValues.IsEmpty();

			// Settings newer than the file keep their defaults
			UserSettings userSettings = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
			foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
				if (m_aSettingsValues.IsIndexValid(i))
					userSettings.Set(settingStr, m_aSettingsValues.Get(i));
        };
    }

	//------------------------------------------------------------------------------------------------
    void SaveToFile()
	{
		m_aSettingsValues.Clear();
		UserSettings userSettings = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
		foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
		{
			int settingValue;
			userSettings.Get(settingStr, settingValue);
			m_aSettingsValues.Insert(settingValue);
		}

		JsonSaveContext jsonSave = new JsonSaveContext();
        jsonSave.WriteValue("m_aSettingsValues", m_aSettingsValues);
		if (!jsonSave.SaveToFile(m_sSettingsPath))
		{
			PrintFormat("CSI_SettingsJson: couldn't write %1, CSI settings weren't saved", m_sSettingsPath, level: LogLevel.WARNING);
			return;
		}

		m_bHasSavedSettings = true;
    }

	//------------------------------------------------------------------------------------------------
	bool HasSavedSettings()
	{
		return m_bHasSavedSettings;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if CSI_Settings.json exists
	static bool HasSettingsFile()
	{
		return FileIO.FileExists(m_sSettingsPath);
	}
}