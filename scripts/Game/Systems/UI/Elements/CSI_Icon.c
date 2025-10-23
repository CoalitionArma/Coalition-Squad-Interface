class CSI_Icon : SCR_ScriptedWidgetComponent
{
	protected ImageWidget m_wArrow;
	protected ImageWidget m_wIcon;
	
	const static string CSI_ICONS_RESOURCE = "{C3E05464509FCE85}UI/Textures/HUD/Icons/CSI_ICONS.edds";
	
	protected int m_iPlayerId;
	
	[Attribute("true", UIWidgets.CheckBox)]
	protected bool m_bIconRotationEnabled;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wArrow = ImageWidget.Cast(w.FindAnyWidget("Arrow"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
	}
	
	
	//------------------------------------------------------------------------------------------------
	void IconUpdate(int playerId, int yaw)
	{
		
		
		
		SetArrowWidget()
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetIconWidget(CSI_EIcon icon)
	{
		// do settings check here
		CSI_EIconThemes theme;
		
		//do icon check here
		
		string iconString;
		
		if (CSI_UIHelper.m_aNonThemedIcons.Contains(icon))
			iconString = string.Format("%1", SCR_Enum.GetEnumName(CSI_EIcon, icon));
		else {			
			iconString = string.Format("%1 %2", SCR_Enum.GetEnumName(CSI_EIconThemes, theme), SCR_Enum.GetEnumName(CSI_EIcon, icon));
		};
		
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