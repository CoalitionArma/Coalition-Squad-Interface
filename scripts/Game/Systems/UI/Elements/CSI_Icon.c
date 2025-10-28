class CSI_Icon : SCR_ScriptedWidgetComponent
{
	protected ImageWidget m_wArrow;
	protected ImageWidget m_wIcon;
	protected ImageWidget m_wOutline;
	
	const static string CSI_ICONS_RESOURCE = "{C3E05464509FCE85}UI/Textures/HUD/Icons/CSI_ICONS.edds";
	
	protected CSI_AuthorityManager m_AuthorityManager;
	protected CSI_PlayerData m_PlayerData;
	protected int m_iPlayerId;
	
	[Attribute("false", UIWidgets.CheckBox)]
	protected bool m_bIsSimpleIcon;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_AuthorityManager = CSI_AuthorityManager.GetInstance();
		
		m_wArrow = ImageWidget.Cast(w.FindAnyWidget("Arrow"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wOutline = ImageWidget.Cast(w.FindAnyWidget("Outline"));
	}
	
	//------------------------------------------------------------------------------------------------
	void IconUpdate(int playerId)
	{
		if (m_iPlayerId != playerId)
		{
			m_iPlayerId = playerId;
			
			if (m_PlayerData)
				m_PlayerData.GetOnDataUpdate().Remove(DataUpdate);
			
			if (playerId == 0)
			{
				m_wRoot.SetVisible(false);
				return;
			};
			
			m_wRoot.SetVisible(true);
			m_PlayerData = m_AuthorityManager.GetPlayerData(playerId);
			m_PlayerData.GetOnDataUpdate().Insert(DataUpdate);
			
			DataUpdate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRotation(float yaw)
	{
		m_wArrow.SetRotation(yaw);
		m_wIcon.SetRotation(yaw);
		m_wOutline.SetRotation(yaw);
	};
	
	//------------------------------------------------------------------------------------------------
	protected void DataUpdate()
	{	
		if (!m_PlayerData)
			return;
		
		SetIconWidget(m_PlayerData.GetDisplayIcon());
		SetArrowWidget();
		
		Color colorTeam = CSI_UIHelper.ConvertColorTeamToColor(m_PlayerData.GetColorTeam());
		
		m_wArrow.SetColor(colorTeam); 
		m_wIcon.SetColor(colorTeam);
		m_wOutline.SetColor(colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetIconWidget(CSI_EIcon icon)
	{
		// do settings check here
		CSI_EIconThemes theme;
		
		string iconString;
		
		if (CSI_UIHelper.m_aNonThemedIcons.Contains(icon))
			iconString = string.Format("%1", SCR_Enum.GetEnumName(CSI_EIcon, icon));
		else	
			iconString = string.Format("%1 %2", SCR_Enum.GetEnumName(CSI_EIconThemes, theme), SCR_Enum.GetEnumName(CSI_EIcon, icon));
		
		m_wIcon.LoadImageFromSet(0, CSI_ICONS_RESOURCE, iconString);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetArrowWidget()
	{
		// do settings check here
		CSI_EArrow arrow;
		
		string arrowString = string.Format("ARROW %1", SCR_Enum.GetEnumName(CSI_EArrow, arrow));
		
		m_wArrow.LoadImageFromSet(0, CSI_ICONS_RESOURCE, arrowString);
	}
}