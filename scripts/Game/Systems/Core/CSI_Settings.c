class CSI_GameSettings : ModuleGameSettings
{
	// Using Int's because I can flip them to negative to indicate if the server has overriden a setting (without having to make a whole seprate list of "server override" settings)
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Compass")]
	int m_iCompassVisible;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Bearing Above The Compass")]
	int m_iBearingVisible;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Radar Inside The Compass")]
	int m_iRadarVisible;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Group Display To The Right Of The Compass")]
	int m_iGroupVisible;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of The Stamina Bar")]
	int m_iStaminaVisible;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Nametags")]
	int m_iNametagVisible;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Ranks In Players Names")]
	int m_iRankVisible;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Roles In Players Nametags")]
	int m_iRoleInNametagVisible;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Visibility Of Groups In Players Nametags")]
	int m_iGroupInNametagVisible;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable Nametags Utilizing Line Of Sight To Determine Visiblity")]
	int m_iNametagLOSVisible;
	
	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Enable/Disable The CSI HUD Auto Hiding Until A Key Is Pressed")]
	int m_iAutoHideHUD;
	
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

	[Attribute(defvalue: "32", uiwidget: UIWidgets.Slider, params: "5 2500 5", desc: "Nametag Range")]
	int m_iNametagRange;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "1 10 1", desc: "When Looking Through A Magnified Sight/Binos, Multiply The Nametag Range By This Number")]
	int m_iNametagMagnificationMultiplication;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, params: "40 500 5", desc: "Icon Size On The Radar")]
	int m_iRadarIconSize;
}
