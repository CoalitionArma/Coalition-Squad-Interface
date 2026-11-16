//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Server Admin panel of the CSI settings tab (admins only): max group size and server defaults.
//!
//------------------------------------------------------------------------------------------------
class CSI_ServerAdmin : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;

	protected Widget m_wSettingsRoot;	//!< CSI settings tab root
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

		// Layout editor: keep the panel visible so it can be edited
		if (SCR_Global.IsEditMode())
			return;

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();

		// Admins in a session only
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
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Sends the max group size if the admin changed it.
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
	//! Sends the tab's shown values as the server defaults.
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
