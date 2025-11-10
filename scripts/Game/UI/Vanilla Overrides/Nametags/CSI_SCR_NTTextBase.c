//------------------------------------------------------------------------------------------------
//! Base nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{		
	protected CSI_HUDManager m_HUDManager;
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;

	protected CSI_PlayerData m_StoredPlayerData;
	protected SCR_NameTagData m_StoredNameTagData;
	protected int m_iStoredIndex;
	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{	
		if (!m_HUDManager || !m_PlayerDataManager)
		{
			m_HUDManager = CSI_HUDManager.GetInstance();
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		};

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
		
		if (tWidget.GetName() == "PlayerName" && data.m_iPlayerID > 0) 
		{	
			CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(data.m_iPlayerID);
			
			if (playerData && m_HUDManager.GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
			{	
				m_StoredPlayerData = playerData;
				m_StoredNameTagData = data;
				m_iStoredIndex = index;

				UpdatePlayerText();
				m_SettingsManager.GetOnSettingsUpdate().Insert(UpdatePlayerText);
				playerData.GetOnDataUpdate().Insert(UpdatePlayerText);
			};
		};
		
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions	
	}
	
	protected void UpdatePlayerText()
	{	
		if (!m_StoredNameTagData 
			|| !m_StoredPlayerData 
			|| m_StoredNameTagData.m_iPlayerID <= 0 
			|| (m_StoredPlayerData.GetPlayerID() != m_StoredNameTagData.m_iPlayerID))
		{
			m_SettingsManager.GetOnSettingsUpdate().Remove(UpdatePlayerText);
			
			if (m_StoredPlayerData)
				m_StoredPlayerData.GetOnDataUpdate().Remove(UpdatePlayerText);
			return;
		};
		
		TextWidget tWidget = TextWidget.Cast( m_StoredNameTagData.m_aNametagElements[m_iStoredIndex] );
		if (!tWidget)
			return;

		tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(m_StoredPlayerData.GetColorTeam()));
	}		
}

//------------------------------------------------------------------------------------------------
//! Group nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase {}