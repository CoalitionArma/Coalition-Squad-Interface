//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Setting Info example for the icon settings: every CSI_EIcon (Icon0-Icon17, in enum order), drawn
//! 				with the tab's icon theme, type and arrow.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfoIconExamples : SCR_ScriptedWidgetComponent
{
	protected static const int ICON_COUNT = 18;

	protected Widget m_wSettingsRoot;	//!< CSI settings tab root
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
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[out] caption text under the example
	//! \return false if this layout has no example for it
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
