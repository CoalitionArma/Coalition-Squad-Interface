//------------------------------------------------------------------------------------
// Enumeration for Color Teams
//------------------------------------------------------------------------------------

enum CSI_EColorTeam
{
	NONE,
	BLUE,
	RED,
	GREEN,
	YELLOW
}

//------------------------------------------------------------------------------------
// Enumeration for Arrows
//------------------------------------------------------------------------------------

enum CSI_EArrowTheme
{
	CLASSIC = 1,
	STUBBY,
	POINTIER,
	ROUNDIER,
	SIMPLE,
}

//------------------------------------------------------------------------------------
// Enumeration for each players Icon
//------------------------------------------------------------------------------------

enum CSI_EIconType
{
	REGULAR = 1,
	OUTLINE
}

enum CSI_EIconTheme
{
	CLASSIC = 1,
	REFORGER_ADV,
	REFORGER_REG,
	NATO,
	SCI_FI,
	WW2,
	MEDIVAL,
}

enum CSI_EIcon
{
	// ----------- BASE ICONS -----------
	MAN,
	OFFICER,
	SL,
	TL,
	RTO,
	ENG,
	AT,
	DEMO,
	SNIPER,
	GREN,
	MG,
	MEDIC,
	// ----------- VEHICLE ICONS -----------
	DRIVER,
	PASSANGER,
	COMMANDER,
	GUNNER,
	HELIPILOT,
	HELICREW,
}

enum CSI_EOverrideIcon
{
	AUTO = 0,
	RTO = 4,
	ENG,
	AT,
	DEMO,
	SNIPER,
	GREN,
	MG,
	MEDIC,
}

//------------------------------------------------------------------------------------
// Enumeration for compass themes
//------------------------------------------------------------------------------------

enum CSI_ECompassTheme
{
	REGULAR = 1,
	SCI_FI,
	WW2,
}