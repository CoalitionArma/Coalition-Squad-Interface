//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the platform icon element is disabled; it neither sets up nor updates its widget.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconPlatform : SCR_NTIconBase
{
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		return;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		return;
	}
}