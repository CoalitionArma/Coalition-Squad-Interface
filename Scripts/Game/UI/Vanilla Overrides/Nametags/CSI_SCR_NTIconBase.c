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
		int scale = GetNametagImageScale();
		
		if (iWidget.GetName() != ("RoleIcon" + SCR_Enum.GetEnumName(CSI_ENametagIconPosition, nametagPos)))
		{
			data.SetVisibility(iWidget, false, 0, false);
			return;
		};
		
		iWidget.SetSize(scale, scale);
		
		if (data.m_eType != ENameTagEntityType.AI)
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
	
	//------------------------------------------------------------------------------------------------
	//! Get the scale of icons for nametags based on CSI_GameSettings.NAMTEAG_SCALE
	protected int GetNametagImageScale()
	{
		int scale = CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMTEAG_SCALE);
		
		switch (scale)
		{
			case 0 : return 12;
			case 20 : return 14;
			case 40 : return 16;
			case 60 : return 18;
			case 80 : return 20;
			default : return 22;
		}
		
		return 22;
	}
}