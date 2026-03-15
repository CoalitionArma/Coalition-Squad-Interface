//------------------------------------------------------------------------------------------------
//! Nametag element for name text
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
		// Initial Appearence And Settings Of The Text Widget
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
		// Initial setup (can't call super due to the vanilla implimetation, so we just manually do it) - TODO: make less fucky
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
		// Vanilla Implimentation
		if (data.m_Flags & ENameTagFlags.NAME_UPDATE || data.m_Flags & ENameTagFlags.ENT_TYPE_UPDATE)
		{
			string name;
			array<string> nameParams = {};
			
			GetText(data, name, nameParams);
			
			if (name.IsEmpty())
				SetText(data, "GETNAME_ERROR", nameParams, index);
			else
			{
				data.m_Flags &= ~ENameTagFlags.NAME_UPDATE;
				data.m_Flags &= ~ENameTagFlags.ENT_TYPE_UPDATE;
			}
		}

		//-----------------------------------------------------------------------
		// Set The Text Of The Widget Based On Our Current Distance
		if (ShouldWeClearText(data))
			SetText(data, string.Empty, data.m_aNameParams, index);
		else
			SetText(data, data.m_sName, data.m_aNameParams, index);
		
		//-----------------------------------------------------------------------
		// Color Updates For VON/Color Teams
		SetWidgetCSIColor(data, index);
	}
};