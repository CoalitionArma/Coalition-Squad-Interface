class CSI_PlayerControllerManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Per-player component on the player controller: loads the local settings JSON and opens the CSI
//! 				settings (group menu for leaders, CSI_PlayerSettingsMenu otherwise). The server has one per player;
//! 				the local setup only runs for the local player's controller.
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

		// A listen server host runs this for every player's controller. Skip the ones that clearly aren't the host's:
		// reloading and saving the host's settings each time a player joins could save settings shown in the CSI tab.
		PlayerController localController = GetGame().GetPlayerController();
		if (localController && localController != owner)
			return;

		GetGame().GetInputManager().AddActionListener("CSI_PlayerSettingsMenu", EActionTrigger.DOWN, OpenLocalPlayerSettingsMenu);

		// Like vanilla SCR_PlayerControllerCommandingComponent: undo the local setup if this controller turns out to be
		// another player's (its ownership moves to them)
		SCR_PlayerController playerController = SCR_PlayerController.Cast(owner);
		if (playerController)
			playerController.GetOnOwnershipChangedInvoker().Insert(OnOwnershipChanged);

		// Apply the saved CSI settings, then let the settings manager merge in server overrides
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
		// Pairs with the listener added in OnPostInit (harmless on a dedicated server, where none was added)
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("CSI_PlayerSettingsMenu", EActionTrigger.DOWN, OpenLocalPlayerSettingsMenu);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Ownership moved to another player, so this isn't the local controller: removes the local setup from OnPostInit.
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
	//! CSI_PlayerSettingsMenu action: group leaders get the group menu, everyone else the player settings menu.
	protected void OpenLocalPlayerSettingsMenu()
	{
		// On a listen server every player's controller registers this listener; only the host's own should act
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
	//! \return the local player's instance (see CSI_RplToAuthorityManager.GetInstance)
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