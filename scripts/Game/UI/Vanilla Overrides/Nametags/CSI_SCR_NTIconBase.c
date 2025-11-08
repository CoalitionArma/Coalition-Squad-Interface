[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
{
	protected CSI_PlayerData m_StoredPlayerData;
	protected SCR_NameTagData m_StoredNameTagData;
	protected int m_iStoredIndex;
	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Insert(DataRefresh);
		m_StoredNameTagData = data;
		m_iStoredIndex = index;
		
		CSI_ENametagIconPosition nametagPos = CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION);
		
		switch (true)
		{
			case (nametagPos == CSI_ENametagIconPosition.CENTER && (iWidget.GetName() != "RoleIconCenter")) : data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.RIGHT && (iWidget.GetName() != "IconNT")) : data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.LEFT && (iWidget.GetName() != "RoleIconLeft")) : data.SetVisibility(iWidget, false, 0, false); return;
		}
		
		Color colorTeam = stateConf.m_vColor;
		string iconString;
		
		if (data.m_iPlayerID > 0)
		{
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(data.m_iPlayerID);
			
			if (playerData)
			{
				array<int> groupArray = CSI_HUDManager.GetInstance().GetLocalGroupPlayerIds();
				
				if (groupArray.Contains(data.m_iPlayerID))
					colorTeam = CSI_UIHelper.ConvertColorTeamToColor(playerData.GetColorTeam());
				
				iconString = CSI_UIHelper.GetIconString(playerData.GetDisplayIcon(), true);
				
				m_StoredPlayerData = playerData;
				playerData.GetOnDataUpdate().Insert(DataRefresh);
			};
		};
		
		if (!iconString.IsEmpty() && (iconString != "MAN_ICON"))
		{
			iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, iconString);
			iWidget.SetColor(colorTeam);
			data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
		} else
			data.SetVisibility(iWidget, false, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
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
			ImageWidget iWidget = ImageWidget.Cast( m_StoredNameTagData.m_aNametagElements[m_iStoredIndex] );
			if (!iWidget)
				return;
			
			m_StoredNameTagData.SetVisibility(iWidget, false, 0, false);
		}
	}	
}

[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconPlatform
{	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		iWidget.SetVisible(false);
	}
};