//------------------------------------------------------------------------------------------------
//! Base nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{
	protected CSI_PlayerData m_StoredPlayerData;
	
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
		{
			tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
			tWidget.SetText(data.GetGroupName());
		};
		
		if (data.m_iPlayerID > 0) 
		{	
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(data.m_iPlayerID);
			
			if (playerData)	
				m_StoredPlayerData = playerData;
			else
				m_StoredPlayerData = null;
		};
		
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions	
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		super.UpdateElement(data, index);
		
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		if (data.m_iPlayerID > 0 && m_StoredPlayerData && m_StoredPlayerData.GetPlayerID() == data.m_iPlayerID)
		{
			if (tWidget.GetName() == "PlayerName" && CSI_HUDManager.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(m_StoredPlayerData.GetColorTeam()));
		} else 
			tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
	}		
}

//------------------------------------------------------------------------------------------------
//! Group nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase {}