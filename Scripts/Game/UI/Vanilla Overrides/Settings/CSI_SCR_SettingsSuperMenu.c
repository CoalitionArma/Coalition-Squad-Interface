modded class SCR_SettingsSuperMenu {
	
    //------------------------------------------------------------------------------------------------
	override void OnMenuOpen() 
	{
		super.OnMenuOpen();

        if (!m_SuperMenuComponent || !m_SuperMenuComponent.GetTabView())
            return;

        m_SuperMenuComponent.GetTabView().AddTab("{2AA1FFAC7A556059}UI/layouts/Menus/SettingsSubMenus/CSI_SettingsSubMenu.layout", "Coalition Squad Interface", true, identifier: "CSI_GameSettings");
    };
};