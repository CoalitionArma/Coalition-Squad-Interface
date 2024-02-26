class CSI_GameSettings : ModuleGameSettings
{
	[Attribute()]
	string compassVisible;

	[Attribute()]
	string compassTexture;

	[Attribute()]
	string squadRadarVisible;

	[Attribute()]
	string groupDisplayVisible;

	[Attribute()]
	string staminaBarVisible;

	[Attribute()]
	string nametagsVisible;

	[Attribute()]
	string rankVisible;

	[Attribute()]
	string squadRadarIconSize;

	[Attribute()]
	string nametagsPosition;

	[Attribute()]
	string nametagsRange;

	[Attribute()]
	string squadRadarSelfIconVisible;

	[Attribute()]
	string roleNametagVisible;

	[Attribute()]
	string personalColorTeamMenu;

	//Server Overrides

	[Attribute()]
	string compassVisibleServerOverride;

	[Attribute()]
	string squadRadarVisibleServerOverride;

	[Attribute()]
	string groupDisplayVisibleServerOverride;

	[Attribute()]
	string staminaBarVisibleServerOverride;

	[Attribute()]
	string nametagsVisibleServerOverride;

	[Attribute()]
	string rankVisibleServerOverride;

	[Attribute()]
	string nametagsRangeServerOverride;

	[Attribute()]
	string roleNametagVisibleServerOverride;

	[Attribute()]
	string personalColorTeamMenuServerOverride;
}
