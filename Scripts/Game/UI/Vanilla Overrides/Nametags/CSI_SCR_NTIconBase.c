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
			case (nametagPos == CSI_ENametagIconPosition.CENTER && (iWidget.GetName() != "RoleIconCenter")) 	: data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.RIGHT && (iWidget.GetName() != "RoleIconRight")) 	: data.SetVisibility(iWidget, false, 0, false); return;
			case (nametagPos == CSI_ENametagIconPosition.LEFT && (iWidget.GetName() != "RoleIconLeft")) 		: data.SetVisibility(iWidget, false, 0, false); return;
		}
		
		int scale = CSI_UIHelper.GetNametagImageScale();
		iWidget.SetSize(scale, scale);
		
		if (data.m_iPlayerID > 0)
			data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
		else
			data.SetVisibility(iWidget, false, 0, false);
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
			switch (true)
			{
				case (data.m_ePriorityEntityState == ENameTagEntityState.UNCONSCIOUS)	: iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "unconscious"); break;
				case (data.m_ePriorityEntityState == ENameTagEntityState.DEAD) 			: iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "death"); break;
				case (data.m_ePriorityEntityState == ENameTagEntityState.VON) 			: iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "VON"); break;
				default : {
					bool setAlt = false; 
					CSI_EIcon displayIcon = data.m_PlayerData.GetDisplayIcon();
					
					if (displayIcon == CSI_EIcon.DRIVER)
						setAlt = true;
					
					if (displayIcon == CSI_EIcon.MAN)
						iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
					else
						iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, CSI_UIHelper.GetIconString(displayIcon, true, setAlt));
				}
			};
			
			Color colorToSet;
				
			if (data.m_ePriorityEntityState == ENameTagEntityState.VON)
				colorToSet = CSI_UIHelper.VANILLA_VON_COLOR;
			else if (CSI_HUDSystem.GetInstance() && CSI_HUDSystem.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				colorToSet = CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam());
			else
				colorToSet = CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE); 
			
			iWidget.SetColor(colorToSet);
		} else {
			iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
			iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, "EMPTY");
		}
	}	
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconPlatform
{	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		data.SetVisibility(iWidget, false, 0, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		if (!data.m_aNametagElements[index])
			return;
		
		if (data.m_eType != ENameTagEntityType.PLAYER && data.m_eType != ENameTagEntityType.VEHICLE)
			return;
		
		ImageWidget image = ImageWidget.Cast(data.m_aNametagElements[index]);
		if (!image)
			return;
		
		data.SetVisibility(image, false, 0, false);
	};
};