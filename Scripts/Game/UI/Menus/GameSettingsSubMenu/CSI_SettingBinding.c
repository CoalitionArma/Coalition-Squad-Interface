//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Binds a CSI settings tab row to its CSI_GameSettings value: vanilla SCR_SettingBindingGameplay, which loads
//! 				and saves spin boxes, sliders, checkboxes and combo boxes, plus plain toolboxes (the colour rows), bound
//! 				the way vanilla InitSpinBox binds a spin box.
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

		// Vanilla already binds its checkbox, which is a toolbox
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
