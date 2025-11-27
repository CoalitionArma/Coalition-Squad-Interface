[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
{
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
		
		data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		super.UpdateElement(data, index);

		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		if (data.m_PlayerData)
		{
			CSI_EIcon dislayIcon = data.m_PlayerData.GetDisplayIcon();
			
			if (dislayIcon == CSI_EIcon.MAN)
				dislayIcon = CSI_EIcon.EMPTY;
			
			iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, CSI_UIHelper.GetIconString(dislayIcon, true));
			
			if (CSI_HUDManager.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
		} else
			iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
	}	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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