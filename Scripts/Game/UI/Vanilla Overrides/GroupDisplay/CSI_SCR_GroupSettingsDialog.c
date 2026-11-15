//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the group size slider shows the group's max members, up to the server's max group size;
//! 				confirming sends the new size to the server.
//! 
//------------------------------------------------------------------------------------------------
modded class GroupSettingsDialogUI
{
	protected SCR_SliderComponent m_GroupSize;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;

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

		CSI_SettingsManager settingsManager = CSI_SettingsManager.GetInstance();
		if (settingsManager)
			groupSize.SetMax(settingsManager.GetMaxGroupSize());

		groupSize.SetValue(playerGroup.GetMaxMembers());
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		SCR_SliderComponent groupSize = SCR_SliderComponent.GetSliderComponent("GroupSizeSlider", GetRootWidget());
		if (!groupSize)
			return;

		if (m_RplToAuthorityManager && groupSize)
			m_RplToAuthorityManager.Owner_SetMaxGroupMembers(SCR_PlayerController.GetLocalPlayerId(), groupSize.GetValue());

		super.OnConfirm();
	}
}
