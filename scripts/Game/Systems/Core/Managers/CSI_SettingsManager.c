[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Player Component for RPC", color: "0 0 255 255")]
class CSI_SettingsManagerClass : ScriptComponentClass {};

class CSI_SettingsManager : ScriptComponent
{	
	static string COMPASS_VISIBLE = "m_iCompassVisible";
	static string BEARING_VISIBLE = "m_iBearingVisible";
	static string RADAR_VISIBLE = "m_iRadarVisible";
	static string GROUP_VISIBLE = "m_iGroupVisible";
	static string STAMINA_VISIBLE = "m_iStaminaVisible";
	static string NAMETAG_VISIBLE = "m_iNametagVisible";
	static string RANK_VISIBLE = "m_iRankVisible";
	static string ROLE_IN_NAMETAG_VISIBLE = "m_iRoleInNametagVisible";
	static string GROUP_IN_NAMETAG_VISIBLE = "m_iGroupInNametagVisible";
	static string NAMETAG_LOS_VISIBLE = "m_iNametagLOSVisible";
	static string AUTO_HIDE_HUD = "m_iAutoHideHUD";
	static string ICON_THEME = "m_iIconTheme";
	static string ICON_TYPE = "m_iIconType";
	static string ARROW_THEME = "m_iArrowTheme";
	static string COMPASS_THEME = "m_iCompassTheme";
	static string NAMETAG_POSITION = "m_iNametagPosition";
	static string NAMETAG_POSITION_OFFSET = "m_iNametagPositionOffset";
	static string NAMETAG_RANGE = "m_iNametagRange";
	static string NAMETAG_MAGNIFICATION_MULTIPLICATION = "m_iNametagMagnificationMultiplication";
	static string RADAR_ICON_SIZE = "m_iRadarIconSize";
	
	static int INDEX_WHERE_BOOL_SETTINGS_STOP = 10;
	
	protected UserSettings m_UserSettigs;
    protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_AuthorityManager m_AuthorityManager;
	
    protected ref TStringArray m_aCSISettingsArray = {};
	
	[RplProp()]
	protected ref TIntArray m_aCSISettingsAuthorityValues = {};

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		m_UserSettigs = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_AuthorityManager = CSI_AuthorityManager.GetInstance();
		
		m_aCSISettingsArray = {
			COMPASS_VISIBLE,
			BEARING_VISIBLE,
			RADAR_VISIBLE,
			GROUP_VISIBLE,
			STAMINA_VISIBLE,
			NAMETAG_VISIBLE,
			RANK_VISIBLE,
			ROLE_IN_NAMETAG_VISIBLE, //
			GROUP_IN_NAMETAG_VISIBLE,
			NAMETAG_LOS_VISIBLE,
			AUTO_HIDE_HUD,
			ICON_THEME,
			ICON_TYPE,
			ARROW_THEME,
			COMPASS_THEME,
			NAMETAG_POSITION,
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
		int settingValue;
		int index = m_aCSISettingsArray.Find(setting);
		
		if (index != -1)
		{
			int serverSetting = m_aCSISettingsAuthorityValues.Get(index);
	
			if (index <= INDEX_WHERE_BOOL_SETTINGS_STOP)
			{
				if (serverSetting == -1)
					settingValue = 0;
				else if (serverSetting == -2)
					settingValue = 1;
				
			} else if (serverSetting < 0)
				settingValue = Math.AbsInt(serverSetting);
		} else
			m_UserSettigs.Get(setting, settingValue); 
		
		Print(setting);
		Print(settingValue);
		
		return settingValue;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateLocalSetting(string setting, int value)
	{
		if (RplSession.Mode() != RplMode.Client) 
		{
			int index = m_aCSISettingsArray.Find(setting);
			if (index != -1)
			{
				if (index <= INDEX_WHERE_BOOL_SETTINGS_STOP)
				{
					if (value == 0)
						value = -1;
					else
						value = -2;
				} else
					value = -value;
				
				m_UserSettigs.Set(setting, value);
				UpdateAuthorityValueArray();
				return;
			};
		};
		
		m_UserSettigs.Set(setting, value);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateAuthorityValueArray()
	{
		m_aCSISettingsAuthorityValues.Clear();
		
		foreach (string setting : m_aCSISettingsArray)
		{
			int settingValue;
			m_UserSettigs.Get(setting, settingValue); 
			
			m_aCSISettingsAuthorityValues.Insert(settingValue);
		}
		
		Replication.BumpMe();
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