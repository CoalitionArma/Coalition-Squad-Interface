//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	CSI's user settings module (the CSI tab in Settings). Read values through CSI_SettingsManager,
//! 				which applies server overrides on top of these.
//! 
//------------------------------------------------------------------------------------------------
class CSI_GameSettings : ModuleGameSettings
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC STRING POINTERS
//=============================================================================================================================================================================================================================================================================================================================================================

	const static string COMPASS_VISIBLE = "m_bCompassVisible";
	const static string BEARING_VISIBLE = "m_bBearingVisible";
	const static string RADAR_VISIBLE = "m_bRadarVisible";
	const static string RADAR_ICON_ARROWS_ROTATE = "m_bRadarIconArrowsRotate";
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
	const static string NAMTEAG_RANGE_SIMPLIFIED = "m_iNametagRangeSimplifiedPercentage";
	const static string NAMTEAG_SCALE = "m_iNametagScale";
	const static string NAMETAG_MAGNIFICATION_MULTIPLICATION = "m_iNametagMagnificationMultiplication";
	const static string RADAR_ICON_SIZE = "m_iRadarIconSize";
	const static string NAMETAG_BRIGHTNESS_DIMMING = "m_bNametagBrightnessDimming";
	const static string NAMETAG_FIELD_OF_VIEW = "m_iNametagFieldOfView";
	const static string NAMETAG_FIELD_OF_VIEW_SIMPLIFIED = "m_iNametagFieldOfViewSimplifiedPercentage";
	const static string RADAR_FIELD_OF_VIEW = "m_iRadarFieldOfView";
	const static string RADAR_RANGE_ON_FOOT = "m_iRadarRangeOnFoot";
	const static string RADAR_RANGE_IN_VEHICLE = "m_iRadarRangeInVehicle";

	const static int NAMETAG_FIELD_OF_VIEW_MIN = 5;	//!< degrees, same as the m_iNametagFieldOfView slider
	const static int NAMETAG_FIELD_OF_VIEW_MAX = 125;	//!< degrees, same as the m_iNametagFieldOfView slider
	const static int NAMETAG_FIELD_OF_VIEW_SIMPLIFIED_MAX = 95;	//!< percent, same as the m_iNametagFieldOfViewSimplifiedPercentage slider
	const static int RADAR_FIELD_OF_VIEW_MIN = 5;		//!< degrees, same as the m_iRadarFieldOfView slider
	const static int RADAR_FIELD_OF_VIEW_MAX = 360;		//!< degrees, same as the m_iRadarFieldOfView slider; nobody fades at this value
	const static int RADAR_RANGE_ON_FOOT_MIN = 5;		//!< metres, same as the m_iRadarRangeOnFoot slider
	const static int RADAR_RANGE_ON_FOOT_MAX = 100;		//!< metres, same as the m_iRadarRangeOnFoot slider
	const static int RADAR_RANGE_IN_VEHICLE_MIN = 1;	//!< metres, same as the m_iRadarRangeInVehicle slider
	const static int RADAR_RANGE_IN_VEHICLE_MAX = 10;	//!< metres, same as the m_iRadarRangeInVehicle slider

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTINGS ARRAY (USED FOR AUTHORITY OVERRIDE OF SETTINGS)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	// Server overrides and CSI_Settings.json store values by index in this array: only ever append, never insert or reorder
    const static ref TStringArray m_aSettingsArray =
	{
		COMPASS_VISIBLE,
		BEARING_VISIBLE,
		RADAR_VISIBLE,
		RADAR_ICON_ARROWS_ROTATE,
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
		RADAR_ICON_SIZE,
		NAMTEAG_RANGE_SIMPLIFIED,
		NAMTEAG_SCALE,
		NAMETAG_BRIGHTNESS_DIMMING,
		NAMETAG_FIELD_OF_VIEW,
		NAMETAG_FIELD_OF_VIEW_SIMPLIFIED,
		RADAR_FIELD_OF_VIEW,
		RADAR_RANGE_ON_FOOT,
		RADAR_RANGE_IN_VEHICLE
	};

	//------------------------------------------------------------------------------------------------
	// Use the boolean server override encoding (see CSI_SettingsManager)
	const static ref TStringArray SETTINGS_THAT_ARE_BOOLEAN =
	{
		COMPASS_VISIBLE,
		BEARING_VISIBLE,
		RADAR_VISIBLE,
		RADAR_ICON_ARROWS_ROTATE,
		GROUP_VISIBLE,
		STAMINA_VISIBLE,
		NAMETAG_VISIBLE,
		RANK_VISIBLE,
		ROLE_IN_NAMETAG_VISIBLE,
		GROUP_IN_NAMETAG_VISIBLE,
		NAMETAG_LOS_VISIBLE,
		AUTO_HIDE_HUD,
		NAMETAG_BRIGHTNESS_DIMMING
	};

	//------------------------------------------------------------------------------------------------
	//! \return setting names in replication (index) order
	static TStringArray GetSettingsArray()
	{
		return m_aSettingsArray;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ACTUAL SETTINGS
//=============================================================================================================================================================================================================================================================================================================================================================

	// General
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Shows each player's short rank before their name on the HUD, nametags and group menu.")]
	int m_bRankVisible;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Hides the whole CSI HUD (compass, group display and stamina bar) until you hold the Reveal UI key.")]
	int m_bAutoHideHUD;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows a stamina bar in the bottom left corner whenever you're not at full stamina.")]
	int m_bStaminaVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Lists your group's members with their role icons beside the compass, coloured by fire team (groups of 2+).")]
	int m_bGroupVisible;

	// Icons
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconTheme), desc: "Art style of the role icons (medic, MG, AT and more) on your HUD, radar, group menu and nametags.")]
	int m_iIconTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconType.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconType), desc: "Regular shows the full themed icons; Inverted shows simpler inverted icons with an outline.")]
	int m_iIconType;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EArrowTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EArrowTheme), desc: "Shape of the arrow around HUD and radar icons that shows where a player is facing.")]
	int m_iArrowTheme;

	// Nametags
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows nametags above friendly players.")]
	int m_bNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows the group name under other groups' nametags. Your own group's tags never show it.")]
	int m_bGroupInNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Hides nametags of players behind walls, terrain or vehicles.")]
	int m_bNametagLOSVisible;

	[Attribute(defvalue: SCR_Enum.GetDefault(ENameTagPosition.BODY), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENameTagPosition), desc: "Places nametags above the head or on the body.")]
	int m_iNametagPosition;

	[Attribute(defvalue: "3", uiwidget: UIWidgets.Slider, params: "0 5 1", desc: "Raises or lowers the nametag from that position.")]
	int m_iNametagPositionOffset;

	[Attribute(defvalue: "50", uiwidget: UIWidgets.Slider, params: "5 2500 5", desc: "How far away friendly nametags appear.")]
	int m_iNametagRange;

	[Attribute(defvalue: "85", uiwidget: UIWidgets.Slider, params: "0 100 1", desc: "The farthest part of the range shows only the role icon, not the name.")]
	int m_iNametagRangeSimplifiedPercentage;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 500 10", desc: "Multiplies the nametag range while looking through scopes or binoculars.")]
	int m_iNametagMagnificationMultiplication;

	[Attribute(defvalue: "125", uiwidget: UIWidgets.Slider, params: "5 125 5", desc: "How far from where you're looking a teammate can be and still show a nametag. The angle narrows with distance.")]
	int m_iNametagFieldOfView;

	[Attribute(defvalue: "50", uiwidget: UIWidgets.Slider, params: "0 95 1", desc: "The outer part of the field of view shows only the role icon, not the name.")]
	int m_iNametagFieldOfViewSimplifiedPercentage;

	[Attribute(defvalue: "40", uiwidget: UIWidgets.Slider, params: "0 100 20", desc: "Size of nametag text and icons.")]
	int m_iNametagScale;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows the player's role icon on their nametag.")]
	int m_bRoleIconInNametagVisible;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ENametagIconPosition.LEFT), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ENametagIconPosition), desc: "Puts the role icon left of, above, or right of the name.")]
	int m_iNametagRoleIconPosition;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Fades nametags in dark scenes such as night, and brightens them under NVGs.")]
	int m_bNametagBrightnessDimming;

	// Compass & Radar
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ECompassTheme.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ECompassTheme), desc: "Style of the compass. Only Regular exists so far.")]
	int m_iCompassTheme;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows the compass at the bottom of the screen.")]
	int m_bCompassVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows your heading in degrees above the compass.")]
	int m_bBearingVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows nearby group members inside the compass, within the radar ranges below.")]
	int m_bRadarVisible;

	[Attribute(defvalue: "25", uiwidget: UIWidgets.Slider, params: "5 100 1", desc: "How far away squadmates show on the radar while you're on foot.")]
	int m_iRadarRangeOnFoot;

	[Attribute(defvalue: "4", uiwidget: UIWidgets.Slider, params: "1 10 1", desc: "How far away squadmates show on the radar while you're in a vehicle.")]
	int m_iRadarRangeInVehicle;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 300 10", desc: "Size of squadmate icons on the radar.")]
	int m_iRadarIconSize;

	[Attribute(defvalue: "360", uiwidget: UIWidgets.Slider, params: "5 360 5", desc: "Squadmates outside this angle around where you're looking slowly fade from the radar. 360° shows everyone.")]
	int m_iRadarFieldOfView;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Only the arrow turns to show where squadmates face; the icon stays upright.")]
	int m_bRadarIconArrowsRotate;
}
