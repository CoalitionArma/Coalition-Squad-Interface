class CSI_GameSettings : ModuleGameSettings
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC STRING POINTERS TO SETTING VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	const static string COMPASS_VISIBLE = "m_bCompassVisible";
	const static string BEARING_VISIBLE = "m_bBearingVisible";
	const static string RADAR_VISIBLE = "m_bRadarVisible";
	const static string ONLY_RADAR_ICON_ARROWS_ROTATE = "m_bOnlyRadarIconArrowsRotate";
	const static string GROUP_VISIBLE = "m_bGroupVisible";
	const static string STAMINA_VISIBLE = "m_bStaminaVisible";
	const static string NAMETAG_VISIBLE = "m_bNametagVisible";
	const static string RANK_VISIBLE = "m_bRankVisible";
	const static string ROLE_IN_NAMETAG_VISIBLE = "m_bRoleIconInNametagVisible";
	const static string GROUP_IN_NAMETAG_VISIBLE = "m_bGroupInNametagVisible";
	const static string NAMETAG_LOS_VISIBLE = "m_bNametagLOSVisible";
	const static string AUTO_HIDE_HUD = "m_bAutoHideHUD";
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
	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTINGS ARRAY (USED FOR AUTHORITY OVERRIDE OF SETTINGS)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
    static ref TStringArray m_aSettingsArray = 
	{
		//BOOLEAN SETTINGS
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
	
		//INTIGER SETTINGS
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

	const static int INDEX_WHERE_BOOL_SETTINGS_STOP = 11;

	//------------------------------------------------------------------------------------------------
	static TStringArray GetSettingsArray() 
	{
		return m_aSettingsArray;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ACTUAL SETTINGS
//=============================================================================================================================================================================================================================================================================================================================================================

	// General
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Ranks In Players Names")]
	int m_bRankVisible;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable The CSI HUD Automatically Hiding Itself Until A Key Is Pressed")]
	int m_bAutoHideHUD;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Stamina Bar")]
	int m_bStaminaVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Group Display To The Right Of The Compass")]
	int m_bGroupVisible;
	
	// Icons
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconTheme), desc: "Theme Of All Specialty Icons")]
	int m_iIconTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconType.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconType), desc: "Type Of Icon To Show On Player Screens")]
	int m_iIconType;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EArrowTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EArrowTheme), desc: "Arrow To Give Icons")]
	int m_iArrowTheme;
	
	// Nametags
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Nametags")]
	int m_bNametagVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Groups In Players Nametags")]
	int m_bGroupInNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Nametags Utilizing Line Of Sight To Determine Visiblity")]
	int m_bNametagLOSVisible;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(ENameTagPosition.BODY), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENameTagPosition), desc: "Position To Put Nametags")]
	int m_iNametagPosition;
	
	[Attribute(defvalue: "3", uiwidget: UIWidgets.Slider, params: "0 5 1", desc: "Z Offset Of Nametags From The Nametag Position")]
	int m_iNametagPositionOffset;

	[Attribute(defvalue: "35", uiwidget: UIWidgets.Slider, params: "5 2500 5", desc: "Nametag Range")]
	int m_iNametagRange;
	
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 500 10", desc: "When Looking Through A Magnified Sight/Binos, Multiply The Nametag Range By This Percent")]
	int m_iNametagMagnificationMultiplication;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Roles In Players Nametags")]
	int m_bRoleIconInNametagVisible;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ENametagIconPosition.LEFT), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ENametagIconPosition), desc: "Position To Put Nametag Icons")]
	int m_iNametagRoleIconPosition;
	
	// Compass & Radar
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ECompassTheme.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ECompassTheme), desc: "Theme Of The Compass")]
	int m_iCompassTheme;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Compass")]
	int m_bCompassVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Bearing Above The Compass")]
	int m_bBearingVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Radar Inside The Compass")]
	int m_bRadarVisible;
	
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 300 10", desc: "Icon Size On The Radar")]
	int m_iRadarIconSize;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Only Have The Arrows Of The Icons Rotate To Indicate The Direction A Player Is Facing On The Radar")]
	int m_bOnlyRadarIconArrowsRotate;
}
