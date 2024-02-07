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
	protected int m_iNametagsRangeSO;
	
	protected CheckBoxWidget m_wCompassVisibleWidget;
	protected CheckBoxWidget m_wSquadRadarVisibleWidget;
	protected CheckBoxWidget m_wGroupDisplayVisibleWidget;
	protected CheckBoxWidget m_wStaminaBarVisibleWidget;
	protected CheckBoxWidget m_wNametagsVisibleWidget;
	protected CheckBoxWidget m_wRankVisibleWidget;
	protected XComboBoxWidget m_wIconSizeWidget;
	protected XComboBoxWidget m_wNametagsRangeWidget;
	protected XComboBoxWidget m_wNametagsPosWidget;
	
	protected CheckBoxWidget m_wCompassVisibleWidgetSO;
	protected CheckBoxWidget m_wSquadRadarVisibleWidgetSO;
	protected CheckBoxWidget m_wGroupDisplayVisibleWidgetSO;
	protected CheckBoxWidget m_wStaminaBarVisibleWidgetSO;
	protected CheckBoxWidget m_wNametagsVisibleWidgetSO;
	protected CheckBoxWidget m_wRankVisibleWidgetSO;
	protected CheckBoxWidget m_wNametagsRangeServerOverrideSO;

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
		
		float squadRadarIconSize = 100;
		m_wIconSizeWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconScaleXComboBox"));
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarIconSize", squadRadarIconSize);
		if (squadRadarIconSize == 0) squadRadarIconSize = 100; 
		m_wIconSizeWidget.SetCurrentItem((squadRadarIconSize/5) - 10);
		
		m_wNametagsPosWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsPosXComboBox"));
		string nametagsPosition = "";
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsPosition", nametagsPosition);
		if (nametagsPosition == "HEAD") 
		{
			m_wNametagsPosWidget.SetCurrentItem(1);
		} else {
			m_wNametagsPosWidget.SetCurrentItem(0);
		};
		 
		// ToDo: Impliment this
		//GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassTexture", m_sCompassTexture);
		
		m_wCompassVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("CompassVisibleCheckBox"));
		m_sCompassVisibleSO = authorityComponent.ReturnAuthoritySettings()[0];
		string compassVisible = "";
		switch (m_sCompassVisibleSO)
		{
			case ("false") : { m_wCompassVisibleWidget.SetChecked(false); m_wCompassVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wCompassVisibleWidget.SetChecked(true);  m_wCompassVisibleWidget.SetEnabled(false); m_wCompassVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassVisible", compassVisible); 
				// default state
				m_wCompassVisibleWidget.SetChecked(true);
				
				if (compassVisible == "false") {
					m_wCompassVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
			
		m_wSquadRadarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("SquadRadarVisibleCheckBox"));
		m_sSquadRadarVisibleSO = authorityComponent.ReturnAuthoritySettings()[1];
		string squadRadarVisible = "";
		switch (m_sSquadRadarVisibleSO)
		{
			case ("false") : { m_wSquadRadarVisibleWidget.SetChecked(false); m_wSquadRadarVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wSquadRadarVisibleWidget.SetChecked(true);  m_wSquadRadarVisibleWidget.SetEnabled(false); m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarVisible", squadRadarVisible); 
				// default state
				m_wSquadRadarVisibleWidget.SetChecked(true);
				
				if (squadRadarVisible == "false") {
					m_wSquadRadarVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wGroupDisplayVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("GroupDisplayVisibleCheckBox"));
		m_sGroupDisplayVisibleSO = authorityComponent.ReturnAuthoritySettings()[2];
		string groupDisplayVisible = "";
		switch (m_sGroupDisplayVisibleSO)
		{
			case ("false") : { m_wGroupDisplayVisibleWidget.SetChecked(false); m_wGroupDisplayVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wGroupDisplayVisibleWidget.SetChecked(true);  m_wGroupDisplayVisibleWidget.SetEnabled(false); m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("groupDisplayVisible", groupDisplayVisible); 
				// default state
				m_wGroupDisplayVisibleWidget.SetChecked(true);
				
				if (groupDisplayVisible == "false") {
					m_wGroupDisplayVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wStaminaBarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("StaminaBarVisibleCheckBox"));
		m_sStaminaBarVisibleSO = authorityComponent.ReturnAuthoritySettings()[3];
		string staminaBarVisible = "";
		switch (m_sStaminaBarVisibleSO)
		{
			case ("false") : { m_wStaminaBarVisibleWidget.SetChecked(false); m_wStaminaBarVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wStaminaBarVisibleWidget.SetChecked(true);  m_wStaminaBarVisibleWidget.SetEnabled(false); m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("staminaBarVisible", staminaBarVisible); 
				// default state
				m_wStaminaBarVisibleWidget.SetChecked(true);
				
				if (staminaBarVisible == "false") {
					m_wStaminaBarVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wNametagsVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsVisibleCheckbox"));
		m_sNametagsVisibleSO = authorityComponent.ReturnAuthoritySettings()[4];
		string nametagsVisible = "";
		switch (m_sNametagsVisibleSO)
		{
			case ("false") : { m_wNametagsVisibleWidget.SetChecked(false); m_wNametagsVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wNametagsVisibleWidget.SetChecked(true);  m_wNametagsVisibleWidget.SetEnabled(false); m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsVisible", nametagsVisible); 
				// default state
				m_wNametagsVisibleWidget.SetChecked(true);
				
				if (nametagsVisible == "false") {
					m_wNametagsVisibleWidget.SetChecked(false);
				};
				break;
			};
		};
		
		m_wRankVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("RankVisibleCheckBox"));
		m_sRankVisibleSO = authorityComponent.ReturnAuthoritySettings()[5];
		string rankVisible = "";
		switch (m_sRankVisibleSO)
		{
			case ("false") : { m_wRankVisibleWidget.SetChecked(false); m_wRankVisibleWidget.SetEnabled(false); break;};
			case ("true") : { m_wRankVisibleWidget.SetChecked(true);  m_wRankVisibleWidget.SetEnabled(false); m_wRankVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("rankVisible", rankVisible); 
				// default state
				m_wRankVisibleWidget.SetChecked(false);
				
				if (rankVisible == "true") {
					m_wRankVisibleWidget.SetChecked(true);
				};
				break;
			};
		};
		
		m_wNametagsRangeWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsRangeXComboBox"));
		string nametagsRangeSOString = authorityComponent.ReturnAuthoritySettings()[6];
		if (nametagsRangeSOString != "") {
			m_iNametagsRangeSO = nametagsRangeSOString.ToInt();
		} else {
			m_iNametagsRangeSO = 0;
		};
		int nametagsRange = 0;
		switch (true)
		{
			case (m_iNametagsRangeSO > 0) : { m_wNametagsRangeWidget.SetCurrentItem(m_iNametagsRangeSO / 5); m_wNametagsRangeWidget.SetEnabled(false); break;};
			default  : { 
				GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsRange", nametagsRange); 
				// default state
				m_wNametagsRangeWidget.SetCurrentItem(1);
				
				if (nametagsRange > 0 ) {
					m_wNametagsRangeWidget.SetCurrentItem((nametagsRange / 5) - 1);
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
		m_wCompassVisibleWidgetSO       					= CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("compassVisibleServerOverride"));
		m_wSquadRadarVisibleWidgetSO    					= CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("squadRadarVisibleServerOverride"));
		m_wGroupDisplayVisibleWidgetSO   					= CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("groupDisplayVisibleServerOverride"));
		m_wStaminaBarVisibleWidgetSO     					= CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("staminaBarVisibleServerOverride"));
		m_wNametagsVisibleWidgetSO       					= CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("nametagsVisibleServerOverride"));
		m_wRankVisibleWidgetSO                   = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("showRankServerOverride"));
		m_wNametagsRangeServerOverrideSO         = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("nametagsRangeServerOverride"));
		
		ImageWidget backgroundServerOverride     = ImageWidget.Cast(m_wRoot.FindAnyWidget("BackgroundServerOverride"));
		ImageWidget prettyServerOverride         = ImageWidget.Cast(m_wRoot.FindAnyWidget("PrettyServerOverride"));
		
		TextWidget prettyTextServerOverride      = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride"));
		TextWidget prettyTextServerOverrideOne   = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride1"));
		TextWidget prettyTextServerOverrideTwo   = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride2"));
		TextWidget prettyTextServerOverrideThree = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride3"));
		TextWidget prettyTextServerOverrideFour  = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride4"));
		TextWidget seperatorServerOverride       = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride"));
		TextWidget seperatorServerOverrideOne    = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride1"));
		TextWidget seperatorServerOverrideTwo    = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride2"));
		TextWidget seperatorServerOverrideThree  = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride3"));

		
		m_wCompassVisibleWidgetSO.SetOpacity(1);
		m_wSquadRadarVisibleWidgetSO.SetOpacity(1);
		m_wGroupDisplayVisibleWidgetSO.SetOpacity(1);
		m_wStaminaBarVisibleWidgetSO.SetOpacity(1);
		m_wNametagsVisibleWidgetSO.SetOpacity(1);
		m_wRankVisibleWidgetSO.SetOpacity(1);
		m_wNametagsRangeServerOverrideSO.SetOpacity(1);
		backgroundServerOverride.SetOpacity(1);
		prettyServerOverride.SetOpacity(1);
		prettyTextServerOverride.SetOpacity(1);
		prettyTextServerOverrideOne.SetOpacity(1);
		prettyTextServerOverrideTwo.SetOpacity(1);
		prettyTextServerOverrideThree.SetOpacity(1);
		prettyTextServerOverrideFour.SetOpacity(1);
		seperatorServerOverride.SetOpacity(1);
		seperatorServerOverrideOne.SetOpacity(1);
		seperatorServerOverrideTwo.SetOpacity(1);
		seperatorServerOverrideThree.SetOpacity(1);
	
		if (m_sCompassVisibleSO == "true" || m_sCompassVisibleSO == "false") m_wCompassVisibleWidgetSO.SetChecked(true);
		if (m_sSquadRadarVisibleSO == "true" || m_sSquadRadarVisibleSO == "false") m_wSquadRadarVisibleWidgetSO.SetChecked(true);
		if (m_sGroupDisplayVisibleSO == "true" || m_sGroupDisplayVisibleSO == "false") m_wGroupDisplayVisibleWidgetSO.SetChecked(true);
		if (m_sStaminaBarVisibleSO == "true" || m_sStaminaBarVisibleSO == "false") m_wStaminaBarVisibleWidgetSO.SetChecked(true);
		if (m_sNametagsVisibleSO == "true" || m_sNametagsVisibleSO == "false") m_wNametagsVisibleWidgetSO.SetChecked(true);
		if (m_sRankVisibleSO == "true" || m_sRankVisibleSO == "false") m_wRankVisibleWidgetSO.SetChecked(true);
		if (m_iNametagsRangeSO > 0) m_wNametagsRangeServerOverrideSO.SetChecked(true);
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
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("nametagsRange",       ((m_wNametagsRangeWidget.GetCurrentItem() + 1) * 5));
		
		if (m_wNametagsPosWidget.GetCurrentItem() == 0) {
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("nametagsPosition", "BODY");
		} else {
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Set("nametagsPosition", "HEAD");
		};
		
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
		
		if (m_wNametagsRangeServerOverrideSO.IsChecked()) {
			clientComponent.Owner_ChangeAuthoritySetting("nametagsRangeServerOverride", (m_wNametagsRangeWidget.GetCurrentItem() * 5).ToString()); 
			m_wNametagsRangeWidget.SetEnabled(false);		
		} else {
			clientComponent.Owner_ChangeAuthoritySetting("nametagsRangeServerOverride", "0");
			m_wNametagsRangeWidget.SetEnabled(true);
		};
	}
	
	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetMenuManager().CloseAllMenus();
	}
}