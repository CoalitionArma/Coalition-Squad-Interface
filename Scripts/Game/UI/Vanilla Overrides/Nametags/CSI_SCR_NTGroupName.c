//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Nametag element showing the player's group name under their name; used in NametagFriendlies.conf.
//! 				Left empty for squadmates with Squadmate group names hidden.
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
		tWidget.SetColor(CSI_ColorPresets.GetColor(CSI_EColorPreset.GRAY));

		data.GetGroupName();

		//-----------------------------------------------------------------------
		// Actually Make The Widget Visible
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions
	};

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		// A name update in super sets the player's name here, so the group name is set again below
		bool nameUpdate = data.m_Flags & ENameTagFlags.NAME_UPDATE || data.m_Flags & ENameTagFlags.ENT_TYPE_UPDATE;

		super.UpdateElement(data, index);

		if (!data.m_aNametagElements[index])
			return;

		//-----------------------------------------------------------------------
		// Set The Text Of The Widget Based On Our Current Distance, only when it changes
		string text = data.m_sGroupName;
		if (ShouldWeClearText(data) || (!m_SettingsManager.GetSettingBool(CSI_GameSettings.SQUADMATE_GROUP_IN_NAMETAG_VISIBLE) && IsLocalGroupMember(data)))
			text = string.Empty;

		if (nameUpdate || text != data.m_aElementTexts[index])
		{
			SetText(data, text, data.m_aNameParams, index);
			data.m_aElementTexts[index] = text;
		}
	}
};