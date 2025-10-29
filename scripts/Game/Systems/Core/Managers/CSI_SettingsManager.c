[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_SettingsManagerClass : ScriptComponentClass {};

class CSI_SettingsManager : ScriptComponent
{	
    protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected static CSI_SettingsManager m_sInstance;
	
    ref TStringArray m_aLocalCSISettingsArray = {};

	//------------------------------------------------------------------------------------------------
	// Returns the instance of the SettingsManager
	static CSI_SettingsManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		if(!Replication.IsServer())
			return;
		
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	TStringArray ReturnLocalCSISettings() 
	{
		return m_aLocalCSISettingsArray;
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeLocalCSISetting(string setting, string value)
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set(setting, value);
		
		UpdateLocalCSISettingArray();
	}
		
	//------------------------------------------------------------------------------------------------
	void UpdateLocalCSISettingArray()
	{
		array<string> settingsToCheck = {
			// Settings that can be overriden by the server
			"compassVisible",            //0
			"squadRadarVisible",         //1
			"groupDisplayVisible",       //2
			"staminaBarVisible",         //3
			"nametagsVisible",           //4
			"rankVisible",               //5
			"nametagsRange",             //6
			"roleNametagVisible",        //7
			"personalColorTeamMenu",     //8
			"groupNametagVisible",       //9
			"nametagLOSEnabled",         //10

			// Settings that are purely local to each client
			"squadRadarIconSize",        //11
			"squadRadarSelfIconVisible", //12
			"nametagsPosition",          //13
			"autoHideUI",                //14
			"compassTexture",            //15
		};

		array<string> tempLocalCSISettingsArray = {};

		foreach (int i, string checkSetting : settingsToCheck)
		{
			string settingValue = "";
			string settingServerOverride = "";
			if (i < 11 && !m_AuthorityManager.ReturnAuthoritySettings().IsEmpty()) 
			{
				settingServerOverride = m_AuthorityManager.ReturnAuthoritySettings()[i];
			};
			switch (true)
			{
				case(!(settingServerOverride.IsEmpty() || settingServerOverride == "N/A")) : {settingValue = settingServerOverride; break;};
				default : {
					GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get(checkSetting, settingValue); 
					if (i < 11 && settingValue.IsEmpty() && (!m_AuthorityManager.ReturnAuthoritySettings().IsEmpty() && m_AuthorityManager.ReturnAuthoritySettings()[11] == "true")) 
					{
						 settingValue = m_AuthorityManager.ReturnAuthoritySettings()[i+12]; 
					}; 
					break; 
				};
			};
			tempLocalCSISettingsArray.Insert(settingValue);
		};
		m_aLocalCSISettingsArray = tempLocalCSISettingsArray;	
	}

    //------------------------------------------------------------------------------------------------
	void CSI_SettingsManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}