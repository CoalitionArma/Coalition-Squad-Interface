//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Saves and loads the server's default settings and max group size in $profile:CSI_ServerDefaults.json,
//! 				so they last between server restarts.
//!
//------------------------------------------------------------------------------------------------
class CSI_ServerDefaultsJson : JsonApiStruct
{
	protected static string m_sServerDefaultsPath = "$profile:CSI_ServerDefaults.json";
	protected ref array<int> m_aSettingsDefaultValues = new array<int>;
	protected int m_iMaxGroupSize;

	//------------------------------------------------------------------------------------------------
	//! \param[out] defaultValues by settings index; empty if none
	//! \param[out] maxGroupSize DEFAULT_MAX_GROUP_SIZE if the file has none
	//! \return false if there's no valid file yet
	bool LoadFromProfile(notnull array<int> defaultValues, out int maxGroupSize)
	{
		if (!FileIO.FileExists(m_sServerDefaultsPath))
			return false;

		JsonLoadContext jsonLoad = new JsonLoadContext(false);
		if (!jsonLoad.LoadFromFile(m_sServerDefaultsPath))
			return false;

		// Missing values (an edited or older file) keep these
		m_aSettingsDefaultValues.Clear();
		m_iMaxGroupSize = CSI_SettingsManager.DEFAULT_MAX_GROUP_SIZE;

		jsonLoad.ReadValue("m_aSettingsDefaultValues", m_aSettingsDefaultValues);
		jsonLoad.ReadValue("m_iMaxGroupSize", m_iMaxGroupSize);

		defaultValues.Clear();
		defaultValues.Copy(m_aSettingsDefaultValues);
		maxGroupSize = m_iMaxGroupSize;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] defaultValues by settings index; empty for no defaults
	//! \param[in] maxGroupSize
	void SaveToProfile(notnull array<int> defaultValues, int maxGroupSize)
	{
		m_aSettingsDefaultValues.Clear();
		m_aSettingsDefaultValues.Copy(defaultValues);
		m_iMaxGroupSize = maxGroupSize;

		JsonSaveContext jsonSave = new JsonSaveContext();
		jsonSave.WriteValue("m_aSettingsDefaultValues", m_aSettingsDefaultValues);
		jsonSave.WriteValue("m_iMaxGroupSize", m_iMaxGroupSize);
		jsonSave.SaveToFile(m_sServerDefaultsPath);
	}

	//------------------------------------------------------------------------------------------------
	void CSI_ServerDefaultsJson()
	{
		RegV("m_aSettingsDefaultValues");
		RegV("m_iMaxGroupSize");
	}
}
