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

enum CSI_EArrow
{
	CLASSIC,
	STUBBY,
	POINTIER,
	ROUNDIER,
	SIMPLE,
}

//------------------------------------------------------------------------------------
// Enumeration for each players Icon
//------------------------------------------------------------------------------------

enum CSI_EIconThemes
{
	CLASSIC,
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