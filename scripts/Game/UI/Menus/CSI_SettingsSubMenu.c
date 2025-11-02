void ScriptInvoker_CSI_SettingsSubMenu(CSI_SettingsSubMenu menu);
typedef func ScriptInvoker_CSI_SettingsSubMenu;
typedef ScriptInvokerBase<ScriptInvoker_CSI_SettingsSubMenu> ScriptInvoker_CSI_SettingsSubMenuChanged;

class CSI_SettingsSubMenu: SCR_SettingsSubMenuBase
{	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		SCR_PlayerController.SetGameUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_aSettingsBindings.Clear();

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iRankVisible", "RankVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iAutoHideHUD", "AutoHideHUD"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iStaminaVisible", "StaminaVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iGroupVisible", "GroupVisible"));
		
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iIconTheme", "IconTheme"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iIconType", "IconType"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iArrowTheme", "ArrowTheme"));
		
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iNametagVisible", "NametagVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iGroupInNametagVisible", "GroupInNametagVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iNametagLOSVisible", "NametagLOSVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iNametagPosition", "NametagPosition"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iNametagPositionOffset", "NametagPositionOffset"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iNametagRange", "NametagRange"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iNametagMagnificationMultiplication", "NametagMagnificationMultiplication"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iRoleIconInNametagVisible", "RoleIconInNametagVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iNametagRoleIconPosition", "NametagRoleIconPosition"));
		
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iCompassTheme", "CompassTheme"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iCompassVisible", "CompassVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iBearingVisible", "BearingVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iRadarVisible", "RadarVisible"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iRadarIconSize", "RadarIconSize"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("CSI_GameSettings", "m_iOnlyRadarIconArrowsRotate", "OnlyRadarIconArrowsRotate"));

		LoadSettings();
	}
}