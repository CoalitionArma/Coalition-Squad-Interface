class CSI_GameSettings : ModuleGameSettings
{
	[Attribute("")]
	string compassVisible;

	[Attribute("{D19C93F5109F3E1D}UI\Textures\HUD\Modded\Compasses\compass_shadow360.edds")]
	string compassTexture;

	[Attribute("")]
	string squadRadarVisible;

	[Attribute("")]
	string groupDisplayVisible;

	[Attribute("")]
	string staminaBarVisible;

	[Attribute("")]
	string nametagsVisible;

	[Attribute("")]
	string rankVisible;

	[Attribute("100")]
	string squadRadarIconSize;

	[Attribute("BODY")]
	string nametagsPosition;

	[Attribute("")]
	string nametagsRange;

	[Attribute("true")]
	string squadRadarSelfIconVisible;

	[Attribute("")]
	string roleNametagVisible;

	[Attribute("")]
	string personalColorTeamMenu;
	
	[Attribute("")]
	string groupNametagVisible;

	[Attribute("")]
	string nametagLOSEnabled;
	
	[Attribute("false")]
	string autoHideUI;
	
	[Attribute("false")]
	CSI_EIconThemes roleTheme;
}
