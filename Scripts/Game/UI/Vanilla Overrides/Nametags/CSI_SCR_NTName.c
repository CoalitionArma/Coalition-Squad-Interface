//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-19
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the name text uses CSI's size and colors and is cleared when the tag is icon only.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTName : SCR_NTTextBase
{
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!stateConf || !tWidget)
			return;

		//-----------------------------------------------------------------------
		// Font, size from NAMTEAG_SCALE and the tag's color
		tWidget.SetFont(m_FontResource);
		tWidget.SetExactFontSize(CSI_NametagSettings.s_iTextSize);
		SetElementColor(data, index, data.m_TagColor);

		//-----------------------------------------------------------------------
		// Actually Make The Widget Visible
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions
	};

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		// A name update in super sets the text, so it's set again below
		bool nameUpdate = data.m_Flags & ENameTagFlags.NAME_UPDATE || data.m_Flags & ENameTagFlags.ENT_TYPE_UPDATE;

		super.UpdateElement(data, index);

		if (!data.m_aNametagElements[index])
			return;

		//-----------------------------------------------------------------------
		// Name, cleared when the tag is icon only; set only when it changes
		string text = data.m_sName;
		if (data.m_bIconOnly)
			text = string.Empty;

		if (nameUpdate || text != data.m_aElementTexts[index])
		{
			SetText(data, text, data.m_aNameParams, index);
			data.m_aElementTexts[index] = text;
		}

		//-----------------------------------------------------------------------
		// Color updates for VON/color teams
		SetElementColor(data, index, data.m_TagColor);
	}
};