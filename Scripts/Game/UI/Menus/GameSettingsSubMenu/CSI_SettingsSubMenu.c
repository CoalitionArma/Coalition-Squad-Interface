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

	protected ref TIntArray m_aValuesOnOpen;				//!< user settings when the tab opened, to tell whether the player changed anything
	protected ref TIntArray m_aValuesBeforeServerDefaults;	//!< user settings before the server defaults were shown, restored if not saved
	protected bool m_bShowingServerDefaults;				//!< the player has no saved settings, so this tab shows the server's defaults

	// Components of the right side's sub-layouts, owned by their widgets
	protected CSI_SettingInfo m_SettingInfo;
	protected CSI_ServerAdmin m_ServerAdmin;

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

		// Players without saved settings get the server's defaults in game, so show (and let them edit) those here
		m_aValuesOnOpen = GetUserSettingValues();
		if (IsInSessionWithoutSavedSettings() && m_SettingsManager && m_SettingsManager.HasServerDefaults())
		{
			m_aValuesBeforeServerDefaults = m_aValuesOnOpen;
			m_SettingsManager.ApplyServerDefaultsToUserSettings();
			m_aValuesOnOpen = GetUserSettingValues();
			m_bShowingServerDefaults = true;
		}

		Widget serverOverridesLayoutWidget = m_wRoot.FindAnyWidget("ServerOverrides");
		foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
		{
			bool setVanillaSettingBind = false;

			// The authority array can be empty before replication arrives, or shorter if the server runs an older version
			TIntArray serverSettings;
			if (m_SettingsManager)
				serverSettings = m_SettingsManager.GetServerSettingsArray();

			if (serverSettings && serverSettings.IsIndexValid(i))
			{
				// Negative = overridden by the server (see CSI_SettingsManager)
				if (serverSettings.Get(i) < 0)
				{
					// Overridden by the server: show the forced value but don't bind it, so the player's own value isn't overwritten
					Widget widget = m_wRoot.FindAnyWidget(settingStr);
					if (widget)
					{
						SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
						SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(widget.FindHandler(SCR_ComboBoxComponent));
						SCR_SpinBoxComponent spinComp = SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));

						switch (true)
						{
							case (sliderComp) : sliderComp.SetValue(m_SettingsManager.GetSettingInt(settingStr)); break;
							case (comboComp) : comboComp.SetCurrentItem(m_SettingsManager.GetSettingInt(settingStr), false, true); break;
							case (spinComp) : spinComp.SetCurrentItem(m_SettingsManager.GetSettingInt(settingStr), false, true); break;
						}

						// Admins can still edit it, to change the override
						if (!SCR_Global.IsAdmin())
							widget.SetEnabled(false);
					}

					CheckBoxWidget checkBoxWidget = GetServerOverrideCheckBox(serverOverridesLayoutWidget, settingStr);
					if (checkBoxWidget)
						checkBoxWidget.SetChecked(true);
				} else
					setVanillaSettingBind = true;
			} else
				setVanillaSettingBind = true;

			if (setVanillaSettingBind)
				m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", settingStr, settingStr));
		}

		// Override checkboxes are admin only
		if (serverOverridesLayoutWidget && !SCR_Global.IsAdmin())
		{
			serverOverridesLayoutWidget.SetVisible(false);
			serverOverridesLayoutWidget.SetEnabled(false);
		}

		LoadSettings();

		Widget settingInfo = m_wRoot.FindAnyWidget("SettingInfo");
		if (settingInfo)
			m_SettingInfo = CSI_SettingInfo.Cast(settingInfo.FindHandler(CSI_SettingInfo));

		if (m_SettingInfo)
			m_SettingInfo.SetSettingsRoot(m_wRoot);

		Widget serverAdmin = m_wRoot.FindAnyWidget("ServerAdmin");
		if (serverAdmin)
			m_ServerAdmin = CSI_ServerAdmin.Cast(serverAdmin.FindHandler(CSI_ServerAdmin));

		if (m_ServerAdmin)
			m_ServerAdmin.SetSettingsRoot(m_wRoot);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		// Back on the tab after OnTabHide restored the player's own values: the widgets still show the defaults
		if (m_bShowingServerDefaults && m_SettingsManager)
			m_SettingsManager.ApplyServerDefaultsToUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! Saves the local settings JSON, sends admin changes to the server and recalculates the effective settings.
	override void OnTabHide()
	{
		super.OnTabHide();

		if (!m_SettingsManager || !m_RplToAuthorityManager || !m_PlayerControllerManager)
		{
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
			m_PlayerControllerManager = CSI_PlayerControllerManager.GetInstance();
		};

		// No player controller exists when settings are opened from the main menu
		if (m_PlayerControllerManager && m_PlayerControllerManager.GetLocalSettingsJson())
		{
			CSI_SettingsJson localSettingsJson = m_PlayerControllerManager.GetLocalSettingsJson();

			// Only save once the player has their own settings or changed something here, so a server's defaults
			// never end up in their file just from looking at this tab
			if (localSettingsJson.HasSavedSettings() || !m_aValuesOnOpen || !SCR_ArrayHelperT<int>.AreEqual(GetUserSettingValues(), m_aValuesOnOpen))
			{
				localSettingsJson.SaveToFile();
				m_bShowingServerDefaults = false;
			}
			else if (m_bShowingServerDefaults)
			{
				SetUserSettingValues(m_aValuesBeforeServerDefaults);
			}
		}
		else
		{
			CSI_SettingsJson settingsJson = new CSI_SettingsJson();
			settingsJson.SaveToFile();
		}

		if (m_SettingsManager && m_RplToAuthorityManager)
		{
			if (SCR_Global.IsAdmin())
			{
				// Send every setting: checked = override with the widget's value, unchecked = clear the override
				Widget serverOverridesLayoutWidget = m_wRoot.FindAnyWidget("ServerOverrides");
				foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
				{
					Widget contentWidget = m_wRoot.FindAnyWidget(settingStr);
					CheckBoxWidget checkBoxWidget = GetServerOverrideCheckBox(serverOverridesLayoutWidget, settingStr);
					if (!contentWidget || !checkBoxWidget)
						continue;

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

				if (m_ServerAdmin)
					m_ServerAdmin.SendMaxGroupSize();
			};

			m_SettingsManager.RequestSettingsUpdate();
		};
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] serverOverridesLayoutWidget the tab's ServerOverrides column
	//! \param[in] setting CSI_GameSettings name, also the name of its row in that column
	//! \return the setting's server override checkbox, null if the column or row is missing
	protected CheckBoxWidget GetServerOverrideCheckBox(Widget serverOverridesLayoutWidget, string setting)
	{
		if (!serverOverridesLayoutWidget)
			return null;

		Widget serverOverrideLayout = serverOverridesLayoutWidget.FindAnyWidget(setting);
		if (!serverOverrideLayout)
			return null;

		return CheckBoxWidget.Cast(serverOverrideLayout.FindAnyWidget("ServerOverride"));
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when a player controller exists (not the main menu) and the player has no saved settings
	protected bool IsInSessionWithoutSavedSettings()
	{
		return m_PlayerControllerManager && m_PlayerControllerManager.GetLocalSettingsJson() && !m_PlayerControllerManager.GetLocalSettingsJson().HasSavedSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! \return the CSI_GameSettings user settings values, by settings index
	protected TIntArray GetUserSettingValues()
	{
		TIntArray values = {};
		UserSettings userSettings = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
		if (!userSettings)
			return values;

		foreach (string settingStr : CSI_GameSettings.GetSettingsArray())
		{
			int value;
			userSettings.Get(settingStr, value);
			values.Insert(value);
		}

		return values;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] values by settings index, as returned by GetUserSettingValues
	protected void SetUserSettingValues(TIntArray values)
	{
		UserSettings userSettings = GetGame().GetGameUserSettings().GetModule("CSI_GameSettings");
		if (!userSettings || !values)
			return;

		foreach (int i, string settingStr : CSI_GameSettings.GetSettingsArray())
		{
			if (values.IsIndexValid(i))
				userSettings.Set(settingStr, values[i]);
		}
	}
}