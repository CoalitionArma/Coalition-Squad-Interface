//------------------------------------------------------------------------------------------------
//! Base nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{		
	protected CSI_HUDManager m_HUDManager;
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
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		};

		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Insert(DataRefresh);
		m_StoredNameTagData = data;
		m_iStoredIndex = index;
		
		tWidget.SetFont(m_FontResource);
		
		if (!m_bScaleElement)
			tWidget.SetExactFontSize(m_fTextSizeMax);
		
		tWidget.SetShadow( stateConf.m_fShadowSize, stateConf.m_vShadowColor.PackToInt(), stateConf.m_fShadowOpacity, 0, 0);
		
		if (tWidget.GetName() == "PlayerGroupName")
		{
			tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
			tWidget.SetText(data.GetGroupName()); 
		};
		
		if (tWidget.GetName() == "PlayerName") 
		{	
			Color colorTeam = stateConf.m_vColor;
			
			if (data.m_iPlayerID > 0)
			{
				CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(data.m_iPlayerID);
				array<int> groupArray = m_HUDManager.GetLocalGroupPlayerIds();
				if (playerData && groupArray.Contains(data.m_iPlayerID))
				{
					colorTeam = CSI_UIHelper.ConvertColorTeamToColor(playerData.GetColorTeam());
					
					m_StoredPlayerData = playerData;
					playerData.GetOnDataUpdate().Insert(DataRefresh);
				};
			};
			
			tWidget.SetColor(colorTeam);
		};
		
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions	
	}
	
	
	
	protected void DataRefresh()
	{	
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Remove(DataRefresh);
		if (m_StoredPlayerData)
			m_StoredPlayerData.GetOnDataUpdate().Remove(DataRefresh);
		
		if (m_StoredNameTagData.m_iPlayerID > 0 && m_StoredNameTagData.m_CharController && m_StoredNameTagData.m_CharController.GetCharacter())
		{
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(m_StoredNameTagData.m_iPlayerID);
			if (playerData == m_StoredPlayerData)
				SetDefaults(m_StoredNameTagData, m_iStoredIndex);
		} else {
			TextWidget tWidget = TextWidget.Cast( m_StoredNameTagData.m_aNametagElements[m_iStoredIndex] );
			if (!tWidget)
				return;
			
			m_StoredNameTagData.SetVisibility(tWidget, false, 0, false);
		}
	}		
}

//------------------------------------------------------------------------------------------------
//! Group nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase {}