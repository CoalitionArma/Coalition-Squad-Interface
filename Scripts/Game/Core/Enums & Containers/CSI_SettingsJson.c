//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Saves and loads the local player's CSI settings as an int array in $profile:CSI_Settings.json,
//! 				indexed like CSI_GameSettings.GetSettingsArray().
//! 
//------------------------------------------------------------------------------------------------
class CSI_SettingsJson : JsonApiStruct
{
	protected static string m_sSettingsPath = "$profile:CSI_Settings.json";
    protected ref array<int> m_aSettingsValues = new array<int>;

	//------------------------------------------------------------------------------------------------
	//! Loads the saved values into the CSI_GameSettings user settings module. Does nothing if there's no file yet.
    void LoadFromFile()
	{
        if (FileIO.FileExists(m_sSettingsPath))
		{
            JsonLoadContext jsonLoad = new JsonLoadContext(false);
            jsonLoad.LoadFromFile(m_sSettingsPath);
            jsonLoad.ReadValue("m_aSettingsValues", m_aSettingsValues);

			// Settings added after the file was saved have no entry and keep their defaults
			UserSettings userSettings = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
			foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
				if (m_aSettingsValues.IsIndexValid(i))
					userSettings.Set(settingStr, m_aSettingsValues.Get(i));
        };
    }

	//------------------------------------------------------------------------------------------------
	//! Writes the current CSI_GameSettings values to the profile file.
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
        jsonSave.SaveToFile(m_sSettingsPath);
    }

	//------------------------------------------------------------------------------------------------
    void CSI_SettingsJson()
	{
		RegV("m_aSettingsValues");
    }
}