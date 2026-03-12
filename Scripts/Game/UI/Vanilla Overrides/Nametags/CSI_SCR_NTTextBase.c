[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		tWidget.SetFont(m_FontResource);
		
		if (tWidget.GetName() == "PlayerGroupName")
		{
			tWidget.SetText(data.GetGroupName());
			tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE)); 
			tWidget.SetExactFontSize(GetNametagTextScale() - 2);
		} else {
			tWidget.SetExactFontSize(GetNametagTextScale());
			if (data.m_PlayerData && CSI_HUDSystem.GetInstance() && CSI_HUDSystem.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
			else
				tWidget.SetColor(stateConf.m_vColor);
		};

		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions		
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		super.UpdateElement(data, index);
		
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		int dist = ((data.m_fDistance / 2) / 10);
		float scale = (CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED) * 0.01);
		int cutoffDist = scale * CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMETAG_RANGE);
		
		if (dist > cutoffDist 
			&& data.m_eType != ENameTagEntityType.AI 
			&& data.m_eType != ENameTagEntityType.VEHICLE)
			data.SetVisibility(tWidget, false, 0, true);
		else
			data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault); // transitions
		
		if (tWidget.GetName() == "PlayerName")
		{
			if (data.m_ePriorityEntityState == ENameTagEntityState.VON)
				tWidget.SetColor(CSI_UIHelper.VANILLA_VON_COLOR);
			else if (data.m_PlayerData && CSI_HUDSystem.GetInstance() && CSI_HUDSystem.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
			else
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE)); 
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the scale of text for nametags based on CSI_GameSettings.NAMTEAG_SCALE
	static int GetNametagTextScale()
	{
		int scale = CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMTEAG_SCALE);
		
		switch (scale)
		{
			case 0 : return 8;
			case 20 : return 9;
			case 40 : return 10;
			case 60 : return 11;
			case 80 : return 12;
			default : return 13;
		}
		
		return 13;
	}
}