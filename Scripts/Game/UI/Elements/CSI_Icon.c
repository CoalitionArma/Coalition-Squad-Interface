class CSI_Icon : SCR_ScriptedWidgetComponent
{
	protected ImageWidget m_wArrow;
	protected ImageWidget m_wIcon;
	
	const static string CSI_ICONS_RESOURCE = "{C3E05464509FCE85}UI/Textures/HUD/Icons/CSI_ICONS.edds";
	
	protected ref array<CSI_EIcon> m_aNonThemedManIcons =
	{
		// ----------- MAN -----------
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,
	};
	
	protected ref array<CSI_EIcon> m_aNonThemedVehicleIcons =
	{
		// ----------- VEHICLE -----------
		CSI_EIcon.DRIVER,
		CSI_EIcon.PASSANGER,
		CSI_EIcon.COMMANDER,
		CSI_EIcon.GUNNER,
		CSI_EIcon.HELIPILOT,
		CSI_EIcon.HELICREW,
	};
	
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
		CSI_ERoleThemes theme;
		
		//do icon check here
		
		string iconString;
		
		if (m_aNonThemedManIcons.Contains(icon) || m_aNonThemedVehicleIcons.Contains(icon))
			iconString = string.Format("%1", SCR_Enum.GetEnumName(CSI_EIcon, icon));
		else {			
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