//------------------------------------------------------------------------------------------------
//! Base nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{			
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{	
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		tWidget.SetFont(m_FontResource);
		
		if (!m_bScaleElement)
			tWidget.SetExactFontSize(m_fTextSizeMax);
		
		tWidget.SetShadow( stateConf.m_fShadowSize, stateConf.m_vShadowColor.PackToInt(), stateConf.m_fShadowOpacity, 0, 0);
		
		if (tWidget.GetName() == "PlayerGroupName")
			tWidget.SetText(data.GetGroupName()); 
		
		data.UpdateAttatchedTo();
		
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions		
	}
}

//------------------------------------------------------------------------------------------------
//! Group nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTName : SCR_NTTextBase 
{	
	protected int m_iCurrentFrame = 35;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		if (m_iCurrentFrame < 35 && !(data.m_Flags & ENameTagFlags.NAME_UPDATE)) 
		{
			m_iCurrentFrame++;
			return;
		};

		m_iCurrentFrame = 0;
		
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
	
		if (tWidget.GetName() == "PlayerName") 
		{	
			int ct = data.GetPlayerColorTeam();
			if (ct == 0) 
			{
				SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
				if (!stateConf)
					return;
				
				tWidget.SetColor(stateConf.m_vColor);
			} else {
				tWidget.SetColorInt(ct);
			};
			
			string name;
			array<string> nameParams = {};
			
			data.GetName(name, nameParams);
			
			tWidget.SetText(name);
		};	
	}
}

//------------------------------------------------------------------------------------------------
//! Group nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase {}