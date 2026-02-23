class CSI_SettingsJson : JsonApiStruct 
{
	protected static string m_sSettingsPath = "$profile:CSI_Settings.json";
    protected ref array<int> m_aSettingsValues = new array<int>;

	//------------------------------------------------------------------------------------------------
    void LoadFromFile()
	{
        if (FileIO.FileExists(m_sSettingsPath)) 
		{
            SCR_JsonLoadContext jsonLoad = new SCR_JsonLoadContext(false);
            jsonLoad.LoadFromFile(m_sSettingsPath);
            jsonLoad.ReadValue("m_aSettingsValues", m_aSettingsValues);
		
			UserSettings userSettings = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
			foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
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
		
		SCR_JsonSaveContext jsonSave = new SCR_JsonSaveContext();
        jsonSave.WriteValue("m_aSettingsValues", m_aSettingsValues);
        jsonSave.SaveToFile(m_sSettingsPath);
    }

	//------------------------------------------------------------------------------------------------
    void CSI_SettingsJson() 
	{
		RegV("m_aSettingsValues");
    }
}