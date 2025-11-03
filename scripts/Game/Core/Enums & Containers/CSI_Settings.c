class CSI_GameSettings : ModuleGameSettings
{
	// General
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Ranks In Players Names")]
	bool m_bRankVisible;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable The CSI HUD Automatically Hiding Itself Until A Key Is Pressed")]
	bool m_bAutoHideHUD;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Stamina Bar")]
	bool m_bStaminaVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Group Display To The Right Of The Compass")]
	bool m_bGroupVisible;
	
	// Icons
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconTheme), desc: "Theme Of All Specialty Icons")]
	CSI_EIconTheme m_iIconTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconType.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconType), desc: "Type Of Icon To Show On Player Screens")]
	CSI_EIconType m_iIconType;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EArrowTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EArrowTheme), desc: "Arrow To Give Icons")]
	CSI_EArrowTheme m_iArrowTheme;
	
	// Nametags
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Nametags")]
	bool m_bNametagVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Groups In Players Nametags")]
	bool m_bGroupInNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Nametags Utilizing Line Of Sight To Determine Visiblity")]
	bool m_bNametagLOSVisible;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(ENameTagPosition.BODY), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENameTagPosition), desc: "Position To Put Nametags")]
	ENameTagPosition m_iNametagPosition;
	
	[Attribute(defvalue: "3", uiwidget: UIWidgets.Slider, params: "0 5 1", desc: "Z Offset Of Nametags From The Nametag Position")]
	int m_iNametagPositionOffset;

	[Attribute(defvalue: "35", uiwidget: UIWidgets.Slider, params: "5 2500 5", desc: "Nametag Range")]
	int m_iNametagRange;
	
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 500 10", desc: "When Looking Through A Magnified Sight/Binos, Multiply The Nametag Range By This Percent")]
	int m_iNametagMagnificationMultiplication;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Roles In Players Nametags")]
	bool m_bRoleIconInNametagVisible;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ENametagIconPosition.LEFT), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ENametagIconPosition), desc: "Position To Put Nametag Icons")]
	CSI_ENametagIconPosition m_iNametagRoleIconPosition;
	
	// Compass & Radar
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ECompassTheme.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ECompassTheme), desc: "Theme Of The Compass")]
	CSI_ECompassTheme m_iCompassTheme;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Compass")]
	bool m_bCompassVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Bearing Above The Compass")]
	bool m_bBearingVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Radar Inside The Compass")]
	bool m_bRadarVisible;
	
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 300 10", desc: "Icon Size On The Radar")]
	int m_iRadarIconSize;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Only Have The Arrows Of The Icons Rotate To Indicate The Direction A Player Is Facing On The Radar")]
	bool m_bOnlyRadarIconArrowsRotate;
}
