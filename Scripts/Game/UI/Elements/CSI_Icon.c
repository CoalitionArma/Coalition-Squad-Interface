class CSI_Icon : SCR_ScriptedWidgetComponent
{
	protected ImageWidget m_wArrow;
	protected ImageWidget m_wIcon;
	
	const static string CSI_ICONS_RESOURCE = "{C3E05464509FCE85}UI/Textures/HUD/Modded/Icons/CSI_ICONS.edds";
	
	protected ref array<CSI_EIcon> m_aNonThemedIcons =
	{
		// ----------- MAN -----------
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,
		// ----------- ICON -----------
		CSI_EIcon.MAN_ICON,
		CSI_EIcon.OFFICER_ICON,
		CSI_EIcon.SL_ICON,
		CSI_EIcon.TL_ICON,
		CSI_EIcon.RTO_ICON,
		// ----------- VEHICLE -----------
		CSI_EIcon.DRIVER,
		CSI_EIcon.PASSANGER,
		CSI_EIcon.COMMANDER,
		CSI_EIcon.GUNNER,
		CSI_EIcon.HELIPILOT,
		CSI_EIcon.HELICREW,
	};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wArrow = ImageWidget.Cast(w.FindAnyWidget("Arrow"));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
	}
	
	//------------------------------------------------------------------------------------------------
	void OnIconUpdate()
	{
		
		
		
		SetArrowWidget()
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetIconWidget(CSI_EIcon icon)
	{
		string iconString;
		
		if (m_aNonThemedIcons.Contains(icon))
			iconString = string.Format("%1", SCR_Enum.GetEnumName(CSI_EIcon, icon));
		else {
			// do settings check here
			CSI_ERoleThemes theme;
			
			iconString = string.Format("%1 %2", SCR_Enum.GetEnumName(CSI_ERoleThemes, theme), SCR_Enum.GetEnumName(CSI_EIcon, icon));
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