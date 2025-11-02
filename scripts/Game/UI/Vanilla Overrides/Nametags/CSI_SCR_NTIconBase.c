[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
{
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget || iWidget.GetName() != "IconNT")
			return;
		
		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		CSI_ENametagIconPosition nametagPos = CSI_SettingsManager.GetInstance().GetCSISettingInt(CSI_SettingsManager.NAMETAG_ROLE_ICON_POSITION);
		Widget parent = iWidget.GetParent().GetParent();
		
		ImageWidget leftWidget = ImageWidget.Cast(parent.FindAnyWidget("RoleIconLeft"));
		ImageWidget centerWidget = ImageWidget.Cast(parent.FindAnyWidget("RoleIconCenter"));
		
		switch (nametagPos)
		{
			case CSI_ENametagIconPosition.LEFT : iWidget = leftWidget; break;
			case CSI_ENametagIconPosition.CENTER : iWidget = centerWidget; break;
		}
		
		if (!iWidget)
			return;
		
		Color colorTeam;
		string iconString;
		
		if (data.m_iPlayerID > 0)
		{
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(data.m_iPlayerID);
			if (playerData)
			{
				colorTeam = CSI_UIHelper.ConvertColorTeamToColor(playerData.GetColorTeam());
				iconString = CSI_UIHelper.GetIconString(playerData.GetDisplayIcon(), true);
			};
		};
		
		if (!colorTeam || !colorTeam.IsZero()) 
				colorTeam = stateConf.m_vColor;
		
		if (!iconString.IsEmpty())
		{
			iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS_RESOURCE, iconString);
			iWidget.SetColor(colorTeam);
			data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
		} else
			data.SetVisibility(iWidget, false, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}

}

[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconPlatform
{	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		CSI_ENametagIconPosition nametagPos = CSI_SettingsManager.GetInstance().GetCSISettingInt(CSI_SettingsManager.NAMETAG_ROLE_ICON_POSITION);
		if (nametagPos == CSI_ENametagIconPosition.LEFT)
		{
			ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
			if (!iWidget)
				return;
			
			iWidget.SetVisible(false);
		} else
			super.SetDefaults(data, index);
	}
};