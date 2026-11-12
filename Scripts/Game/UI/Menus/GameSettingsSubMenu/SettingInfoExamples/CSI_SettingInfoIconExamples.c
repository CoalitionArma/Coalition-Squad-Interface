//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Icon examples (CSI_SettingInfoIcons.layout) for the Icons settings: Icon0-Icon17, every CSI_EIcon but
//! 				EMPTY in enum order, drawn in the icon theme, type and arrow selected in the settings tab.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfoIconExamples : SCR_ScriptedWidgetComponent
{
	protected static const int ICON_COUNT = 18;

	protected Widget m_wSettingsRoot;	//!< CSI settings tab root, for the selected values
	protected ref array<Widget> m_aIcons = {};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		for (int iconIndex = 0; iconIndex < ICON_COUNT; iconIndex++)
		{
			Widget icon = w.FindAnyWidget("Icon" + iconIndex);
			if (icon)
				m_aIcons.Insert(icon);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] settingsRoot CSI settings tab root
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the example for a setting, if this layout has one.
	//! \param[in] setting CSI_GameSettings name
	//! \param[out] caption text under the example (may be empty)
	//! \return false if this layout has no example for the setting
	bool ShowExample(string setting, out string caption)
	{
		if (setting != CSI_GameSettings.ICON_THEME && setting != CSI_GameSettings.ICON_TYPE && setting != CSI_GameSettings.ARROW_THEME)
			return false;

		m_wRoot.SetVisible(true);
		foreach (int iconIndex, Widget icon : m_aIcons)
		{
			CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, 0, iconIndex, CSI_EColorTeam.NONE, 0, 0, 1);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void HideExample()
	{
		if (m_wRoot)
			m_wRoot.SetVisible(false);
	}
}
