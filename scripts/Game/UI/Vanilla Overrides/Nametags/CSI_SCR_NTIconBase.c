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
		
		if (!m_bScaleElement)
			FrameSlot.SetSize(iWidget, m_iImageSizeMin, m_iImageSizeMin);
		
		Color colorTeam;
		string iconString;
		
		if (data.m_iPlayerID > 0)
		{
			CSI_PlayerData playerData = CSI_PlayerDataManager.GetInstance().GetPlayerData(data.m_iPlayerID);
			if (playerData)
			{
				colorTeam = CSI_UIHelper.ConvertColorTeamToColor(playerData.GetColorTeam());
				
				iconString = CSI_UIHelper.GetIconString(playerData.GetDisplayIcon());
				iconString = iconString + "_ICON";
			};
			
		};
		
		if (!colorTeam || !colorTeam.IsZero()) 
				colorTeam = stateConf.m_vColor;
		
		if (!m_sImageset.IsEmpty() && !iconString.IsEmpty())
		{
			iWidget.LoadImageFromSet(0, m_sImageset, iconString);
			iWidget.SetColor(colorTeam);
		};
		
		data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions
	}
}