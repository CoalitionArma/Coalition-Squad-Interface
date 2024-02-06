class CSI_GameSettings: ModuleGameSettings
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
	int squadRadarIconSize;
	
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
}