modded enum ChimeraMenuPreset {
	CSI_PlayerSettingsDialog,
	CSI_PlayerSelectionDialog,
	CSI_SettingsDialog
}

class CSI_SettingsDialog : ChimeraMenuBase 
{
	protected Widget m_wRoot;
	
	protected string m_sCompassVisibleSO;
	protected string m_sSquadRadarVisibleSO;
	protected string m_sGroupDisplayVisibleSO;
	protected string m_sStaminaBarVisibleSO;
	protected string m_sNametagsVisibleSO;
	protected string m_sRankVisibleSO;
	
	protected CheckBoxWidget m_wCompassVisibleWidget;
	protected CheckBoxWidget m_wSquadRadarVisibleWidget;
	protected CheckBoxWidget m_wGroupDisplayVisibleWidget;
	protected CheckBoxWidget m_wStaminaBarVisibleWidget;
	protected CheckBoxWidget m_wNametagsVisibleWidget;
	protected CheckBoxWidget m_wRankVisibleWidget;
	protected XComboBoxWidget m_wIconSizeWidget;
	
	protected CheckBoxWidget m_wCompassVisibleWidgetSO;
	protected CheckBoxWidget m_wSquadRadarVisibleWidgetSO;
	protected CheckBoxWidget m_wGroupDisplayVisibleWidgetSO;
	protected CheckBoxWidget m_wStaminaBarVisibleWidgetSO;
	protected CheckBoxWidget m_wNametagsVisibleWidgetSO;
	protected CheckBoxWidget m_wRankVisibleWidgetSO;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();
		CSI_AuthorityComponent authorityComponent = CSI_AuthorityComponent.GetInstance();
		if (!authorityComponent) return;
		
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent back = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Back").FindHandler(SCR_InputButtonComponent));
		back.m_OnClicked.Insert(OnMenuBack);
		
		SCR_InputButtonComponent apply = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Apply").FindHandler(SCR_InputButtonComponent));
		apply.m_OnClicked.Insert(ApplySettings);
		
		float m_fSquadRadarIconSize = 100;
		m_wIconSizeWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconScaleXComboBox"));
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarIconSize", m_fSquadRadarIconSize);
		if (m_fSquadRadarIconSize == 0) m_fSquadRadarIconSize = 100; 
		m_wIconSizeWidget.SetCurrentItem((m_fSquadRadarIconSize/5) - 10);
		 
		// ToDo: Impliment this
		//GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassTexture", m_sCompassTexture);
		
