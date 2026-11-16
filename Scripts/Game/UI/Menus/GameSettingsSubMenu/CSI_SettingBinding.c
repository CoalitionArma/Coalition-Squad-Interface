//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Binds a CSI settings tab row to its CSI_GameSettings value, including toolboxes (the color rows).
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingBinding : SCR_SettingBindingGameplay
{
	//------------------------------------------------------------------------------------------------
	void CSI_SettingBinding(string module, string name, string widgetName, string subModule = string.Empty)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void LoadEntry(Widget parent, bool forceLoadSettings = false, bool addEntryChangedEventHandler = true)
	{
		super.LoadEntry(parent, forceLoadSettings, addEntryChangedEventHandler);

		if (!m_Setting || m_iItem < 0)
			return;

		// Checkboxes are toolboxes, already bound by super
		SCR_ToolboxComponent toolbox = SCR_ToolboxComponent.Cast(GetComponent(m_sWidgetName, parent));
		if (!toolbox || SCR_CheckboxComponent.Cast(toolbox))
			return;

		int current;
		m_Setting.Get(m_sName, current);
		toolbox.SetCurrentItem(current);
		m_Setting.GetEnumValues(m_iItem, m_aNames, m_aVals);

		if (addEntryChangedEventHandler)
			toolbox.m_OnChanged.Insert(OnSelectableChanged);
	}
}
