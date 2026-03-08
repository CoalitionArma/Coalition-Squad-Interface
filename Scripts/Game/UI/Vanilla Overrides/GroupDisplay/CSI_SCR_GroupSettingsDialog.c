modded class GroupSettingsDialogUI
{
	protected SCR_SliderComponent m_GroupSize;
	protected CSI_RplToAuthoritySystem m_RplToAuthoritySystem;

	//------------------------------------------------------------------------------------------------
	override void OnPrivilegeCallback(UserPrivilege privilege, UserPrivilegeResult result)
	{
		super.OnPrivilegeCallback(privilege, result);

		Widget w = GetRootWidget();
		if (!w)
			return;
		
		SCR_SliderComponent groupSize = SCR_SliderComponent.GetSliderComponent("GroupSizeSlider", w);
		if (!groupSize)
			return;
		
		SCR_AIGroup playerGroup = SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playerGroup)
			return;
		
		groupSize.SetValue(playerGroup.GetMaxMembers());
		m_RplToAuthoritySystem = CSI_RplToAuthoritySystem.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{	
		SCR_SliderComponent groupSize = SCR_SliderComponent.GetSliderComponent("GroupSizeSlider", GetRootWidget());
		if (!groupSize)
			return;
		
		if (m_RplToAuthoritySystem && groupSize)
			m_RplToAuthoritySystem.Owner_SetMaxGroupMembers(SCR_PlayerController.GetLocalPlayerId(), groupSize.GetValue());
		
		super.OnConfirm();
	}
}
