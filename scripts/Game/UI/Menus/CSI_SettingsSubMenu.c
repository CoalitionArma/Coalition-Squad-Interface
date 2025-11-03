void ScriptInvoker_CSI_SettingsSubMenu(CSI_SettingsSubMenu menu);
typedef func ScriptInvoker_CSI_SettingsSubMenu;
typedef ScriptInvokerBase<ScriptInvoker_CSI_SettingsSubMenu> ScriptInvoker_CSI_SettingsSubMenuChanged;

class CSI_SettingsSubMenu: SCR_SettingsSubMenuBase
{	
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
		SCR_PlayerController.SetGameUserSettings();
		
		if (m_SettingsManager)
			m_SettingsManager.RequestSettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_aSettingsBindings.Clear();
		
		if (!m_SettingsManager || !m_RplToAuthorityManager)
		{
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		};
		
		foreach (int i, string settingStr : CSI_SettingsManager.GetSettingsArray())
		{
			bool settingOverriden = false;
			
			if (m_SettingsManager)
			{
				if (m_SettingsManager.GetServerSettingsArray().Get(i) < 0)
				{	// Overriden By Server, Grey out and dont insert so we arent overriding local player values with server overriden ones
					Widget widget = m_wRoot.FindAnyWidget(settingStr);
					
					SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
					SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(widget.FindHandler(SCR_ComboBoxComponent));
					SCR_SpinBoxComponent spinComp = SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));
					
					if (sliderComp)
						sliderComp.SetValue(m_SettingsManager.GetSettingInt(settingStr));
					
					if (comboComp)
						comboComp.SetCurrentItem(m_SettingsManager.GetSettingInt(settingStr), false, true);
					
					if (spinComp)
						spinComp.SetCurrentItem(m_SettingsManager.GetSettingInt(settingStr), false, true);
					
					widget.SetEnabled(false);
					settingOverriden = true
				}
			};
		
			if (!settingOverriden)
				m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", settingStr, settingStr));
		}

		LoadSettings();
	}
}