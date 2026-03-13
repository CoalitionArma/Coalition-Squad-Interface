[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTTextBase : SCR_NTElementBase
{	
	protected float m_fScale;
	protected float m_fCutoffDist;
	protected int m_iZoneStart;
	protected int m_iZoneEnd;

	protected CSI_HUDSystem m_HUDSystem;
	protected CSI_SettingsManager m_SettingsManager;

	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		if (!m_HUDSystem || !m_SettingsManager)
		{
			m_HUDSystem = CSI_HUDSystem.GetInstance();
			m_SettingsManager = CSI_SettingsManager.GetInstance();
		};
		
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;

		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone)
			return;
		
		//-----------------------------------------------------------------------
		// Initial Appearence And Settings Of The Text Widget
		tWidget.SetFont(m_FontResource);
		
		if (tWidget.GetName() == "PlayerGroupName")
		{
			tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE)); 
			tWidget.SetExactFontSize(GetNametagTextScale() - 2);
		} else {
			tWidget.SetExactFontSize(GetNametagTextScale());
			if (data.m_PlayerData && m_HUDSystem.GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
			else
				tWidget.SetColor(stateConf.m_vColor);
		};

		//-----------------------------------------------------------------------
		// Basic Static Distance Calculations, Done Only On Init Of The Nametag For Performance
		int m_iZoneStart = zone.GetZoneStart();
		if (m_iZoneStart < 1)
			m_iZoneStart = 1;

		m_iZoneEnd = zone.m_iZoneEnd;
		float m_fScale = (m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED) * 0.01);
		float m_fCutoffDist = m_fScale * m_iZoneEnd;
		m_fCutoffDist = m_iZoneEnd - m_fCutoffDist;// flip it arround
		
		//-----------------------------------------------------------------------
		// Actually Make The Widget Visible
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions		
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{	
		super.UpdateElement(data, index);
		
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;

		//-----------------------------------------------------------------------
		// Distance Calculations For Our Emulation Of Icon Nametags In Vanilla
		float dist = Math.InverseLerp(m_iZoneStart, m_iZoneEnd, data.m_fDistance);
		
		if (dist > m_fCutoffDist
			&& m_fScale != 0
			&& data.m_eType != ENameTagEntityType.AI 
			&& data.m_eType != ENameTagEntityType.VEHICLE 
			&& m_SettingsManager.GetSettingBool(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE))
			SetText(data, "", data.m_aNameParams, index);
		else {
			if (tWidget.GetName() == "PlayerGroupName")
				SetText(data, data.m_sGroupName, data.m_aNameParams, index);
			else
				SetText(data, data.m_sName, data.m_aNameParams, index);
		};

		//-----------------------------------------------------------------------
		// Color Updates For VON/Color Teams
		if (tWidget.GetName() == "PlayerName")
		{
			if (data.m_ePriorityEntityState == ENameTagEntityState.VON)
				tWidget.SetColor(CSI_UIHelper.VANILLA_VON_COLOR);
			else if (data.m_PlayerData && m_HUDSystem.GetLocalGroupPlayerIds().Contains(data.m_iPlayerID))
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(data.m_PlayerData.GetColorTeam()));
			else
				tWidget.SetColor(CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE));
		}
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
}