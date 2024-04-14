modded enum ChimeraMenuPreset {
	CSI_PlayerSettingsDialog,
	CSI_PlayerSelectionDialog,
	CSI_SettingsDialog
}

class CSI_SettingsDialog : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected CSI_ClientComponent m_ClientComponent;
	protected CSI_AuthorityComponent m_AuthorityComponent;

	protected int m_iNametagsRangeSO;
	
	protected string m_sCompassVisibleSO
	,m_sSquadRadarVisibleSO
	,m_sGroupDisplayVisibleSO
	,m_sStaminaBarVisibleSO
	,m_sNametagsVisibleSO
	,m_sRankVisibleSO
	,m_sRoleNametagVisibleSO
	,m_sPersonalColorTeamMenuSO;	

	protected CheckBoxWidget m_wCompassVisibleWidget
	,m_wSquadRadarVisibleWidget
	,m_wGroupDisplayVisibleWidget
	,m_wStaminaBarVisibleWidget
	,m_wNametagsVisibleWidget
	,m_wRankVisibleWidget
	,m_wRoleNametagVisibleWidget
	,m_wSquadRadarSelfIconVisibleWidget
	,m_wPersonalColorTeamMenuWidget
	,m_wCompassVisibleWidgetSO
	,m_wSquadRadarVisibleWidgetSO
	,m_wGroupDisplayVisibleWidgetSO
	,m_wStaminaBarVisibleWidgetSO
	,m_wNametagsVisibleWidgetSO
	,m_wRankVisibleWidgetSO
	,m_wNametagsRangeSO
	,m_wRoleNametagVisibleSO
	,m_wPersonalColorTeamMenuSO
	,m_wServerDefaults
	,m_wServerDefaultsActive;

	protected XComboBoxWidget m_wIconSizeWidget
	,m_wNametagsRangeWidget
	,m_wNametagsPosWidget;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------

	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_wRoot = GetRootWidget();
		
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Cancel").FindHandler(SCR_InputButtonComponent));
		cancel.m_OnClicked.Insert(OnMenuBack);

		m_ClientComponent = CSI_ClientComponent.GetInstance();
		if (!m_ClientComponent) return;

		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		if (!m_AuthorityComponent) return;

		GetGame().GetInputManager().AddActionListener("MenuSelectHold", EActionTrigger.DOWN, ApplySettings);
		SCR_InputButtonComponent confirm = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("Confirm").FindHandler(SCR_InputButtonComponent));
		confirm.m_OnActivated.Insert(ApplySettings);

		string squadRadarIconSizeStr = m_AuthorityComponent.ReturnLocalCSISettings()[9];
		m_wIconSizeWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("IconScaleXComboBox"));
		int squadRadarIconSize = 100;
		if (squadRadarIconSizeStr != "") {
			squadRadarIconSize = squadRadarIconSizeStr.ToInt();
		};

		if (squadRadarIconSize == 0) squadRadarIconSize = 100;
		m_wIconSizeWidget.SetCurrentItem((squadRadarIconSize/ 5) - 10);

		m_wNametagsPosWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsPosXComboBox"));
		string nametagsPosition = "";
		nametagsPosition = m_AuthorityComponent.ReturnLocalCSISettings()[11];
		if (nametagsPosition == "HEAD")
		{
			m_wNametagsPosWidget.SetCurrentItem(1);
		} else {
			m_wNametagsPosWidget.SetCurrentItem(0);
		};

		m_wSquadRadarSelfIconVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("squadRadarSelfIconVisibleCheckBox"));
		string squadRadarSelfIconVisible = "";
		squadRadarSelfIconVisible = m_AuthorityComponent.ReturnLocalCSISettings()[10];
		if (squadRadarSelfIconVisible == "false")
		{
			m_wSquadRadarSelfIconVisibleWidget.SetChecked(false);
		} else {
			m_wSquadRadarSelfIconVisibleWidget.SetChecked(true);
		};

		m_wCompassVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("CompassVisibleCheckBox"));
		m_sCompassVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[0];
		string compassVisible = "";
		switch (m_sCompassVisibleSO)
		{
			case ("false") : { m_wCompassVisibleWidget.SetChecked(false); m_wCompassVisibleWidget.SetEnabled(false); break; };
			case ("true") : { m_wCompassVisibleWidget.SetChecked(true); m_wCompassVisibleWidget.SetEnabled(false); m_wCompassVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				compassVisible = m_AuthorityComponent.ReturnLocalCSISettings()[0];
				// default state
				m_wCompassVisibleWidget.SetChecked(true);

				if (compassVisible == "false") {
					m_wCompassVisibleWidget.SetChecked(false);
				};
				break;
			};
		};

		m_wSquadRadarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("SquadRadarVisibleCheckBox"));
		m_sSquadRadarVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[1];
		string squadRadarVisible = "";
		switch (m_sSquadRadarVisibleSO)
		{
			case ("false") : { m_wSquadRadarVisibleWidget.SetChecked(false); m_wSquadRadarVisibleWidget.SetEnabled(false); break; };
			case ("true") : { m_wSquadRadarVisibleWidget.SetChecked(true); m_wSquadRadarVisibleWidget.SetEnabled(false); m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				squadRadarVisible = m_AuthorityComponent.ReturnLocalCSISettings()[1];
				// default state
				m_wSquadRadarVisibleWidget.SetChecked(true);

				if (squadRadarVisible == "false") {
					m_wSquadRadarVisibleWidget.SetChecked(false);
				};
				break;
			};
		};

		m_wGroupDisplayVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("GroupDisplayVisibleCheckBox"));
		m_sGroupDisplayVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[2];
		string groupDisplayVisible = "";
		switch (m_sGroupDisplayVisibleSO)
		{
			case ("false") : { m_wGroupDisplayVisibleWidget.SetChecked(false); m_wGroupDisplayVisibleWidget.SetEnabled(false); break; };
			case ("true") : { m_wGroupDisplayVisibleWidget.SetChecked(true); m_wGroupDisplayVisibleWidget.SetEnabled(false); m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				groupDisplayVisible = m_AuthorityComponent.ReturnLocalCSISettings()[2];
				// default state
				m_wGroupDisplayVisibleWidget.SetChecked(true);

				if (groupDisplayVisible == "false") {
					m_wGroupDisplayVisibleWidget.SetChecked(false);
				};
				break;
			};
		};

		m_wStaminaBarVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("StaminaBarVisibleCheckBox"));
		m_sStaminaBarVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[3];
		string staminaBarVisible = "";
		switch (m_sStaminaBarVisibleSO)
		{
			case ("false") : { m_wStaminaBarVisibleWidget.SetChecked(false); m_wStaminaBarVisibleWidget.SetEnabled(false); break; };
			case ("true") : { m_wStaminaBarVisibleWidget.SetChecked(true); m_wStaminaBarVisibleWidget.SetEnabled(false); m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				staminaBarVisible = m_AuthorityComponent.ReturnLocalCSISettings()[3];
				// default state
				m_wStaminaBarVisibleWidget.SetChecked(true);

				if (staminaBarVisible == "false") {
					m_wStaminaBarVisibleWidget.SetChecked(false);
				};
				break;
			};
		};

		m_wNametagsVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsVisibleCheckbox"));
		m_sNametagsVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[4];
		string nametagsVisible = "";
		switch (m_sNametagsVisibleSO)
		{
			case ("false") : { m_wNametagsVisibleWidget.SetChecked(false); m_wNametagsVisibleWidget.SetEnabled(false); break; };
			case ("true") : { m_wNametagsVisibleWidget.SetChecked(true); m_wNametagsVisibleWidget.SetEnabled(false); m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				nametagsVisible = m_AuthorityComponent.ReturnLocalCSISettings()[4];
				// default state
				m_wNametagsVisibleWidget.SetChecked(true);

				if (nametagsVisible == "false") {
					m_wNametagsVisibleWidget.SetChecked(false);
				};
				break;
			};
		};

		m_wRankVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("RankVisibleCheckBox"));
		m_sRankVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[5];
		string rankVisible = "";
		switch (m_sRankVisibleSO)
		{
			case ("false") : { m_wRankVisibleWidget.SetChecked(false); m_wRankVisibleWidget.SetEnabled(false); break; };
			case ("true") : { m_wRankVisibleWidget.SetChecked(true); m_wRankVisibleWidget.SetEnabled(false); m_wRankVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				rankVisible = m_AuthorityComponent.ReturnLocalCSISettings()[5];
				// default state
				m_wRankVisibleWidget.SetChecked(false);

				if (rankVisible == "true") {
					m_wRankVisibleWidget.SetChecked(true);
				};
				break;
			};
		};

		m_wNametagsRangeWidget = XComboBoxWidget.Cast(m_wRoot.FindAnyWidget("NametagsRangeXComboBox"));
		string nametagsRangeSOString = m_AuthorityComponent.ReturnAuthoritySettings()[6];
		if (nametagsRangeSOString != "") {
			m_iNametagsRangeSO = nametagsRangeSOString.ToInt();
		} else {
			m_iNametagsRangeSO = 0;
		};
		string nametagsRangeStr = "0";
		switch (true)
		{
			case (m_iNametagsRangeSO > 0) : { m_wNametagsRangeWidget.SetCurrentItem((m_iNametagsRangeSO / 5) - 1); m_wNametagsRangeWidget.SetEnabled(false); break; };
			default : {
				nametagsRangeStr = m_AuthorityComponent.ReturnLocalCSISettings()[6];
				int nametagsRange = nametagsRangeStr.ToInt();
				// default state
				m_wNametagsRangeWidget.SetCurrentItem(6);

				if (nametagsRange > 0) {
					m_wNametagsRangeWidget.SetCurrentItem((nametagsRange / 5) - 1);
				};
				break;
			};
		};

		m_wRoleNametagVisibleWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("roleNametagVisibleCheckbox"));
		m_sRoleNametagVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[7];
		string roleNametagVisible = "";
		switch (m_sRoleNametagVisibleSO)
		{
			case ("false") : { m_wRoleNametagVisibleWidget.SetChecked(false); m_wRoleNametagVisibleWidget.SetEnabled(false); break; };
			case ("true") : { m_wRoleNametagVisibleWidget.SetChecked(true); m_wRoleNametagVisibleWidget.SetEnabled(false); m_wRoleNametagVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				roleNametagVisible = m_AuthorityComponent.ReturnLocalCSISettings()[7];
				// default state
				m_wRoleNametagVisibleWidget.SetChecked(false);

				if (roleNametagVisible == "true") {
					m_wRoleNametagVisibleWidget.SetChecked(true);
				};
				break;
			};
		};

		m_wPersonalColorTeamMenuWidget = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("PersonalColorTeamMenuCheckbox"));
		m_sPersonalColorTeamMenuSO = m_AuthorityComponent.ReturnAuthoritySettings()[8];
		string personalColorTeamMenuEnabled = "";
		switch (m_sPersonalColorTeamMenuSO)
		{
			case ("false") : { m_wPersonalColorTeamMenuWidget.SetChecked(false); m_wPersonalColorTeamMenuWidget.SetEnabled(false); break; };
			case ("true") : { m_wPersonalColorTeamMenuWidget.SetChecked(true); m_wPersonalColorTeamMenuWidget.SetEnabled(false); m_wPersonalColorTeamMenuWidget.SetColorInt(ARGB(255, 95, 95, 95)); break; };
			default : {
				personalColorTeamMenuEnabled = m_AuthorityComponent.ReturnLocalCSISettings()[8];
				// default state
				m_wPersonalColorTeamMenuWidget.SetChecked(true);

				if (personalColorTeamMenuEnabled == "false") {
					m_wPersonalColorTeamMenuWidget.SetChecked(false);
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
		m_wCompassVisibleWidgetSO                = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("compassVisibleServerOverride"));
		m_wSquadRadarVisibleWidgetSO             = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("squadRadarVisibleServerOverride"));
		m_wGroupDisplayVisibleWidgetSO           = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("groupDisplayVisibleServerOverride"));
		m_wStaminaBarVisibleWidgetSO             = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("staminaBarVisibleServerOverride"));
		m_wNametagsVisibleWidgetSO               = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("nametagsVisibleServerOverride"));
		m_wRankVisibleWidgetSO                   = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("showRankServerOverride"));
		m_wNametagsRangeSO                       = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("nametagsRangeServerOverride"));
		m_wRoleNametagVisibleSO                  = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("roleNametagVisibleServerOverride"));
		m_wPersonalColorTeamMenuSO               = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("personalColorTeamMenuServerOverride"));

		ImageWidget backgroundServerOverride     = ImageWidget.Cast(m_wRoot.FindAnyWidget("BackgroundServerOverride"));
		ImageWidget prettyServerOverride         = ImageWidget.Cast(m_wRoot.FindAnyWidget("PrettyServerOverride"));

		TextWidget prettyTextServerOverride      = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride"));
		TextWidget prettyTextServerOverrideOne   = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride1"));
		TextWidget prettyTextServerOverrideTwo   = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride2"));
		TextWidget prettyTextServerOverrideThree = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride3"));
		TextWidget prettyTextServerOverrideFour  = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride4"));
		TextWidget prettyTextServerOverrideFive  = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerOverride5"));
		TextWidget seperatorServerOverride       = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride"));
		TextWidget seperatorServerOverrideOne    = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride1"));
		TextWidget seperatorServerOverrideTwo    = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride2"));
		TextWidget seperatorServerOverrideThree  = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride3"));
		TextWidget seperatorServerOverrideFour   = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerOverride4"));
		
		m_wServerDefaults                        = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("ServerDefaults"));
		m_wServerDefaultsActive                  = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("ServerDefaultsActive"));
		
		TextWidget seperatorServerDefaults0      = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerDefaults0"));
		TextWidget seperatorServerDefaults1      = TextWidget.Cast(m_wRoot.FindAnyWidget("SeperatorServerDefaults1"));
		TextWidget prettyTextServerDefaults0     = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerDefaults0"));
		TextWidget prettyTextServerDefaults1     = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerDefaults1"));
		TextWidget prettyTextServerDefaults2     = TextWidget.Cast(m_wRoot.FindAnyWidget("PrettyTextServerDefaults2"));
		ImageWidget backgroundServerDefaults     = ImageWidget.Cast(m_wRoot.FindAnyWidget("BackgroundServerDefaults"));
		ImageWidget prettyServerDefaults         = ImageWidget.Cast(m_wRoot.FindAnyWidget("PrettyServerDefaults"));


		m_wCompassVisibleWidgetSO.SetOpacity(1);
		m_wSquadRadarVisibleWidgetSO.SetOpacity(1);
		m_wGroupDisplayVisibleWidgetSO.SetOpacity(1);
		m_wStaminaBarVisibleWidgetSO.SetOpacity(1);
		m_wNametagsVisibleWidgetSO.SetOpacity(1);
		m_wRankVisibleWidgetSO.SetOpacity(1);
		m_wRoleNametagVisibleSO.SetOpacity(1);
		m_wNametagsRangeSO.SetOpacity(1);
		m_wPersonalColorTeamMenuSO.SetOpacity(1);

		m_wCompassVisibleWidgetSO.SetEnabled(true);
		m_wSquadRadarVisibleWidgetSO.SetEnabled(true);
		m_wGroupDisplayVisibleWidgetSO.SetEnabled(true);
		m_wStaminaBarVisibleWidgetSO.SetEnabled(true);
		m_wNametagsVisibleWidgetSO.SetEnabled(true);
		m_wRankVisibleWidgetSO.SetEnabled(true);
		m_wRoleNametagVisibleSO.SetEnabled(true);
		m_wNametagsRangeSO.SetEnabled(true);
		m_wPersonalColorTeamMenuSO.SetEnabled(true);

		backgroundServerOverride.SetOpacity(1);
		prettyServerOverride.SetOpacity(1);
		prettyTextServerOverride.SetOpacity(1);
		prettyTextServerOverrideOne.SetOpacity(1);
		prettyTextServerOverrideTwo.SetOpacity(1);
		prettyTextServerOverrideThree.SetOpacity(1);
		prettyTextServerOverrideFour.SetOpacity(1);
		prettyTextServerOverrideFive.SetOpacity(1);
		seperatorServerOverride.SetOpacity(1);
		seperatorServerOverrideOne.SetOpacity(1);
		seperatorServerOverrideTwo.SetOpacity(1);
		seperatorServerOverrideThree.SetOpacity(1);
		seperatorServerOverrideFour.SetOpacity(1);
		
		m_wServerDefaults.SetEnabled(true);
		m_wServerDefaultsActive.SetEnabled(true);
	
		m_wServerDefaults.SetOpacity(1); 	
		m_wServerDefaultsActive.SetOpacity(1);
		seperatorServerDefaults0.SetOpacity(1);
		seperatorServerDefaults1.SetOpacity(1);
		prettyTextServerDefaults0.SetOpacity(1);
		prettyTextServerDefaults1.SetOpacity(1);
		prettyTextServerDefaults2.SetOpacity(1);
		backgroundServerDefaults.SetOpacity(1);
		prettyServerDefaults.SetOpacity(1);
		
		if (m_AuthorityComponent.ReturnAuthoritySettings()[9] == "true") m_wServerDefaultsActive.SetChecked(true);

		if (m_sCompassVisibleSO == "true" || m_sCompassVisibleSO == "false") m_wCompassVisibleWidgetSO.SetChecked(true);
		if (m_sSquadRadarVisibleSO == "true" || m_sSquadRadarVisibleSO == "false") m_wSquadRadarVisibleWidgetSO.SetChecked(true);
		if (m_sGroupDisplayVisibleSO == "true" || m_sGroupDisplayVisibleSO == "false") m_wGroupDisplayVisibleWidgetSO.SetChecked(true);
		if (m_sStaminaBarVisibleSO == "true" || m_sStaminaBarVisibleSO == "false") m_wStaminaBarVisibleWidgetSO.SetChecked(true);
		if (m_sNametagsVisibleSO == "true" || m_sNametagsVisibleSO == "false") m_wNametagsVisibleWidgetSO.SetChecked(true);
		if (m_sRankVisibleSO == "true" || m_sRankVisibleSO == "false") m_wRankVisibleWidgetSO.SetChecked(true);
		if (m_sRoleNametagVisibleSO == "true" || m_sRoleNametagVisibleSO == "false") m_wRoleNametagVisibleSO.SetChecked(true);
		if (m_iNametagsRangeSO > 0) m_wNametagsRangeSO.SetChecked(true);
		if (m_sPersonalColorTeamMenuSO == "true" || m_sPersonalColorTeamMenuSO == "false") m_wPersonalColorTeamMenuSO.SetChecked(true);
	};

	protected void ApplySettings()
	{
		m_AuthorityComponent.ChangeLocalCSISetting("compassVisible",            m_wCompassVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("squadRadarVisible",         m_wSquadRadarVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("groupDisplayVisible",       m_wGroupDisplayVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("staminaBarVisible",         m_wStaminaBarVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("nametagsVisible",           m_wNametagsVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("rankVisible",               m_wRankVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("squadRadarIconSize",        ((m_wIconSizeWidget.GetCurrentItem() * 5) + 50).ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("nametagsRange",             ((m_wNametagsRangeWidget.GetCurrentItem() + 1) * 5).ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("roleNametagVisible",        m_wRoleNametagVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("squadRadarSelfIconVisible", m_wSquadRadarSelfIconVisibleWidget.IsChecked().ToString());
		m_AuthorityComponent.ChangeLocalCSISetting("personalColorTeamMenu",     m_wPersonalColorTeamMenuWidget.IsChecked().ToString());

		if (m_wNametagsPosWidget.GetCurrentItem() == 0) {
			m_AuthorityComponent.ChangeLocalCSISetting("nametagsPosition", "BODY");
		} else {
			m_AuthorityComponent.ChangeLocalCSISetting("nametagsPosition", "HEAD");
		};

		if (SCR_Global.IsAdmin()) ApplyAdminSettings();
	}

	protected void ApplyAdminSettings()
	{
		if (!m_ClientComponent) return;
		
		if (m_wServerDefaultsActive.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("serverDefaultsActive", "true");
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("serverDefaultsActive", "false");
		}
		
		if (m_wServerDefaults.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("compassVisibleServerDefault",        m_wCompassVisibleWidget.IsChecked().ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("squadRadarVisibleServerDefault",     m_wSquadRadarVisibleWidget.IsChecked().ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("groupDisplayVisibleServerDefault",   m_wGroupDisplayVisibleWidget.IsChecked().ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("staminaBarVisibleServerDefault",     m_wStaminaBarVisibleWidget.IsChecked().ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsVisibleServerDefault",       m_wNametagsVisibleWidget.IsChecked().ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("rankVisibleServerDefault",           m_wRankVisibleWidget.IsChecked().ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsRangeServerDefault",         ((m_wNametagsRangeWidget.GetCurrentItem() + 1) * 5).ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("roleNametagVisibleServerDefault",    m_wRoleNametagVisibleWidget.IsChecked().ToString());
			m_ClientComponent.Owner_ChangeAuthoritySetting("personalColorTeamMenuServerDefault", m_wPersonalColorTeamMenuWidget.IsChecked().ToString());
		};
	
		if (m_wCompassVisibleWidgetSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("compassVisibleServerOverride", m_wCompassVisibleWidget.IsChecked().ToString());
			m_wCompassVisibleWidget.SetEnabled(false);
			if (m_wCompassVisibleWidget.IsChecked()) { m_wCompassVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("compassVisibleServerOverride", "N/A");
			m_wCompassVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wCompassVisibleWidget.SetEnabled(true);
		};

		if (m_wSquadRadarVisibleWidgetSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("squadRadarVisibleServerOverride", m_wSquadRadarVisibleWidget.IsChecked().ToString());
			m_wSquadRadarVisibleWidget.SetEnabled(false);
			if (m_wSquadRadarVisibleWidget.IsChecked()) { m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("squadRadarVisibleServerOverride", "N/A");
			m_wSquadRadarVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wSquadRadarVisibleWidget.SetEnabled(true);
		};

		if (m_wGroupDisplayVisibleWidgetSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("groupDisplayVisibleServerOverride", m_wGroupDisplayVisibleWidget.IsChecked().ToString());
			m_wGroupDisplayVisibleWidget.SetEnabled(false);
			if (m_wGroupDisplayVisibleWidget.IsChecked()) { m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("groupDisplayVisibleServerOverride", "N/A");
			m_wGroupDisplayVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wGroupDisplayVisibleWidget.SetEnabled(true);
		};

		if (m_wStaminaBarVisibleWidgetSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("staminaBarVisibleServerOverride", m_wStaminaBarVisibleWidget.IsChecked().ToString());
			m_wStaminaBarVisibleWidget.SetEnabled(false);
			if (m_wStaminaBarVisibleWidget.IsChecked()) { m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("staminaBarVisibleServerOverride", "N/A");
			m_wStaminaBarVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wStaminaBarVisibleWidget.SetEnabled(true);
		};

		if (m_wNametagsVisibleWidgetSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsVisibleServerOverride", m_wNametagsVisibleWidget.IsChecked().ToString());
			m_wNametagsVisibleWidget.SetEnabled(false);
			if (m_wNametagsVisibleWidget.IsChecked()) { m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95)); };
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsVisibleServerOverride", "N/A");
			m_wNametagsVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wNametagsVisibleWidget.SetEnabled(true);
		};

		if (m_wRankVisibleWidgetSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("rankVisibleServerOverride", m_wRankVisibleWidget.IsChecked().ToString());
			m_wRankVisibleWidget.SetEnabled(false);
			if (m_wRankVisibleWidget.IsChecked()) { m_wRankVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95));	};
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("rankVisibleServerOverride", "N/A");
			m_wRankVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wRankVisibleWidget.SetEnabled(true);
		};

		if (m_wNametagsRangeSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsRangeServerOverride", ((m_wNametagsRangeWidget.GetCurrentItem() + 1) * 5).ToString());
			m_wNametagsRangeWidget.SetEnabled(false);
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("nametagsRangeServerOverride", "N/A");
			m_wNametagsRangeWidget.SetEnabled(true);
		};

		if (m_wRoleNametagVisibleSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("roleNametagVisibleServerOverride", m_wRoleNametagVisibleWidget.IsChecked().ToString());
			m_wRoleNametagVisibleWidget.SetEnabled(false);
			if (m_wRoleNametagVisibleWidget.IsChecked()) { m_wRoleNametagVisibleWidget.SetColorInt(ARGB(255, 95, 95, 95));	};
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("roleNametagVisibleServerOverride", "N/A");
			m_wRoleNametagVisibleWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wRoleNametagVisibleWidget.SetEnabled(true);
		};

		if (m_wPersonalColorTeamMenuSO.IsChecked()) {
			m_ClientComponent.Owner_ChangeAuthoritySetting("personalColorTeamMenuServerOverride", m_wPersonalColorTeamMenuWidget.IsChecked().ToString());
			m_wPersonalColorTeamMenuWidget.SetEnabled(false);
			if (m_wPersonalColorTeamMenuWidget.IsChecked()) { m_wPersonalColorTeamMenuWidget.SetColorInt(ARGB(255, 95, 95, 95));	};
		} else {
			m_ClientComponent.Owner_ChangeAuthoritySetting("personalColorTeamMenuServerOverride", "N/A");
			m_wPersonalColorTeamMenuWidget.SetColorInt(ARGB(255, 255, 255, 255));
			m_wPersonalColorTeamMenuWidget.SetEnabled(true);
		};
	}

	protected void OnMenuBack()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
		GetGame().GetInputManager().RemoveActionListener("MenuSelectHold", EActionTrigger.DOWN, ApplySettings);
		GetGame().GetMenuManager().CloseAllMenus();
	}
}
