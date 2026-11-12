void ScriptInvoker_CSI_SettingsSubMenu(CSI_SettingsSubMenu menu);
typedef func ScriptInvoker_CSI_SettingsSubMenu;
typedef ScriptInvokerBase<ScriptInvoker_CSI_SettingsSubMenu> ScriptInvoker_CSI_SettingsSubMenuChanged;

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	CSI tab in Settings. Settings the server overrides show the forced value and are locked for non-admins;
//! 				admins get a server override checkbox per setting (sent when the tab closes). The right side holds two
//! 				sub-layouts with their own components, driven from here like CSI_HUD drives its elements: Setting Info for
//! 				the hovered setting (CSI_SettingInfo) and, at the bottom, the admin-only Server Admin panel (CSI_ServerAdmin).
//! 				Players without saved settings see the server's defaults here; their settings file is only written once
//! 				they change something.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingsSubMenu: SCR_SettingsSubMenuBase
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerControllerManager m_PlayerControllerManager;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		// Bindings are rebuilt below, skipping server-overridden settings
		m_aSettingsBindings.Clear();

		if (!m_SettingsManager || !m_RplToAuthorityManager || !m_PlayerControllerManager)
		{
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
			m_PlayerControllerManager = CSI_PlayerControllerManager.GetInstance();
		};

		Widget serverOverridesLayoutWidget = m_wRoot.FindAnyWidget("ServerOverrides");
		foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
		{
			bool setVanillaSettingBind = false;

			if (m_SettingsManager)
			{
				if (m_SettingsManager.GetServerSettingsArray().Get(i) < 0)
				{
					Widget serverOverrideLayout = serverOverridesLayoutWidget.FindAnyWidget(settingStr);
					CheckBoxWidget checkBoxWidget = CheckBoxWidget.Cast(serverOverrideLayout.FindAnyWidget("ServerOverride"));

					// Overriden By Server, Grey out and dont insert so we arent overriding local player values with server overriden ones
					Widget widget = m_wRoot.FindAnyWidget(settingStr);

					SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
					SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(widget.FindHandler(SCR_ComboBoxComponent));
					SCR_SpinBoxComponent spinComp = SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));

					switch (true)
					{
						case (sliderComp) : sliderComp.SetValue(m_SettingsManager.GetSettingInt(settingStr)); break;
						case (comboComp) : comboComp.SetCurrentItem(m_SettingsManager.GetSettingInt(settingStr), false, true); break;
						case (spinComp) : spinComp.SetCurrentItem(m_SettingsManager.GetSettingInt(settingStr), false, true); break;
					}

					checkBoxWidget.SetChecked(true);

					if (!SCR_Global.IsAdmin())
						widget.SetEnabled(false);
				} else
					setVanillaSettingBind = true;
			} else
				setVanillaSettingBind = true;

			if (setVanillaSettingBind)
				m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", settingStr, settingStr));
		}

		if (!SCR_Global.IsAdmin())
		{
			serverOverridesLayoutWidget.SetVisible(false);
			serverOverridesLayoutWidget.SetEnabled(false);
		}

		LoadSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		if (!m_SettingsManager || !m_RplToAuthorityManager || !m_PlayerControllerManager)
		{
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
			m_PlayerControllerManager = CSI_PlayerControllerManager.GetInstance();
		};

			m_PlayerControllerManager.GetLocalSettingsJson().SaveToFile();

		if (m_SettingsManager && m_RplToAuthorityManager)
		{
			if (SCR_Global.IsAdmin())
			{
				// Send every setting: checked = override with the widget's value, unchecked = clear the override
				Widget serverOverridesLayoutWidget = m_wRoot.FindAnyWidget("ServerOverrides");
				foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
				{
					Widget contentWidget = m_wRoot.FindAnyWidget(settingStr);
					Widget serverOverrideLayout = serverOverridesLayoutWidget.FindAnyWidget(settingStr);

					CheckBoxWidget checkBoxWidget = CheckBoxWidget.Cast(serverOverrideLayout.FindAnyWidget("ServerOverride"));
					int value;

					SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(contentWidget.FindHandler(SCR_SliderComponent));
					SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(contentWidget.FindHandler(SCR_ComboBoxComponent));
					SCR_SpinBoxComponent spinComp = SCR_SpinBoxComponent.Cast(contentWidget.FindHandler(SCR_SpinBoxComponent));

					switch (true)
					{
						case (sliderComp) : value = sliderComp.GetValue(); break;
						case (comboComp) : value = comboComp.GetCurrentIndex(); break;
						case (spinComp) : value = spinComp.GetCurrentIndex(); break;
					}

					if (checkBoxWidget.IsChecked())
						m_RplToAuthorityManager.Owner_ChangeAuthoritySetting(settingStr, value, true);
					else
						m_RplToAuthorityManager.Owner_ChangeAuthoritySetting(settingStr, 0, false);
				}
			};

			m_SettingsManager.RequestSettingsUpdate();
		};
	}
}