//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the name text uses CSI's size and colors and is cleared when the tag is simplified.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTName : SCR_NTTextBase
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
		// Font, size from NAMTEAG_SCALE and color team color
		tWidget.SetFont(m_FontResource);
		tWidget.SetExactFontSize(GetNametagTextScale());
		SetWidgetCSIColor(data, index);

		//-----------------------------------------------------------------------
		// Actually Make The Widget Visible
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions
	};

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		//-----------------------------------------------------------------------
		// CSI: brightness-based opacity
		data.UpdateOpacity(null, data: data, index: index);

		//-----------------------------------------------------------------------
		// Base element setup, done here since super also sets the text
		if ( !data.m_aNametagElements[index] )
		{
			InitElementWidget(data, index);
			return;
		}

		// scale
		if (m_bScaleElement)
		{
			data.m_fTimeSliceUpdate = 1.0;
			ScaleElement(data, index);
		}

		//-----------------------------------------------------------------------
		// Name update (text is set below)
		if (data.m_Flags & ENameTagFlags.NAME_UPDATE || data.m_Flags & ENameTagFlags.ENT_TYPE_UPDATE)
		{
			string name;
			array<string> nameParams = {};

			GetText(data, name, nameParams);

			// Keep the flags until a name is found, so it's retried
			if (name.IsEmpty())
				SetText(data, "GETNAME_ERROR", nameParams, index);
			else
			{
				data.m_Flags &= ~ENameTagFlags.NAME_UPDATE;
				data.m_Flags &= ~ENameTagFlags.ENT_TYPE_UPDATE;
			}
		}

		//-----------------------------------------------------------------------
		// CSI: clear text when simplified
		if (ShouldWeClearText(data))
			SetText(data, string.Empty, data.m_aNameParams, index);
		else
			SetText(data, data.m_sName, data.m_aNameParams, index);

		//-----------------------------------------------------------------------
		// CSI: color updates for VON/color teams
		SetWidgetCSIColor(data, index);
	}
};