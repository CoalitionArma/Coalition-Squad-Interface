//------------------------------------------------------------------------------------------------
//! Nametag zone element base class, not for setup
[BaseContainerProps()]
modded class SCR_NTElementBase
{		
	protected CSI_HUDSystem m_HUDSystem;
	protected CSI_SettingsManager m_SettingsManager;
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldWeClearText(SCR_NameTagData data)
	{
		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone)
			return true;
		
		//-----------------------------------------------------------------------
		// Basic Static Distance Calculations
		int zoneEnd = zone.m_iZoneEnd;
		int zoneStart = zone.GetZoneStart();
		if (zoneStart < 1)
			zoneStart = 1;
		
		//-----------------------------------------------------------------------
		// Get Scale And Cutoff Distance Based On The Percentage The Player Set In Settings
		float scale = (m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED) * 0.01);
		float cutoffDist = scale * zoneEnd;
		cutoffDist = zoneEnd - cutoffDist;// flip it arround
		
		//-----------------------------------------------------------------------
		// Dynamic Distance Calculation For Our Emulation Of Icon Nametags In Vanilla
		float dist = data.m_fDistance;
		
		if (dist > cutoffDist
			&& scale != 0
			&& data.m_eType != ENameTagEntityType.AI
			&& data.m_eType != ENameTagEntityType.VEHICLE
			&& m_SettingsManager.GetSettingBool(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWidgetCSIColor(SCR_NameTagData data, int index)
	{
		Widget widget = data.m_aNametagElements[index];
		if (!widget)
			return;
		
		if (data.m_ePriorityEntityState == ENameTagEntityState.VON)
			widget.SetColor(CSI_UIHelper.VANILLA_VON_COLOR);
		else if (data.m_PlayerData && m_HUDSystem.GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
			widget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
		else
			widget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the scale of text for nametags based on CSI_GameSettings.NAMTEAG_SCALE
	protected int GetNametagTextScale()
	{
		int scale = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_SCALE);
		
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
	
	//------------------------------------------------------------------------------------------------
	//! Get the scale of icons for nametags based on CSI_GameSettings.NAMTEAG_SCALE
	protected int GetNametagImageScale()
	{
		int scale = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_SCALE);
		
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
};