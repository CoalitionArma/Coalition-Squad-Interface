//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Server Admin panel (CSI_ServerAdmin.layout) at the bottom right of the CSI settings tab, for admins
//! 				in a session: max group size and the server's default settings. CSI_SettingsSubMenu finds it and
//! 				drives it, like CSI_HUD does its elements.
//!
//------------------------------------------------------------------------------------------------
class CSI_ServerAdmin : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;

	protected Widget m_wSettingsRoot;	//!< CSI settings tab root, to read the values saved as server defaults
	protected SCR_SliderComponent m_MaxGroupSizeSlider;
	protected SCR_ButtonBaseComponent m_SaveDefaultsButton;
	protected SCR_ButtonBaseComponent m_ClearDefaultsButton;
	protected TextWidget m_wServerDefaultsStatus;

	protected int m_iSentMaxGroupSize;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Workbench's layout editor runs this too: keep the panel visible there so it can be edited
		if (SCR_Global.IsEditMode())
			return;

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();

		// Admins only, and there's no server to change from the main menu
		if (!SCR_Global.IsAdmin() || !m_SettingsManager || !m_RplToAuthorityManager)
		{
			w.SetVisible(false);
			w.SetEnabled(false);
			return;
		}

		m_MaxGroupSizeSlider = SCR_SliderComponent.GetSliderComponent("MaxGroupSize", w);
		if (m_MaxGroupSizeSlider)
		{
			m_iSentMaxGroupSize = m_SettingsManager.GetMaxGroupSize();
			m_MaxGroupSizeSlider.SetValue(m_iSentMaxGroupSize, false);
		}

		m_SaveDefaultsButton = SCR_ButtonBaseComponent.GetButtonBase("SaveServerDefaults", w);
		if (m_SaveDefaultsButton)
			m_SaveDefaultsButton.m_OnClicked.Insert(OnSaveServerDefaultsClicked);

		m_ClearDefaultsButton = SCR_ButtonBaseComponent.GetButtonBase("ClearServerDefaults", w);
		if (m_ClearDefaultsButton)
			m_ClearDefaultsButton.m_OnClicked.Insert(OnClearServerDefaultsClicked);

		m_wServerDefaultsStatus = TextWidget.Cast(w.FindAnyWidget("ServerDefaultsStatus"));
		SetServerDefaultsStatus(m_SettingsManager.HasServerDefaults());
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_SaveDefaultsButton)
			m_SaveDefaultsButton.m_OnClicked.Remove(OnSaveServerDefaultsClicked);

		if (m_ClearDefaultsButton)
			m_ClearDefaultsButton.m_OnClicked.Remove(OnClearServerDefaultsClicked);

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] settingsRoot CSI settings tab root, where the setting widgets are
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Sends the max group size to the server if the admin changed it. Called when the tab closes.
	void SendMaxGroupSize()
	{
		if (!m_MaxGroupSizeSlider || !m_RplToAuthorityManager)
			return;

		int maxGroupSize = m_MaxGroupSizeSlider.GetValue();
		if (maxGroupSize == m_iSentMaxGroupSize)
			return;

		m_RplToAuthorityManager.Owner_SetServerMaxGroupSize(maxGroupSize);
		m_iSentMaxGroupSize = maxGroupSize;
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the values currently shown in the tab to the server as its default settings.
	protected void OnSaveServerDefaultsClicked()
	{
		if (!m_RplToAuthorityManager || !m_wSettingsRoot)
			return;

		foreach (string settingStr : CSI_GameSettings.GetSettingsArray())
		{
			int value;
			if (CSI_SettingInfo.GetWidgetValue(m_wSettingsRoot, settingStr, value))
				m_RplToAuthorityManager.Owner_SetServerDefault(settingStr, value);
		}

		SetServerDefaultsStatus(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClearServerDefaultsClicked()
	{
		if (!m_RplToAuthorityManager)
			return;

		m_RplToAuthorityManager.Owner_ClearServerDefaults();
		SetServerDefaultsStatus(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hasDefaults
	protected void SetServerDefaultsStatus(bool hasDefaults)
	{
		if (!m_wServerDefaultsStatus)
			return;

		if (hasDefaults)
			m_wServerDefaultsStatus.SetText("Server defaults: saved");
		else
			m_wServerDefaultsStatus.SetText("Server defaults: none");
	}
}
