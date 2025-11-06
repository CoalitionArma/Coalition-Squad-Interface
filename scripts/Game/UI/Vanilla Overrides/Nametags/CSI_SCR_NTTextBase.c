//------------------------------------------------------------------------------------------------
//! Base nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{		
	protected CSI_HUDManager m_HUDManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;

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
			tWidget.SetText(data.GetGroupName()); 
		
		if (tWidget.GetName() == "PlayerName") 
		{	
			bool ctSet = false;
			if (data.m_iPlayerID > 0)
			{
				Color ct;
				CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(data.m_iPlayerID);
				array<int> groupArray = m_HUDManager.GetLocalGroupPlayerIds();
				if (playerData && groupArray.Contains(data.m_iPlayerID))
				{
					ct = CSI_UIHelper.ConvertColorTeamToColor(playerData.GetColorTeam());
					playerData.GetOnDataUpdate().Insert(DataRefresh);
				};
				
				if (ct && !ct.IsZero()) 
				{
					ctSet = true;
					tWidget.SetColor(ct);
				}
			}
			
			if (!ctSet)
				tWidget.SetColor(stateConf.m_vColor);
		};
		
		data.UpdateAttatchedTo();
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions		
	}
	
	protected void DataRefresh()
	{		
		CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(m_StoredNameTagData.m_iPlayerID);
		
		playerData.GetOnDataUpdate().Remove(DataRefresh);
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Remove(DataRefresh);
		
		SetDefaults(m_StoredNameTagData, m_iStoredIndex);
	}	
}

//------------------------------------------------------------------------------------------------
//! Group nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase {}