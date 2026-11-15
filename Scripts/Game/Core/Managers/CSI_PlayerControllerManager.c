class CSI_PlayerControllerManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Player controller component: loads the local settings JSON and opens the CSI settings
//! 				(group menu for leaders, CSI_PlayerSettingsMenu otherwise).
//! 
//------------------------------------------------------------------------------------------------
class CSI_PlayerControllerManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref CSI_SettingsJson m_SettingsJson;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 MANAGER INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		// No local player or UI on a dedicated server
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		// Listen server host: skip other players' controllers
		PlayerController localController = GetGame().GetPlayerController();
		if (localController && localController != owner)
			return;

		GetGame().GetInputManager().AddActionListener("CSI_PlayerSettingsMenu", EActionTrigger.DOWN, OpenLocalPlayerSettingsMenu);

		// Undo the local setup if another player takes this controller
		SCR_PlayerController playerController = SCR_PlayerController.Cast(owner);
		if (playerController)
			playerController.GetOnOwnershipChangedInvoker().Insert(OnOwnershipChanged);

		// Load saved settings, then apply server overrides
		m_SettingsJson = new CSI_SettingsJson;
		m_SettingsJson.LoadFromFile();

		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();

		if (CSI_SettingsManager.GetInstance())
			CSI_SettingsManager.GetInstance().RequestSettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("CSI_PlayerSettingsMenu", EActionTrigger.DOWN, OpenLocalPlayerSettingsMenu);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes the local setup when another player takes this controller.
	//! \param[in] isChanging
	//! \param[in] becameOwner
	protected void OnOwnershipChanged(bool isChanging, bool becameOwner)
	{
		if (isChanging || becameOwner)
			return;

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("CSI_PlayerSettingsMenu", EActionTrigger.DOWN, OpenLocalPlayerSettingsMenu);

		m_SettingsJson = null;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTING METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \return null on a dedicated server
	CSI_SettingsJson GetLocalSettingsJson()
	{
		return m_SettingsJson;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 MENU METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Opens the group menu for group leaders, else CSI_PlayerSettingsMenu.
	protected void OpenLocalPlayerSettingsMenu()
	{
		// Listen server: only the host's own controller acts
		if (GetOwner() != GetGame().GetPlayerController())
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup group = groupsManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (group && group.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId()))
			GetGame().OpenGroupMenu();
		else
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CSI_PlayerSettingsMenu, 0, true);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected static CSI_PlayerControllerManager m_sInstance;	//!< last constructed instance, only a fallback
	//! \return the local player's instance
	static CSI_PlayerControllerManager GetInstance()
	{
		PlayerController localController = GetGame().GetPlayerController();
		if (localController)
		{
			CSI_PlayerControllerManager localInstance = CSI_PlayerControllerManager.Cast(localController.FindComponent(CSI_PlayerControllerManager));
			if (localInstance)
				return localInstance;
		}

		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	void CSI_PlayerControllerManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}