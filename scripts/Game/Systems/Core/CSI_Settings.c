class CSI_GameSettings : ModuleGameSettings
{
	[Attribute("")]
	bool compassVisible;

	[Attribute("")]
	bool squadRadarVisible;

	[Attribute("")]
	bool groupDisplayVisible;

	[Attribute("")]
	bool staminaBarVisible;

	[Attribute("")]
	bool nametagsVisible;

	[Attribute("")]
	bool rankVisible;

	[Attribute("")]
	bool roleInNametagVisible;
	
	[Attribute("")]
	bool groupNametagVisible;

	[Attribute("")]
	bool nametagLOSVisible;
	
	[Attribute("")]
	bool autoHideUI;
	
	[Attribute()]
	CSI_EIconTheme iconTheme;

	[Attribute()]
	CSI_EIconType iconType;

	[Attribute()]
	CSI_EArrowTheme arrowTheme;

	[Attribute()]
	CSI_ECompassTheme compassTheme;

	[Attribute()]
	ENameTagPosition nametagsPosition;

	[Attribute()]
	int nametagsRange;

	[Attribute()]
	int squadRadarIconSize;
}
