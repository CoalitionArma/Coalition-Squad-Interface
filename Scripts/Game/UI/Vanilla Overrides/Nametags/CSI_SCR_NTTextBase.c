[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{	
	CSI_EName currentName;
	
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
			currentName = CSI_EName.GROUP;
		} else {
			tWidget.SetExactFontSize(GetNametagTextScale());
			currentName = CSI_EName.NAME;
			if (data.m_PlayerData && CSI_HUDSystem.GetInstance() && CSI_HUDSystem.GetInstance().GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
			else
				tWidget.SetColor(stateConf.m_vColor);
		};
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
		
		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone)
			return;
		
		int zoneStart = zone.GetZoneStart();
		if (zoneStart < 1)
			zoneStart = 1;
		
		float dist = Math.InverseLerp(zoneStart, zone.m_iZoneEnd, data.m_fDistance);
		float scale = (CSI_SettingsManager.GetInstance().GetSettingInt(CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED) * 0.01);
		float cutoffDist = scale * zone.m_iZoneEnd;
		cutoffDist = zone.m_iZoneEnd - cutoffDist;// flip it arround
		
		if (dist > cutoffDist && data.m_eType != ENameTagEntityType.AI && data.m_eType != ENameTagEntityType.VEHICLE)
			ChangeNameText(data, index, CSI_EName.EMPTY);
		else {
			if (tWidget.GetName() == "PlayerGroupName")
				ChangeNameText(data, index, CSI_EName.GROUP);
			else
				ChangeNameText(data, index, CSI_EName.NAME);
		};
		
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
	protected void ChangeNameText(SCR_NameTagData data, int index, CSI_EName nameToSet)
	{
		if (currentName != nameToSet)
		{
			string name;
			array<string> nameParams = {};
			data.GetName(name, nameParams);
			
			switch (nameToSet)
			{
				//-----------------------------------------
				case CSI_EName.EMPTY: {	
					Print("EMPTY");
					SetText(data, "EMPTY", nameParams, index);
					break;
				}
				//-----------------------------------------
				case CSI_EName.GROUP: {	
					Print(data.GetGroupName());
					SetText(data, data.GetGroupName(), nameParams, index);
					break;
				}
				//-----------------------------------------
				case CSI_EName.NAME: {
					Print(name);
					SetText(data, name, nameParams, index);
					break;
				}
			}
			
			currentName = nameToSet
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the scale of text for nametags based on CSI_GameSettings.NAMTEAG_SCALE
	protected int GetNametagTextScale()
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