[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
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
		if (!m_HUDManager || !m_SettingsManager || !m_PlayerDataManager)
		{
			m_HUDManager = CSI_HUDManager.GetInstance();
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		};
		
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		CSI_ENametagIconPosition nametagPos = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION);
		
		switch (true)
		{
			case (nametagPos == CSI_ENametagIconPosition.CENTER && (iWidget.GetName() != "RoleIconCenter")) : data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.RIGHT && (iWidget.GetName() != "IconNT")) : data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.LEFT && (iWidget.GetName() != "RoleIconLeft")) : data.SetVisibility(iWidget, false, 0, false); return;
		}
		
		string iconString;
		
		if (data.m_iPlayerID > 0)
		{
			CSI_PlayerData playerData = m_PlayerDataManager.GetPlayerData(data.m_iPlayerID);
			
			if (playerData)
			{
				iconString = CSI_UIHelper.GetIconString(playerData.GetDisplayIcon(), true);
				m_StoredPlayerData = playerData;
				m_StoredNameTagData = data;
				m_iStoredIndex = index;
				UpdateIcon();
				m_SettingsManager.GetOnSettingsUpdate().Insert(UpdateIcon);
				playerData.GetOnDataUpdate().Insert(UpdateIcon);
			};
		};
		
		if (!iconString.IsEmpty() && (iconString != "MAN_ICON"))
			data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
		else
			data.SetVisibility(iWidget, false, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}

	protected void UpdateIcon()
	{	
		if (!m_StoredNameTagData 
			|| !m_StoredPlayerData 
			|| m_StoredNameTagData.m_iPlayerID <= 0 
			|| (m_StoredPlayerData.GetPlayerID() != m_StoredNameTagData.m_iPlayerID))
		{
			m_SettingsManager.GetOnSettingsUpdate().Remove(UpdateIcon);

			if (m_StoredPlayerData)
				m_StoredPlayerData.GetOnDataUpdate().Remove(UpdateIcon);
			return;
		};

		ImageWidget iWidget = ImageWidget.Cast( m_StoredNameTagData.m_aNametagElements[m_iStoredIndex] );
		if (!iWidget)
			return;

		Color colorTeam = CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE);

		if (m_HUDManager.GetLocalGroupPlayerIds().Contains(m_StoredNameTagData.m_iPlayerID))
			colorTeam = CSI_UIHelper.ConvertColorTeamToColor(m_StoredPlayerData.GetColorTeam());

		iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, CSI_UIHelper.GetIconString(m_StoredPlayerData.GetDisplayIcon(), true));
		iWidget.SetColor(colorTeam);
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