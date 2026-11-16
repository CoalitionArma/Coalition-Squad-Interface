//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
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
	const static string REVEAL_UI_MODE = "m_iRevealUIMode";
	const static string COMPASS_COLOR = "m_iCompassColor";
	const static string SQUADMATE_GROUP_IN_NAMETAG_VISIBLE = "m_bSquadmateGroupInNametagVisible";
	const static string STANDARD_SQUAD_COLOR = "m_iNametagSquadmateColor";	// once nametags only; the name keeps saved settings
	const static string NAMETAG_COLOR = "m_iNametagOtherColor";	// once Other nametag color; the field name keeps saved settings
	const static string COLOR_TEAM_SET = "m_iColorTeamSet";
	const static string NAMETAG_VON_COLOR = "m_iNametagVONColor";

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTINGS ARRAY (USED FOR AUTHORITY OVERRIDE OF SETTINGS)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	// Overrides and saved settings use these indices: only ever append
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
		RADAR_RANGE_IN_VEHICLE,
		REVEAL_UI_MODE,
		COMPASS_COLOR,
		SQUADMATE_GROUP_IN_NAMETAG_VISIBLE,
		STANDARD_SQUAD_COLOR,
		NAMETAG_COLOR,
		COLOR_TEAM_SET,
		NAMETAG_VON_COLOR
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
		NAMETAG_BRIGHTNESS_DIMMING,
		SQUADMATE_GROUP_IN_NAMETAG_VISIBLE
	};

	//------------------------------------------------------------------------------------------------
	// Personal preferences the server can't override
	const static ref TStringArray SETTINGS_WITHOUT_SERVER_OVERRIDE =
	{
		REVEAL_UI_MODE
	};

	//------------------------------------------------------------------------------------------------
	// CSI_EColorPreset values, shown as a row of color boxes
	const static ref TStringArray COLOR_SETTINGS =
	{
		COMPASS_COLOR,
		STANDARD_SQUAD_COLOR,
		NAMETAG_COLOR,
		NAMETAG_VON_COLOR
	};

	//------------------------------------------------------------------------------------------------
	// Color settings that can't match a fire team color (CSI_ColorPresets.GetNonClashingPreset)
	const static ref TStringArray NON_TEAM_COLOR_SETTINGS =
	{
		STANDARD_SQUAD_COLOR,
		NAMETAG_COLOR,
		NAMETAG_VON_COLOR
	};

	//------------------------------------------------------------------------------------------------
	// Color settings that can't match the Nametag VON color, so a talking player stands out
	const static ref TStringArray VON_EXCLUSIVE_COLOR_SETTINGS =
	{
		STANDARD_SQUAD_COLOR,
		NAMETAG_COLOR
	};

	//------------------------------------------------------------------------------------------------
	//! \return setting names in replication (index) order
	static TStringArray GetSettingsArray()
	{
		return m_aSettingsArray;
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the min and max in a setting's attribute params, the range of its slider.
	//! \param[in] setting
	//! \param[out] min
	//! \param[out] max
	//! \return false if the setting has no min and max
	static bool GetSettingLimits(string setting, out int min, out int max)
	{
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
		if (!settings)
			return false;

		int varIndex = settings.GetVarIndex(setting);
		if (varIndex < 0)
			return false;

		float minValue, maxValue, step;
		if (settings.GetLimits(varIndex, minValue, maxValue, step) < 2)
			return false;

		min = minValue;
		max = maxValue;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting
	//! \param[in] value
	//! \return value clamped to the setting's min and max, unchanged if it has none
	static int ClampToLimits(string setting, int value)
	{
		int min, max;
		if (!GetSettingLimits(setting, min, max))
			return value;

		return Math.ClampInt(value, min, max);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ACTUAL SETTINGS
//=============================================================================================================================================================================================================================================================================================================================================================

	// General
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Shows each player's short rank before their name in the group display, nametags and group menu.")]
	int m_bRankVisible;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Hides the CSI HUD (compass, radar, group display and stamina bar) until you press the Reveal CSI HUD key, Insert by default.")]
	int m_bAutoHideHUD;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ERevealUIMode.HOLD), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ERevealUIMode), desc: "How the Reveal CSI HUD key brings back the hidden HUD. Hold: shown while you hold the key. Toggle: one press shows it, the next hides it.")]
	int m_iRevealUIMode;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows a stamina bar in the bottom left corner while you're below full stamina. It turns from green to amber to red as you tire.")]
	int m_bStaminaVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Lists your group's members beside the compass with their role icons, colored by fire team, squad leader first. Shows in groups of 2 or more.")]
	int m_bGroupVisible;

	// Icons
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconTheme), desc: "Art style of the role icons (medic, MG, AT and more) in the group display, radar, group menu and nametags.")]
	int m_iIconTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconType.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconType), desc: "Regular cuts each role symbol out of a solid circle; Inverted shows the symbol inside a ring. Used in the group display and on the radar.")]
	int m_iIconType;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EArrowTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EArrowTheme), desc: "Shape of the arrow around group display and radar icons. On the radar it points where each squadmate is facing.")]
	int m_iArrowTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EColorTeamSet.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EColorTeamSet), desc: "Colors of the Red, Blue, Yellow and Green fire teams in the group display, radar, nametags and group menu. Color-blind keeps the four teams apart for most color vision deficiencies.")]
	int m_iColorTeamSet;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EColorPreset.GRAY), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EColorPreset), desc: "Color of your group members who aren't in a fire team: their name and icon in the group display, radar, group menu and nametags. Colors that match a fire team color can't be picked.")]
	int m_iNametagSquadmateColor;

	// Nametags
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows nametags on friendly players. Your own group's members are colored by fire team.")]
	int m_bNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows each player's group under their name: the group's custom name, or its callsign if it has none.")]
	int m_bGroupInNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows the group name on your own group members' nametags too. Hide shows it only for players in other groups.")]
	int m_bSquadmateGroupInNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Hides the nametags of players you can't see, such as behind terrain, walls or vehicles. Off shows them through everything.")]
	int m_bNametagLOSVisible;

	[Attribute(defvalue: SCR_Enum.GetDefault(ENameTagPosition.BODY), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENameTagPosition), desc: "Puts nametags above each player's head or on their torso. Nametag height fine-tunes it.")]
	int m_iNametagPosition;

	[Attribute(defvalue: "3", uiwidget: UIWidgets.Slider, params: "0 5 1", desc: "Moves the nametag in 10 cm steps: higher values put it farther above the head, or higher up the torso.")]
	int m_iNametagPositionOffset;

	[Attribute(defvalue: "50", uiwidget: UIWidgets.Slider, params: "5 2500 5", desc: "How far away, in meters, friendly nametags show.")]
	int m_iNametagRange;

	[Attribute(defvalue: "85", uiwidget: UIWidgets.Slider, params: "0 100 1", desc: "Distant tags show only the role icon, not the name. Higher values start this closer to you; 0% turns it off. Only works with Role icons shown.")]
	int m_iNametagRangeSimplifiedPercentage;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 500 10", desc: "Multiplies the nametag range while you look through a scope or binoculars. 100% keeps the normal range.")]
	int m_iNametagMagnificationMultiplication;

	[Attribute(defvalue: "125", uiwidget: UIWidgets.Slider, params: "5 145 5", desc: "Nametags show only within this angle around where you're looking. It's full width up close and narrows to nothing at the edge of the range.")]
	int m_iNametagFieldOfView;

	[Attribute(defvalue: "50", uiwidget: UIWidgets.Slider, params: "0 95 1", desc: "Tags in this outer part of the field of view show only the role icon, not the name. Only works with Role icons shown.")]
	int m_iNametagFieldOfViewSimplifiedPercentage;

	[Attribute(defvalue: "40", uiwidget: UIWidgets.Slider, params: "0 100 20", desc: "Size of nametag text and icons.")]
	int m_iNametagScale;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows each player's role icon on their nametag, plus unconscious, dead and talking icons. The icon-only settings need it.")]
	int m_bRoleIconInNametagVisible;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ENametagIconPosition.LEFT), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ENametagIconPosition), desc: "Puts the role icon left of the name, above it or right of it.")]
	int m_iNametagRoleIconPosition;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Fades nametags at night and in dark places, and shows them clearly again while night vision is on. Off keeps them at a fixed opacity.")]
	int m_bNametagBrightnessDimming;


	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EColorPreset.GRAY), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EColorPreset), desc: "Color of the nametags of everyone outside your group, AI included. Colors that match a fire team color can't be picked.")]
	int m_iNametagOtherColor;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EColorPreset.AMBER), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EColorPreset), desc: "Color of a nametag while that player is talking on the radio or voice. Colors that match a fire team color can't be picked.")]
	int m_iNametagVONColor;

	// Compass
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ECompassTheme.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ECompassTheme), desc: "Style of the compass: Classic, Sci-fi or WW2. Sci-fi's two faint rings mark a third and two thirds of the radar range.")]
	int m_iCompassTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EColorPreset.WHITE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EColorPreset), desc: "Color of the compass: its ring, marks and letters, and the bearing above it.")]
	int m_iCompassColor;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows the compass at the bottom of the screen, turning as you look around.")]
	int m_bCompassVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows your heading in degrees above the compass.")]
	int m_bBearingVisible;

	// Radar
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Shows nearby squadmates inside the compass, with you in the middle and where you're looking at the top. Shows in groups of 2 or more.")]
	int m_bRadarVisible;

	[Attribute(defvalue: "25", uiwidget: UIWidgets.Slider, params: "5 100 1", desc: "How far away, in meters, squadmates show on the radar while you're on foot. Icons fade out near the edge.")]
	int m_iRadarRangeOnFoot;

	[Attribute(defvalue: "4", uiwidget: UIWidgets.Slider, params: "1 10 1", desc: "How far away, in meters, squadmates show on the radar while you're in a vehicle. Kept short to show who's in your vehicle and in which seat.")]
	int m_iRadarRangeInVehicle;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 300 10", desc: "Size of squadmate icons on the radar. 100% is the normal size.")]
	int m_iRadarIconSize;

	[Attribute(defvalue: "360", uiwidget: UIWidgets.Slider, params: "5 360 5", desc: "Squadmates outside this angle around where you're looking fade from the radar over 2 seconds. 360° shows everyone.")]
	int m_iRadarFieldOfView;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "On: only the arrow turns to show where each squadmate faces and the icon stays upright. Off: the whole icon turns.")]
	int m_bRadarIconArrowsRotate;
}
