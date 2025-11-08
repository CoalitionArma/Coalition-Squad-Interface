[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
{
	protected SCR_NameTagData m_StoredNameTagData;
	protected int m_iStoredIndex;
	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget || iWidget.GetName() != "IconNT")
			return;
		
		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Insert(DataRefresh);
		m_StoredNameTagData = data;
		m_iStoredIndex = index;
		
		CSI_ENametagIconPosition nametagPos = CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION);
		Widget parent = iWidget.GetParent().GetParent();
		
		ImageWidget rightWidget = iWidget;
		ImageWidget leftWidget = ImageWidget.Cast(parent.FindAnyWidget("RoleIconLeft"));
		ImageWidget centerWidget = ImageWidget.Cast(parent.FindAnyWidget("RoleIconCenter"));
		
		ImageWidget widgetToEdit;
		
		switch (nametagPos)
		{
			case CSI_ENametagIconPosition.CENTER : {
				data.SetVisibility(rightWidget, false, 0, false);
				data.SetVisibility(leftWidget, false, 0, false);
				widgetToEdit = centerWidget; 
				break;
			}
			case CSI_ENametagIconPosition.RIGHT : {
				data.SetVisibility(centerWidget, false, 0, false);
				data.SetVisibility(leftWidget, false, 0, false);
				widgetToEdit = rightWidget; 
				break;
			}
			default : {
				data.SetVisibility(centerWidget, false, 0, false);
				data.SetVisibility(rightWidget, false, 0, false);
				widgetToEdit = leftWidget; 
			}
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
				playerData.GetOnDataUpdate().Insert(DataRefresh);
			};
		};
		
		if (!iconString.IsEmpty() && (iconString != "MAN_ICON"))
		{
			widgetToEdit.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, iconString);
			widgetToEdit.SetColor(colorTeam);
			data.SetVisibility(widgetToEdit, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
		} else
			data.SetVisibility(widgetToEdit, false, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}

	protected void DataRefresh()
	{	
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Remove(DataRefresh);
		
		if (m_StoredNameTagData.m_iPlayerID > 0)
		{
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(m_StoredNameTagData.m_iPlayerID);
			
			if (playerData)
				playerData.GetOnDataUpdate().Remove(DataRefresh);
			
			SetDefaults(m_StoredNameTagData, m_iStoredIndex);
		};
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