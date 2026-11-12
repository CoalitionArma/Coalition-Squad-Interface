//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Nametag element showing the player's group name under their name. New class (not modded),
//! 				used in NametagFriendlies.conf.
//!
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTGroupName : SCR_NTTextBase
{
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		m_HUDSystem = CSI_HUDSystem.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();

		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!stateConf || !tWidget)
			return;

		//-----------------------------------------------------------------------
		// Font, size (two smaller than the name) and neutral color
		tWidget.SetFont(m_FontResource);
		tWidget.SetExactFontSize(GetNametagTextScale() - 2);
		tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));

		data.GetGroupName();

		//-----------------------------------------------------------------------
		// Actually Make The Widget Visible
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions
	};

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);

		//-----------------------------------------------------------------------
		// Update Widget Opacity
		data.UpdateOpacity(null, data: data, index: index);

		//-----------------------------------------------------------------------
		// Set The Text Of The Widget Based On Our Current Distance
		if (ShouldWeClearText(data))
			SetText(data, string.Empty, data.m_aNameParams, index);
		else
			SetText(data, data.m_sGroupName, data.m_aNameParams, index);
	}
};