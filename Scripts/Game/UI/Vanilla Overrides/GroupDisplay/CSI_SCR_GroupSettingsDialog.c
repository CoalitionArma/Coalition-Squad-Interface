//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: the group size slider shows the group's max members, up to the server's max group size;
//! 				confirming sends the size to the server if it was changed.
//! 
//------------------------------------------------------------------------------------------------
modded class GroupSettingsDialogUI
{
	protected SCR_SliderComponent m_GroupSize;	//!< set once the slider shows the group's size
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected float m_fGroupSizeAtSetup;		//!< slider value when it was set up

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

		m_GroupSize = groupSize;
		m_fGroupSizeAtSetup = groupSize.GetValue();
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// Only a changed size: the slider can't show unlimited groups or ones above the max
		if (m_GroupSize && m_RplToAuthorityManager && m_GroupSize.GetValue() != m_fGroupSizeAtSetup)
			m_RplToAuthorityManager.Owner_SetMaxGroupMembers(SCR_PlayerController.GetLocalPlayerId(), m_GroupSize.GetValue());

		super.OnConfirm();
	}
}
