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
		
		if (data.m_iPlayerID > 0)
			data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		super.UpdateElement(data, index);

		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget || !data.m_PlayerData)
		{
			if (iWidget)
				iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
			return;
		};
		
		switch (true)
		{
			case (data.m_ePriorityEntityState == ENameTagEntityState.UNCONSCIOUS) 	: SetImageAndSize(iWidget, CSI_UIHelper.VANILLA_ICONS, "unconscious", 48); break;
			case (data.m_ePriorityEntityState == ENameTagEntityState.DEAD) 		: SetImageAndSize(iWidget, CSI_UIHelper.VANILLA_ICONS, "death", 48); break;
			case (data.m_ePriorityEntityState == ENameTagEntityState.VON) 			: SetImageAndSize(iWidget, CSI_UIHelper.VANILLA_ICONS, "VON", 48); break;
			default : {
				CSI_EIcon dislayIcon = data.m_PlayerData.GetDisplayIcon();
				
				if (dislayIcon == CSI_EIcon.MAN)
					dislayIcon = CSI_EIcon.EMPTY;
				
				SetImageAndSize(iWidget, CSI_UIHelper.CSI_ICONS, CSI_UIHelper.GetIconString(dislayIcon, true), 22);
			}
		};
			
		if (CSI_HUDManager.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
		{
			if (data.m_ePriorityEntityState == ENameTagEntityState.VON)
				iWidget.SetColor(CSI_UIHelper.VANILLA_VON_COLOR);
			else
				iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
		};
	}	
	
	protected void SetImageAndSize(ImageWidget iWidget, ResourceName imageset, string image, int size)
	{
		iWidget.LoadImageFromSet(0, imageset, image);
		iWidget.SetSize(size, size);
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