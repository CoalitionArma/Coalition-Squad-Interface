class CSI_GameSettings: ModuleGameSettings
{
	[Attribute()]
	int compassVisible;
	
	[Attribute()]
	string compassTexture;
	
	[Attribute()]
	int squadRadarVisible;
	
	[Attribute()]
	int groupDisplayVisible;
	
	[Attribute()]
	int staminaBarVisible;
	
	[Attribute()]
	int nametagsVisible;
	
	[Attribute()]
	int rankVisible;
	
	[Attribute()]
	float squadRadarIconSize;

	[Attribute()]
	bool clientSetupCompleted;
	
	//Server Overrides
	
	[Attribute()]
	int compassVisibleServerOverride;
	
	[Attribute()]
	int squadRadarVisibleServerOverride;

	[Attribute()]
	int groupDisplayVisibleServerOverride;
	
	[Attribute()]
	int staminaBarVisibleServerOverride;
	
	[Attribute()]
	int nametagsVisibleServerOverride;

	[Attribute()]
	int rankVisibleServerOverride;
	
	[Attribute()]
	bool serverSetupCompleted;
}