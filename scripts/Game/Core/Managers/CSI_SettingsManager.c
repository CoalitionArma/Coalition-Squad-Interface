class CSI_SettingsManagerClass : ScriptComponentClass {};

class CSI_SettingsManager : ScriptComponent
{	
	const static string COMPASS_VISIBLE = "m_iCompassVisible";
	const static string BEARING_VISIBLE = "m_iBearingVisible";
	const static string RADAR_VISIBLE = "m_iRadarVisible";
	const static string ONLY_RADAR_ICON_ARROWS_ROTATE = "m_iOnlyRadarIconArrowsRotate";
	const static string GROUP_VISIBLE = "m_iGroupVisible";
	const static string STAMINA_VISIBLE = "m_iStaminaVisible";
	const static string NAMETAG_VISIBLE = "m_iNametagVisible";
	const static string RANK_VISIBLE = "m_iRankVisible";
	const static string ROLE_IN_NAMETAG_VISIBLE = "m_iRoleInNametagVisible";
	const static string GROUP_IN_NAMETAG_VISIBLE = "m_iGroupInNametagVisible";
	const static string NAMETAG_LOS_VISIBLE = "m_iNametagLOSVisible";
	const static string AUTO_HIDE_HUD = "m_iAutoHideHUD";
	const static string ICON_THEME = "m_iIconTheme";
	const static string ICON_TYPE = "m_iIconType";
	const static string ARROW_THEME = "m_iArrowTheme";
	const static string COMPASS_THEME = "m_iCompassTheme";
	const static string NAMETAG_POSITION = "m_iNametagPosition";
	const static string NAMETAG_ROLE_ICON_POSITION = "m_iNametagRoleIconPosition";
	const static string NAMETAG_POSITION_OFFSET = "m_iNametagPositionOffset";
	const static string NAMETAG_RANGE = "m_iNametagRange";
	const static string NAMETAG_MAGNIFICATION_MULTIPLICATION = "m_iNametagMagnificationMultiplication";
	const static string RADAR_ICON_SIZE = "m_iRadarIconSize";
	
	const static int INDEX_WHERE_BOOL_SETTINGS_STOP = 11;
	const static int SERVER_OVERRIDE_DISABLED_VALUE = 0;
	const static int SERVER_OVERRIDE_FALSE = -1;
	const static int SERVER_OVERRIDE_TRUE = -2;
	
	protected ref ScriptInvoker m_OnSettingsUpdate;
	protected UserSettings m_UserSettigs;
    protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	
    protected ref TStringArray m_aCSISettingsArray = {};
	
	[RplProp(onRplName: "SettingsUpdate")]
	protected ref TIntArray m_aCSISettingsAuthorityValues = {};
	
	protected ref map<string, int> m_mCSISettingsLocalValues = new map<string, int>;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		m_UserSettigs = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		
		m_aCSISettingsArray = {
			COMPASS_VISIBLE,
			BEARING_VISIBLE,
			RADAR_VISIBLE,
			ONLY_RADAR_ICON_ARROWS_ROTATE,
			GROUP_VISIBLE,
			STAMINA_VISIBLE,
			NAMETAG_VISIBLE,
			RANK_VISIBLE,
			ROLE_IN_NAMETAG_VISIBLE,
			GROUP_IN_NAMETAG_VISIBLE,
			NAMETAG_LOS_VISIBLE,
			AUTO_HIDE_HUD,
			ICON_THEME,
			ICON_TYPE,
			ARROW_THEME,
			COMPASS_THEME,
			NAMETAG_POSITION,
			NAMETAG_ROLE_ICON_POSITION,
			NAMETAG_POSITION_OFFSET,
			NAMETAG_RANGE,
			NAMETAG_MAGNIFICATION_MULTIPLICATION,
			RADAR_ICON_SIZE
		};
		
		if (RplSession.Mode() != RplMode.Client) 
			UpdateAuthorityValueArray();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetCSISettingBool(string setting) 
	{
		int index = m_aCSISettingsArray.Find(setting);
		if (index != -1 && index <= INDEX_WHERE_BOOL_SETTINGS_STOP)
			return GetCSISettingInt(setting);
		
		return false;
	};	
	
	//------------------------------------------------------------------------------------------------
	int GetCSISettingInt(string setting) 
	{
		return m_mCSISettingsLocalValues.Get(setting);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateLocalSetting(string setting, int value)
	{
		if (RplSession.Mode() != RplMode.Client) 
		{
			int index = m_aCSISettingsArray.Find(setting);
			if (index != -1)
			{
				if (value != SERVER_OVERRIDE_DISABLED_VALUE)
				{
					if (index <= INDEX_WHERE_BOOL_SETTINGS_STOP)
					{
						if (value == false)
							value = SERVER_OVERRIDE_FALSE;
						else
							value = SERVER_OVERRIDE_TRUE;
					} else
						value = -value;
				};
				m_UserSettigs.Set(setting, value);
				UpdateAuthorityValueArray();
				return;
			};
		};
		
		m_UserSettigs.Set(setting, value);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAuthorityValueArray()
	{
		m_aCSISettingsAuthorityValues.Clear();
		
		foreach (string setting : m_aCSISettingsArray)
		{
			int settingValue;
			m_UserSettigs.Get(setting, settingValue); 
			
			m_aCSISettingsAuthorityValues.Insert(settingValue);
		}
		
		Replication.BumpMe();
		
		#ifdef WORKBENCH
			SettingsUpdate();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void SettingsUpdate()
	{
		foreach (int i, string setting : m_aCSISettingsArray)
		{
			int settingValue;
			bool IsBool = i <= INDEX_WHERE_BOOL_SETTINGS_STOP;
			int serverSetting = m_aCSISettingsAuthorityValues.Get(i);
	
			switch (true)
			{
				case (IsBool && serverSetting == SERVER_OVERRIDE_FALSE) : settingValue = 0; break;
				case (IsBool && serverSetting == SERVER_OVERRIDE_TRUE) : settingValue = 1; break;
				case (serverSetting < 0) : settingValue = Math.AbsInt(serverSetting); break;
				default : m_UserSettigs.Get(setting, settingValue);
			}
			
			m_mCSISettingsLocalValues.Set(setting, settingValue);
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