		m_wCompassVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("CompassVisibleCheckBox"));
		m_sCompassVisibleSO = authorityComponent.ReturnAuthoritySettings()[0];
		string m_sCompassVisible = "";
		switch (m_sCompassVisibleSO)
		{
			case ("false") : { m_wCompassVisibleWidget.SetChecked(false); m_wCompassVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wCompassVisibleWidget.SetChecked(true);  m_wCompassVisibleWidget.SetEnabled(false); m_wCompassVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassVisible", m_sCompassVisible); 
				// default state
				m_wCompassVisibleWidget.SetChecked(true);
				
				if (m_sCompassVisible == "false") {
					m_wCompassVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
			
		m_wSquadRadarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("SquadRadarVisibleCheckBox"));
		m_sSquadRadarVisibleSO = authorityComponent.ReturnAuthoritySettings()[1];
		string m_sSquadRadarVisible = "";
		switch (m_sSquadRadarVisibleSO)
		{
			case ("false") : { m_wSquadRadarVisibleWidget.SetChecked(false); m_wSquadRadarVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wSquadRadarVisibleWidget.SetChecked(true);  m_wSquadRadarVisibleWidget.SetEnabled(false); m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarVisible", m_sSquadRadarVisible); 
				// default state
				m_wSquadRadarVisibleWidget.SetChecked(true);
				
				if (m_sSquadRadarVisible == "false") {
					m_wSquadRadarVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wGroupDisplayVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("GroupDisplayVisibleCheckBox"));
		m_sGroupDisplayVisibleSO = authorityComponent.ReturnAuthoritySettings()[2];
		string m_sGroupDisplayVisible = "";
		switch (m_sGroupDisplayVisibleSO)
		{
			case ("false") : { m_wGroupDisplayVisibleWidget.SetChecked(false); m_wGroupDisplayVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wGroupDisplayVisibleWidget.SetChecked(true);  m_wGroupDisplayVisibleWidget.SetEnabled(false); m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("groupDisplayVisible", m_sGroupDisplayVisible); 
				// default state
				m_wGroupDisplayVisibleWidget.SetChecked(true);
				
				if (m_sGroupDisplayVisible == "false") {
					m_wGroupDisplayVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wStaminaBarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("StaminaBarVisibleCheckBox"));
		m_sStaminaBarVisibleSO = authorityComponent.ReturnAuthoritySettings()[3];
		string m_sStaminaBarVisible = "";
		switch (m_sStaminaBarVisibleSO)
		{
			case ("false") : { m_wStaminaBarVisibleWidget.SetChecked(false); m_wStaminaBarVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wStaminaBarVisibleWidget.SetChecked(true);  m_wStaminaBarVisibleWidget.SetEnabled(false); m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("staminaBarVisible", m_sStaminaBarVisible); 
				// default state
				m_wStaminaBarVisibleWidget.SetChecked(true);
				
				if (m_sStaminaBarVisible == "false") {
					m_wStaminaBarVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wNametagsVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsVisibleCheckbox"));
		m_sNametagsVisibleSO = authorityComponent.ReturnAuthoritySettings()[4];
		string m_sNametagsVisible = "";
		switch (m_sNametagsVisibleSO)
		{
			case ("false") : { m_wNametagsVisibleWidget.SetChecked(false); m_wNametagsVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wNametagsVisibleWidget.SetChecked(true);  m_wNametagsVisibleWidget.SetEnabled(false); m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsVisible", m_sNametagsVisible); 
				// default state
				m_wNametagsVisibleWidget.SetChecked(true);
				
				if (m_sNametagsVisible == "false") {
					m_wNametagsVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wRankVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("RankVisibleCheckBox"));
		m_sRankVisibleSO = authorityComponent.ReturnAuthoritySettings()[5];
		string m_sRankVisible = "";
		switch (m_sRankVisibleSO)
		{
			case ("false") : { m_wRankVisibleWidget.SetChecked(false); m_wRankVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wRankVisibleWidget.SetChecked(true);  m_wRankVisibleWidget.SetEnabled(false); m_wRankVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("rankVisible", m_sRankVisible); 
				// default state
				m_wRankVisibleWidget.SetChecked(false);
				
				if (m_sRankVisible == "true") {
					m_wRankVisibleWidget.SetChecked(true);
				};
				break;
			};
		};
		
		if (SCR_Global.IsAdmin()) ShowServerOverrideMenu();
	}
	
	//------------------------------------------------------------------------------------------------

	// Additional menu functions

	//------------------------------------------------------------------------------------------------
	
	protected void ShowServerOverrideMenu() 
	{
		m_wCompassVisibleWidgetSO      = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("compassVisibleServerOverride"));
		m_wSquadRadarVisibleWidgetSO   = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("squadRadarVisibleServerOverride"));
		m_wGroupDisplayVisibleWidgetSO = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("groupDisplayVisibleServerOverride"));
		m_wStaminaBarVisibleWidgetSO   = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("staminaBarVisibleServerOverride"));
		m_wNametagsVisibleWidgetSO     = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("nametagsVisibleServerOverride"));
		m_wRankVisibleWidgetSO         = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("showRankServerOverride"));
		
		ImageWidget backgroundServerOverride       = ImageWidget.Cast(m_wRoot.FindAnyWidget("BackgroundServerOverride"));
		ImageWidget prettyServerOverride           = ImageWidget.Cast(m_wRoot.FindAnyWidget("PrettyServerOverride"));
		
		TextWidget prettyTextServerOverride        = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride"));
		TextWidget prettyTextServerOverrideTwo     = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverrideTwo"));
		TextWidget seperatorServerOverride         = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride"));
		
		m_wCompassVisibleWidgetSO.SetOpacity(1);
		m_wSquadRadarVisibleWidgetSO.SetOpacity(1);
		m_wGroupDisplayVisibleWidgetSO.SetOpacity(1);
		m_wStaminaBarVisibleWidgetSO.SetOpacity(1);
		m_wNametagsVisibleWidgetSO.SetOpacity(1);
		m_wRankVisibleWidgetSO.SetOpacity(1);
		backgroundServerOverride.SetOpacity(1);
		prettyServerOverride.SetOpacity(1);
		prettyTextServerOverride.SetOpacity(1);
		prettyTextServerOverrideTwo.SetOpacity(1);
		seperatorServerOverride.SetOpacity(1);
	
		if (m_sCompassVisibleSO == "true" || m_sCompassVisibleSO == "false") m_wCompassVisibleWidgetSO.SetChecked(true);
		if (m_sSquadRadarVisibleSO == "true" || m_sSquadRadarVisibleSO == "false") m_wSquadRadarVisibleWidgetSO.SetChecked(true);
		if (m_sGroupDisplayVisibleSO == "true" || m_sGroupDisplayVisibleSO == "false") m_wGroupDisplayVisibleWidgetSO.SetChecked(true);
		if (m_sStaminaBarVisibleSO == "true" || m_sStaminaBarVisibleSO == "false") m_wStaminaBarVisibleWidgetSO.SetChecked(true);
		if (m_sNametagsVisibleSO == "true" || m_sNametagsVisibleSO == "false") m_wNametagsVisibleWidgetSO.SetChecked(true);
		if (m_sRankVisibleSO == "true" || m_sRankVisibleSO == "false") m_wRankVisibleWidgetSO.SetChecked(true);
	};
	
	protected void ApplySettings()
	{		
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("compassVisible",      m_wCompassVisibleWidget.IsChecked().ToString());
	//	GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("compassTexture",   	  "{D19C93F5109F3E1D}UI\Textures\HUD\Modded\Compasses\compass_shadow360.edds");
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("squadRadarVisible",   m_wSquadRadarVisibleWidget.IsChecked().ToString());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("groupDisplayVisible", m_wGroupDisplayVisibleWidget.IsChecked().ToString());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("staminaBarVisible",   m_wStaminaBarVisibleWidget.IsChecked().ToString());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("nametagsVisible",     m_wNametagsVisibleWidget.IsChecked().ToString());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("rankVisible",         m_wRankVisibleWidget.IsChecked().ToString());
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("squadRadarIconSize",  (m_wIconSizeWidget.GetCurrentItem() * 5) + 50);
		
		if (SCR_Global.IsAdmin()) ApplyAdminSettings();
		
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
		
	protected void ApplyAdminSettings()
	{
		CSI_ClientComponent clientComponent = CSI_ClientComponent.GetInstance();
		if (!clientComponent) return;
		
		if (m_wCompassVisibleWidgetSO.IsChecked()) {
			clientComponent.Owner_ChangeAuthoritySetting("compassVisibleServerOverride", m_wCompassVisibleWidget.IsChecked().ToString()); 
			m_wCompassVisibleWidget.SetEnabled(false);
			if (m_wCompassVisibleWidget.IsChecked()) { m_wCompassVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			clientComponent.Owner_ChangeAuthoritySetting("compassVisibleServerOverride", "N/A");
			m_wCompassVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));	
			m_wCompassVisibleWidget.SetEnabled(true);
		};
		
		if (m_wSquadRadarVisibleWidgetSO.IsChecked()) {
			clientComponent.Owner_ChangeAuthoritySetting("squadRadarVisibleServerOverride", m_wSquadRadarVisibleWidget.IsChecked().ToString()); 
			m_wSquadRadarVisibleWidget.SetEnabled(false); 
			if (m_wSquadRadarVisibleWidget.IsChecked()) { m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };				
		} else {
			clientComponent.Owner_ChangeAuthoritySetting("squadRadarVisibleServerOverride", "N/A");
			m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));		
			m_wSquadRadarVisibleWidget.SetEnabled(true);
		};
		
		if (m_wGroupDisplayVisibleWidgetSO.IsChecked()) {
			clientComponent.Owner_ChangeAuthoritySetting("groupDisplayVisibleServerOverride", m_wGroupDisplayVisibleWidget.IsChecked().ToString());
			m_wGroupDisplayVisibleWidget.SetEnabled(false);
			if (m_wGroupDisplayVisibleWidget.IsChecked()) { m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			clientComponent.Owner_ChangeAuthoritySetting("groupDisplayVisibleServerOverride", "N/A");
			m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wGroupDisplayVisibleWidget.SetEnabled(true);
		};
		
		if (m_wStaminaBarVisibleWidgetSO.IsChecked()) {
			clientComponent.Owner_ChangeAuthoritySetting("staminaBarVisibleServerOverride", m_wStaminaBarVisibleWidget.IsChecked().ToString()); 
			m_wStaminaBarVisibleWidget.SetEnabled(false); 
			if (m_wStaminaBarVisibleWidget.IsChecked()) { m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			clientComponent.Owner_ChangeAuthoritySetting("staminaBarVisibleServerOverride", "N/A"); 
			m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));		
			m_wStaminaBarVisibleWidget.SetEnabled(true);
		};
		
		if (m_wNametagsVisibleWidgetSO.IsChecked()) {
			clientComponent.Owner_ChangeAuthoritySetting("nametagsVisibleServerOverride", m_wNametagsVisibleWidget.IsChecked().ToString());
			m_wNametagsVisibleWidget.SetEnabled(false); 
			if (m_wNametagsVisibleWidget.IsChecked()) { m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };		
		} else {
			clientComponent.Owner_ChangeAuthoritySetting("nametagsVisibleServerOverride", "N/A"); 
			m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wNametagsVisibleWidget.SetEnabled(true);
		};
		
		if (m_wRankVisibleWidgetSO.IsChecked()) {
			clientComponent.Owner_ChangeAuthoritySetting("rankVisibleServerOverride", m_wRankVisibleWidget.IsChecked().ToString()); 
			m_wRankVisibleWidget.SetEnabled(false);	
			if (m_wRankVisibleWidget.IsChecked()) { m_wRankVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95));	};		
		} else {
			clientComponent.Owner_ChangeAuthoritySetting("rankVisibleServerOverride", "N/A");
			m_wRankVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));	
			m_wRankVisibleWidget.SetEnabled(true);
		};
	}
	
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseAllMenus();
	}
}