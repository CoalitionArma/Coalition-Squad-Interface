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

	//Server Overrides

	[Attribute("N/A")]
	string compassVisibleServerOverride;

	[Attribute("N/A")]
	string squadRadarVisibleServerOverride;

	[Attribute("N/A")]
	string groupDisplayVisibleServerOverride;

	[Attribute("N/A")]
	string staminaBarVisibleServerOverride;

	[Attribute("N/A")]
	string nametagsVisibleServerOverride;

	[Attribute("N/A")]
	string rankVisibleServerOverride;

	[Attribute("N/A")]
	string nametagsRangeServerOverride;

	[Attribute("N/A")]
	string roleNametagVisibleServerOverride;

	[Attribute("N/A")]
	string personalColorTeamMenuServerOverride;
	
	[Attribute("N/A")]
	string groupNametagVisibleServerOverride;

	[Attribute("N/A")]
	string nametagLOSEnabledServerOverride;
	
	//Server Defaults
	
	[Attribute("false")]
	string serverDefaultsActive;

	[Attribute("true")]
	string compassVisibleServerDefault;

	[Attribute("true")]
	string squadRadarVisibleServerDefault;

	[Attribute("true")]
	string groupDisplayVisibleServerDefault;

	[Attribute("true")]
	string staminaBarVisibleServerDefault;

	[Attribute("true")]
	string nametagsVisibleServerDefault;

	[Attribute("false")]
	string rankVisibleServerDefault;

	[Attribute("35")]
	string nametagsRangeServerDefault;

	[Attribute("false")]
	string roleNametagVisibleServerDefault;

	[Attribute("true")]
	string personalColorTeamMenuServerDefault;

	[Attribute("true")]
	string groupNametagVisibleServerDefault;

	[Attribute("true")]
	string nametagLOSEnabledServerDefault;
}
