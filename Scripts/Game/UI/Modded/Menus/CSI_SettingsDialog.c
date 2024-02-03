modded enum ChimeraMenuPreset {
	CSI_PlayerSettingsDialog,
	CSI_PlayerSelectionDialog,
	CSI_SettingsDialog
}

class CSI_SettingsDialog : ChimeraMenuBase 
{
	protected Widget m_wRoot;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected CSI_ClientComponent m_ClientComponent;
	
	protected int m_iCompassVisible;
	protected string m_sCompassTexture;
	protected int m_iSquadRadarVisible;
	protected int m_iGroupDisplayVisible;
	protected int m_iStaminaBarVisible;
	protected int m_iNametagsVisible;
	protected int m_iRankVisible;
	protected int m_iSquadRadarIconSize;
	
	protected int m_iCompassVisibleServerOverride;
	protected int m_iSquadRadarVisibleServerOverride;
	protected int m_iGroupDisplayVisibleServerOverride;
	protected int m_iStaminaBarVisibleServerOverride;
	protected int m_iNametagsVisibleServerOverride;
	protected int m_iRankVisibleServerOverride;
	
	protected CheckBoxWidget m_wCompassVisibleWidget;
	protected CheckBoxWidget m_wSquadRadarVisibleWidget;
	protected CheckBoxWidget m_wGroupDisplayVisibleWidget;
	protected CheckBoxWidget m_wStaminaBarVisibleWidget;
	protected CheckBoxWidget m_wNametagsVisibleWidget;
	protected CheckBoxWidget m_wRankVisibleWidget;
	protected XComboBoxWidget m_wIconSizeWidget;
	
	protected CheckBoxWidget m_wCompassVisibleWidgetServerOverride;
	protected CheckBoxWidget m_wSquadRadarVisibleWidgetServerOverride;
	protected CheckBoxWidget m_wGroupDisplayVisibleWidgetServerOverride;
	protected CheckBoxWidget m_wStaminaBarVisibleWidgetServerOverride;
	protected CheckBoxWidget m_wNametagsVisibleWidgetServerOverride;
	protected CheckBoxWidget m_wRankVisibleWidgetServerOverride;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		back.m_OnClicked.Insert(OnMenuBack);
		
		SCR_InputButtonComponent apply = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Apply").FindHandler(SCR_InputButtonComponent));
		apply.m_OnClicked.Insert(ApplySettings);
		
		m_wIconSizeWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconScaleXComboBox"));
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarIconSize", m_iSquadRadarIconSize);
		m_wIconSizeWidget.SetCurrentItem((m_iSquadRadarIconSize/5) - 10);
		 
		// ToDo: Impliment this
		//GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassTexture", m_sCompassTexture);
		
