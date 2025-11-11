[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
{
	protected CSI_PlayerData m_StoredPlayerData;
	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		CSI_ENametagIconPosition nametagPos = CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION);
		
		switch (true)
		{
			case (nametagPos == CSI_ENametagIconPosition.CENTER && (iWidget.GetName() != "RoleIconCenter")) : data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.RIGHT && (iWidget.GetName() != "IconNT")) : data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.LEFT && (iWidget.GetName() != "RoleIconLeft")) : data.SetVisibility(iWidget, false, 0, false); return;
		}
		
		string iconString;
		
		if (data.m_iPlayerID > 0) 
		{	
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(data.m_iPlayerID);
			
			if (playerData)	
			{
				iconString = CSI_UIHelper.GetIconString(playerData.GetDisplayIcon(), true);
				m_StoredPlayerData = playerData;
			} else
				m_StoredPlayerData = null;
		};
		
		if (!iconString.IsEmpty() && (iconString != "MAN_ICON"))
			data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
		else
			data.SetVisibility(iWidget, false, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		super.UpdateElement(data, index);

		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		if (data.m_iPlayerID > 0 && m_StoredPlayerData && m_StoredPlayerData.GetPlayerID() == data.m_iPlayerID)
		{
			iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, CSI_UIHelper.GetIconString(m_StoredPlayerData.GetDisplayIcon(), true));
	
			if (CSI_HUDManager.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(m_StoredPlayerData.GetColorTeam()));
		} else
			iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
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