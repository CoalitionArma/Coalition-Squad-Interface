//------------------------------------------------------------------------------------------------
//! Base nametag element for text
modded class SCR_NTTextBase : SCR_NTElementBase
{			
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		super.SetDefaults(data, index);
		
		data.UpdateAttatchedTo();
		
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		tWidget.SetFont(m_FontResource);
		
		if (!m_bScaleElement)
			tWidget.SetExactFontSize(m_fTextSizeMax);
		
		tWidget.SetShadow(stateConf.m_fShadowSize, stateConf.m_vShadowColor.PackToInt(), stateConf.m_fShadowOpacity, 0, 0);
		
		if (tWidget.GetName() == "PlayerGroupName") {
			tWidget.SetText(data.GetGroupName()); 
		};
		
		if (tWidget.GetName() == "PlayerName") {
			int ct = data.GetPlayerColorTeam();
			if (ct == 0)
				tWidget.SetColor(stateConf.m_vColor);
			else
				tWidget.SetColorInt(ct);
		};
		
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, true); // transitions	
	}
}

//------------------------------------------------------------------------------------------------
//! Nametag element for group text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase
{
	
}