		m_wCompassVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("CompassVisibleCheckBox"));
		m_iCompassVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[0];
		switch (m_iCompassVisibleServerOverride)
		{
			case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassVisible", m_iCompassVisible); m_wCompassVisibleWidget.SetChecked(m_iCompassVisible); break;};
			case (0)  : { m_wCompassVisibleWidget.SetChecked(false); m_wCompassVisibleWidget.SetEnabled(false); break;};
			case (1)  : { m_wCompassVisibleWidget.SetChecked(true);  m_wCompassVisibleWidget.SetEnabled(false); m_wCompassVisibleWidget.SetColorInt(ARGB(255, 125, 125, 125)); break;};
		};
			
		m_wSquadRadarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("SquadRadarVisibleCheckBox"));
		m_iSquadRadarVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[1];
		switch (m_iSquadRadarVisibleServerOverride)
		{
			case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarVisible", m_iSquadRadarVisible); m_wSquadRadarVisibleWidget.SetChecked(m_iSquadRadarVisible); break;};
			case (0)  : { m_wSquadRadarVisibleWidget.SetChecked(false); m_wSquadRadarVisibleWidget.SetEnabled(false); break;};
			case (1)  : { m_wSquadRadarVisibleWidget.SetChecked(true);  m_wSquadRadarVisibleWidget.SetEnabled(false); m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 125, 125, 125)); break;};
		};
		
		m_wGroupDisplayVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("GroupDisplayVisibleCheckBox"));
		m_iGroupDisplayVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[2];
		switch (m_iGroupDisplayVisibleServerOverride)
		{
			case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("groupDisplayVisible", m_iGroupDisplayVisible); m_wGroupDisplayVisibleWidget.SetChecked(m_iGroupDisplayVisible); break;};
			case (0)  : { m_wGroupDisplayVisibleWidget.SetChecked(false); m_wGroupDisplayVisibleWidget.SetEnabled(false); break;};
			case (1)  : { m_wGroupDisplayVisibleWidget.SetChecked(true);  m_wGroupDisplayVisibleWidget.SetEnabled(false); m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 125, 125, 125)); break;};
		};
		
		m_wStaminaBarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("StaminaBarVisibleCheckBox"));
		m_iStaminaBarVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[3];
		switch (m_iStaminaBarVisibleServerOverride)
		{
			case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("staminaBarVisible", m_iStaminaBarVisible); m_wStaminaBarVisibleWidget.SetChecked(m_iStaminaBarVisible); break;};
			case (0)  : { m_wStaminaBarVisibleWidget.SetChecked(false); m_wStaminaBarVisibleWidget.SetEnabled(false); break;};
			case (1)  : { m_wStaminaBarVisibleWidget.SetChecked(true);  m_wStaminaBarVisibleWidget.SetEnabled(false); m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 125, 125, 125)); break;};
		};
		
		m_wNametagsVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsVisibleCheckbox"));
		m_iNametagsVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[4];
		switch (m_iNametagsVisibleServerOverride)
		{
			case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsVisible", m_iNametagsVisible); m_wNametagsVisibleWidget.SetChecked(m_iNametagsVisible); break;};
			case (0)  : { m_wNametagsVisibleWidget.SetChecked(false); m_wNametagsVisibleWidget.SetEnabled(false); break;};
			case (1)  : { m_wNametagsVisibleWidget.SetChecked(true);  m_wNametagsVisibleWidget.SetEnabled(false); m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 125, 125, 125)); break;};
		};
		
		m_wRankVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("RankVisibleCheckBox"));
		m_iRankVisibleServerOverride = m_AuthorityComponent.ReturnAuthoritySettings()[5];
		switch (m_iRankVisibleServerOverride)
		{
			case (-1) : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("rankVisible", m_iRankVisible); m_wRankVisibleWidget.SetChecked(m_iRankVisible); break;};
			case (0)  : { m_wRankVisibleWidget.SetChecked(false); m_wRankVisibleWidget.SetEnabled(false); break;};
			case (1)  : { m_wRankVisibleWidget.SetChecked(true);  m_wRankVisibleWidget.SetEnabled(false); m_wRankVisibleWidget.SetColorInt(ARGB(255, 125, 125, 125)); break;};
		};
		
		if (SCR_Global.IsAdmin()) ShowServerOverrideMenu();
	}
	
	//------------------------------------------------------------------------------------------------

	// Additional menu functions

	//------------------------------------------------------------------------------------------------
	
	protected void ShowServerOverrideMenu() 
	{
		m_wCompassVisibleWidgetServerOverride      = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("compassVisibleServerOverride"));
		m_wSquadRadarVisibleWidgetServerOverride   = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("squadRadarVisibleServerOverride"));
		m_wGroupDisplayVisibleWidgetServerOverride = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("groupDisplayVisibleServerOverride"));
		m_wStaminaBarVisibleWidgetServerOverride   = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("staminaBarVisibleServerOverride"));
		m_wNametagsVisibleWidgetServerOverride     = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("nametagsVisibleServerOverride"));
		m_wRankVisibleWidgetServerOverride         = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("showRankServerOverride"));
		
		ImageWidget backgroundServerOverride       = ImageWidget.Cast(m_wRoot.FindAnyWidget("BackgroundServerOverride"));
		ImageWidget prettyServerOverride           = ImageWidget.Cast(m_wRoot.FindAnyWidget("PrettyServerOverride"));
		
		TextWidget prettyTextServerOverride        = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride"));
		TextWidget prettyTextServerOverrideTwo     = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverrideTwo"));
		TextWidget seperatorServerOverride         = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride"));
		
		m_wCompassVisibleWidgetServerOverride.SetOpacity(1);
		m_wSquadRadarVisibleWidgetServerOverride.SetOpacity(1);
		m_wGroupDisplayVisibleWidgetServerOverride.SetOpacity(1);
		m_wStaminaBarVisibleWidgetServerOverride.SetOpacity(1);
		m_wNametagsVisibleWidgetServerOverride.SetOpacity(1);
		m_wRankVisibleWidgetServerOverride.SetOpacity(1);
		backgroundServerOverride.SetOpacity(1);
		prettyServerOverride.SetOpacity(1);
		prettyTextServerOverride.SetOpacity(1);
		prettyTextServerOverrideTwo.SetOpacity(1);
		seperatorServerOverride.SetOpacity(1);
	
		if (m_iCompassVisibleServerOverride != -1) m_wCompassVisibleWidgetServerOverride.SetChecked(true);
		if (m_iSquadRadarVisibleServerOverride != -1) m_wSquadRadarVisibleWidgetServerOverride.SetChecked(true);
		if (m_iGroupDisplayVisibleServerOverride != -1) m_wGroupDisplayVisibleWidgetServerOverride.SetChecked(true);
		if (m_iStaminaBarVisibleServerOverride != -1) m_wStaminaBarVisibleWidgetServerOverride.SetChecked(true);
		if (m_iNametagsVisibleServerOverride != -1) m_wNametagsVisibleWidgetServerOverride.SetChecked(true);
		if (m_iRankVisibleServerOverride != -1) m_wRankVisibleWidgetServerOverride.SetChecked(true);
	};
	
	protected void ApplySettings()
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("compassVisible",      m_wCompassVisibleWidget.IsChecked());
	//	GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("compassTexture",   	  "{D19C93F5109F3E1D}UI\Textures\HUD\Modded\compass_shadow360.edds");
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("squadRadarVisible",   m_wSquadRadarVisibleWidget.IsChecked());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("groupDisplayVisible", m_wGroupDisplayVisibleWidget.IsChecked());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("staminaBarVisible",   m_wStaminaBarVisibleWidget.IsChecked());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("nametagsVisible",     m_wNametagsVisibleWidget.IsChecked());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("rankVisible",         m_wRankVisibleWidget.IsChecked());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("squadRadarIconSize",  ((m_wIconSizeWidget.GetCurrentItem() * 5) + 50));
		
		if (SCR_Global.IsAdmin()) ApplyAdminSettings();
		
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
		
	protected void ApplyAdminSettings()
	{
		m_ClientComponent = CSI_ClientComponent.GetInstance();
		if (!m_ClientComponent) return;
		
		if (m_wCompassVisibleWidgetServerOverride.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("compassVisibleServerOverride", m_wCompassVisibleWidget.IsChecked()); m_wCompassVisibleWidget.SetEnabled(false);		
		} else {m_ClientComponent.Owner_ChangeAuthoritySetting("compassVisibleServerOverride", -1); m_wCompassVisibleWidget.SetEnabled(true); };
		
		if (m_wSquadRadarVisibleWidgetServerOverride.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("squadRadarVisibleServerOverride", m_wSquadRadarVisibleWidget.IsChecked()); m_wSquadRadarVisibleWidget.SetEnabled(false);			
		} else {m_ClientComponent.Owner_ChangeAuthoritySetting("squadRadarVisibleServerOverride", -1); m_wSquadRadarVisibleWidget.SetEnabled(true);};
		
		if (m_wGroupDisplayVisibleWidgetServerOverride.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("groupDisplayVisibleServerOverride", m_wGroupDisplayVisibleWidget.IsChecked()); m_wGroupDisplayVisibleWidget.SetEnabled(false);			
		} else {m_ClientComponent.Owner_ChangeAuthoritySetting("groupDisplayVisibleServerOverride", -1); m_wGroupDisplayVisibleWidget.SetEnabled(true);};
		
		if (m_wStaminaBarVisibleWidgetServerOverride.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("staminaBarVisibleServerOverride", m_wStaminaBarVisibleWidget.IsChecked())	; m_wStaminaBarVisibleWidget.SetEnabled(false);		
		} else {m_ClientComponent.Owner_ChangeAuthoritySetting("staminaBarVisibleServerOverride", -1); m_wStaminaBarVisibleWidget.SetEnabled(true);};
		
		if (m_wNametagsVisibleWidgetServerOverride.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsVisibleServerOverride", m_wNametagsVisibleWidget.IsChecked()); m_wNametagsVisibleWidget.SetEnabled(false);			
		} else {m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsVisibleServerOverride", -1); m_wNametagsVisibleWidget.SetEnabled(true);};
		
		if (m_wRankVisibleWidgetServerOverride.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("rankVisibleServerOverride", m_wRankVisibleWidget.IsChecked()); m_wRankVisibleWidget.SetEnabled(false);		
		} else {m_ClientComponent.Owner_ChangeAuthoritySetting("rankVisibleServerOverride", -1); m_wRankVisibleWidget.SetEnabled(true);};
	}
	
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseAllMenus();
	}
}