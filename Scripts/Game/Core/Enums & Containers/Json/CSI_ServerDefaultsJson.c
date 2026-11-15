//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Saves and loads the server's default settings, server overrides and max group size in
//! 				$profile:CSI_ServerDefaults.json, so they last between server restarts.
//!
//------------------------------------------------------------------------------------------------
class CSI_ServerDefaultsJson
{
	protected static string m_sServerDefaultsPath = "$profile:CSI_ServerDefaults.json";
	protected ref array<int> m_aSettingsDefaultValues = new array<int>;
	protected int m_iMaxGroupSize;
	protected ref array<int> m_aSettingsOverrideValues = new array<int>;	//!< encoded as in CSI_SettingsManager

	//------------------------------------------------------------------------------------------------
	//! \param[out] defaultValues by settings index; empty if none
	//! \param[out] maxGroupSize DEFAULT_MAX_GROUP_SIZE if the file has none
	//! \param[out] overrideValues encoded server overrides by settings index; empty if none
	//! \return false if there's no valid file yet
	bool LoadFromProfile(notnull array<int> defaultValues, out int maxGroupSize, notnull array<int> overrideValues)
	{
		if (!FileIO.FileExists(m_sServerDefaultsPath))
			return false;

		JsonLoadContext jsonLoad = new JsonLoadContext(false);
		if (!jsonLoad.LoadFromFile(m_sServerDefaultsPath))
			return false;

		// Missing values (an edited or older file) keep these
		m_aSettingsDefaultValues.Clear();
		m_iMaxGroupSize = CSI_SettingsManager.DEFAULT_MAX_GROUP_SIZE;
		m_aSettingsOverrideValues.Clear();

		jsonLoad.ReadValue("m_aSettingsDefaultValues", m_aSettingsDefaultValues);
		jsonLoad.ReadValue("m_iMaxGroupSize", m_iMaxGroupSize);
		jsonLoad.ReadValue("m_aSettingsOverrideValues", m_aSettingsOverrideValues);

		defaultValues.Clear();
		defaultValues.Copy(m_aSettingsDefaultValues);
		maxGroupSize = m_iMaxGroupSize;
		overrideValues.Clear();
		overrideValues.Copy(m_aSettingsOverrideValues);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] defaultValues by settings index; empty for no defaults
	//! \param[in] maxGroupSize
	//! \param[in] overrideValues encoded server overrides by settings index; empty for none
	void SaveToProfile(notnull array<int> defaultValues, int maxGroupSize, notnull array<int> overrideValues)
	{
		m_aSettingsDefaultValues.Clear();
		m_aSettingsDefaultValues.Copy(defaultValues);
		m_iMaxGroupSize = maxGroupSize;
		m_aSettingsOverrideValues.Clear();
		m_aSettingsOverrideValues.Copy(overrideValues);

		JsonSaveContext jsonSave = new JsonSaveContext();
		jsonSave.WriteValue("m_aSettingsDefaultValues", m_aSettingsDefaultValues);
		jsonSave.WriteValue("m_iMaxGroupSize", m_iMaxGroupSize);
		jsonSave.WriteValue("m_aSettingsOverrideValues", m_aSettingsOverrideValues);
		if (!jsonSave.SaveToFile(m_sServerDefaultsPath))
			PrintFormat("CSI_ServerDefaultsJson: couldn't write %1, the server's CSI settings won't last a restart", m_sServerDefaultsPath, level: LogLevel.WARNING);
	}
}
