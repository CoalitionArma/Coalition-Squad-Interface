class CSI_GameSettings : ModuleGameSettings
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Compass")]
	bool m_iCompassVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Bearing Above The Compass")]
	bool m_iBearingVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Radar Inside The Compass")]
	bool m_iRadarVisible;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Only Have The Arrows Of The Icons Rotate To Indicate The Direction A Player Is Facing On The Radar")]
	bool m_iOnlyRadarIconArrowsRotate;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Group Display To The Right Of The Compass")]
	bool m_iGroupVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Stamina Bar")]
	bool m_iStaminaVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Nametags")]
	bool m_iNametagVisible;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Ranks In Players Names")]
	bool m_iRankVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Roles In Players Nametags")]
	bool m_iRoleInNametagVisible;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Groups In Players Nametags")]
	bool m_iGroupInNametagVisible;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Nametags Utilizing Line Of Sight To Determine Visiblity")]
	bool m_iNametagLOSVisible;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable The CSI HUD Automatically Hiding Itself Until A Key Is Pressed")]
	bool m_iAutoHideHUD;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconTheme), desc: "Theme Of All Specialty Icons")]
	int m_iIconTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EIconType.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EIconType), desc: "Type Of Icon To Show On Player Screens")]
	int m_iIconType;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_EArrowTheme.CLASSIC), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_EArrowTheme), desc: "Arrow To Give Icons")]
	int m_iArrowTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(CSI_ECompassTheme.REGULAR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CSI_ECompassTheme), desc: "Theme Of The Compass")]
	int m_iCompassTheme;

	[Attribute(defvalue: SCR_Enum.GetDefault(ENameTagPosition.BODY), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENameTagPosition), desc: "Position To Put Nametags")]
	int m_iNametagPosition;
	
	[Attribute(defvalue: "8", uiwidget: UIWidgets.Slider, params: "0 10 1", desc: "Z Offset Of Nametags From The Nametag Position")]
	int m_iNametagPositionOffset;

	[Attribute(defvalue: "35", uiwidget: UIWidgets.Slider, params: "5 2500 5", desc: "Nametag Range")]
	int m_iNametagRange;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "1 10 1", desc: "When Looking Through A Magnified Sight/Binos, Multiply The Nametag Range By This Number")]
	int m_iNametagMagnificationMultiplication;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "10 300 10", desc: "Icon Size On The Radar")]
	int m_iRadarIconSize;
}
