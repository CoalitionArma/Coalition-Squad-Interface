[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
{
	protected CSI_HUDSystem m_HUDSystem;
	protected CSI_SettingsManager m_SettingsManager;

	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		m_HUDSystem = CSI_HUDSystem.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();

		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		//-----------------------------------------------------------------------
		// Hide All Icon Widgets Not Being Used
		CSI_ENametagIconPosition nametagPos = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION);
		if (iWidget.GetName() != ("RoleIcon" + SCR_Enum.GetEnumName(CSI_ENametagIconPosition, nametagPos)) || data.m_eType == ENameTagEntityType.AI || !m_SettingsManager.GetSettingBool(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE))
		{
			data.SetVisibility(iWidget, false, 0, false);
			return;
		};
		
		//-----------------------------------------------------------------------
		// Set Static Scale Of The Icon
		int scale = GetNametagImageScale();
		iWidget.SetSize(scale, scale);
		
		//-----------------------------------------------------------------------
		// Actually Make The Widget Visible
		data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		super.UpdateElement(data, index);

		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget || !iWidget.IsVisible())
			return;
		
		if (data.m_PlayerData)
		{
			//-----------------------------------------------------------------------
			// Icon Updates
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
			
			//-----------------------------------------------------------------------
			// Color Updates For VON/Color Teams
			SetWidgetCSIColor(data, index);
		} else {
			iWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
			iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, "EMPTY");
		}
	}	
}