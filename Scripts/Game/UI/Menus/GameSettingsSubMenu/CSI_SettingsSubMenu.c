void ScriptInvoker_CSI_SettingsSubMenu(CSI_SettingsSubMenu menu);
typedef func ScriptInvoker_CSI_SettingsSubMenu;
typedef ScriptInvokerBase<ScriptInvoker_CSI_SettingsSubMenu> ScriptInvoker_CSI_SettingsSubMenuChanged;

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	CSI tab in Settings. Settings the server overrides show the forced value and are locked for non-admins;
//! 				admins get a server override checkbox per setting (sent when the tab closes). Settings in
//! 				CSI_GameSettings.SETTINGS_WITHOUT_SERVER_OVERRIDE have no checkbox and are never locked. The right side
//! 				holds two sub-layouts with their own components, driven from here like CSI_HUD drives its elements: Setting
//! 				Info for the hovered setting (CSI_SettingInfo) and, at the bottom, the admin-only Server Admin panel
//! 				(CSI_ServerAdmin). Players without saved settings see the server's defaults here; their settings file is
//! 				only written once they change something. Settings that only matter while a switch is on are nested under
//! 				it (RegisterDependentSettings): indented, joined to it by tree lines (DrawSettingsTree), and only shown
//! 				while every switch above them is on. Missing widgets are logged as warnings when the tab is created.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingsSubMenu: SCR_SettingsSubMenuBase
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	// Widgets in CSI_SettingsSubMenu.layout; each setting's row and server override row are named after the setting
	protected const string SETTINGS_LIST_WIDGET_NAME = "Content";				//!< the setting rows
	protected const string SETTINGS_TREE_WIDGET_NAME = "SettingsTree";			//!< canvas over the setting rows
	protected const string SERVER_OVERRIDES_WIDGET_NAME = "ServerOverrides";	//!< column of server override checkboxes
	protected const string SERVER_OVERRIDE_CHECKBOX_NAME = "ServerOverride";	//!< checkbox in each server override row
	protected const string SETTING_INFO_WIDGET_NAME = "SettingInfo";
	protected const string SERVER_ADMIN_WIDGET_NAME = "ServerAdmin";

	// Inside each setting row's label (vanilla WLib_Label.layout)
	protected const string LABEL_ROOT_WIDGET_NAME = "LabelRoot";
	protected const string LABEL_FRAME_WIDGET_NAME = "Frame0";				//!< frame around the text, padded to indent it
	protected const string LABEL_TEXT_WIDGET_NAME = "Text";

	protected const string SETTINGS_MODULE_NAME = "CSI_GameSettings";

	protected static const float NESTED_SETTING_INDENT = 24;		//!< px a nested setting's label moves right per nesting level

	protected static const float TREE_LINE_WIDTH = 2;				//!< px
	protected static const float TREE_LINE_INSET = 7;				//!< px from a switch's label start to its line
	protected static const float TREE_LINE_TOP_GAP = 3;				//!< px between a switch's label and the top of its line
	protected static const float TREE_BRANCH_GAP = 5;				//!< px between the end of a branch and the nested label
	protected static const int TREE_REDRAW_FRAMES = 5;				//!< frames the tree is redrawn after a change, while the rows move
	protected static const ref Color TREE_LINE_COLOR = new Color(1, 1, 1, 0.35);

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerControllerManager m_PlayerControllerManager;

	protected ref TIntArray m_aValuesOnOpen;				//!< user settings when the tab opened, to tell whether the player changed anything
	protected ref TIntArray m_aValuesBeforeServerDefaults;	//!< user settings before the server defaults were shown, restored if not saved
	protected bool m_bShowingServerDefaults;				//!< the player has no saved settings, so this tab shows the server's defaults

	// Components of the right side's sub-layouts, owned by their widgets
	protected CSI_SettingInfo m_SettingInfo;
	protected CSI_ServerAdmin m_ServerAdmin;

	// Nested settings (RegisterDependentSettings)
	protected ref map<string, string> m_mSettingParents;				//!< setting -> the on/off setting it's nested under
	protected ref array<SCR_SpinBoxComponent> m_aParentSpinBoxes = {};	//!< those on/off settings' rows, whose changes show or hide the others

	// Tree lines from each switch to its nested settings (DrawSettingsTree)
	protected CanvasWidget m_wSettingsTree;
	protected ref array<ref CanvasWidgetCommand> m_aSettingsTreeCommands = {};	//!< the tree's lines, kept while the canvas draws them
	protected int m_iSettingsTreeRedraws;										//!< frames left to redraw the tree

//=============================================================================================================================================================================================================================================================================================================================================================
//	 TAB LIFECYCLE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		FindManagers();

		// Players without saved settings get the server's defaults in game, so show (and let them edit) those here
		m_aValuesOnOpen = GetUserSettingValues();
		if (IsInSessionWithoutSavedSettings() && m_SettingsManager && m_SettingsManager.HasServerDefaults())
		{
			m_aValuesBeforeServerDefaults = m_aValuesOnOpen;
			m_SettingsManager.ApplyServerDefaultsToUserSettings();
			m_aValuesOnOpen = GetUserSettingValues();
			m_bShowingServerDefaults = true;
		}

		SetupSettingBindings();
		LoadSettings();

		SetupSidePanels();
		SetupNestedSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		m_iSettingsTreeRedraws = TREE_REDRAW_FRAMES;

		// Back on the tab after OnTabHide restored the player's own values: the widgets still show the defaults
		if (m_bShowingServerDefaults && m_SettingsManager)
			m_SettingsManager.ApplyServerDefaultsToUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! Saves the local settings JSON, sends admin changes to the server and recalculates the effective settings.
	override void OnTabHide()
	{
		super.OnTabHide();

		FindManagers();
		SaveLocalSettings();

		if (!m_SettingsManager || !m_RplToAuthorityManager)
			return;

		if (SCR_Global.IsAdmin())
		{
			SendServerOverrides();

			if (m_ServerAdmin)
				m_ServerAdmin.SendMaxGroupSize();
		}

		m_SettingsManager.RequestSettingsUpdate();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		foreach (SCR_SpinBoxComponent parentSpinBox : m_aParentSpinBoxes)
		{
			if (parentSpinBox)
				parentSpinBox.m_OnChanged.Remove(UpdateDependentSettingsVisibility);
		}

		m_aParentSpinBoxes.Clear();

		super.OnTabRemove();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		// Rows only move once the layout has updated after they're shown or hidden, so the tree is redrawn for a few frames
		if (m_iSettingsTreeRedraws > 0 && DrawSettingsTree())
			m_iSettingsTreeRedraws--;
	}

	//------------------------------------------------------------------------------------------------
	protected void FindManagers()
	{
		if (m_SettingsManager && m_RplToAuthorityManager && m_PlayerControllerManager)
			return;

		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_RplToAuthorityManager = CSI_RplToAuthorityManager.GetInstance();
		m_PlayerControllerManager = CSI_PlayerControllerManager.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the right side's Setting Info and Server Admin components and hands them the tab root.
	protected void SetupSidePanels()
	{
		Widget settingInfo = m_wRoot.FindAnyWidget(SETTING_INFO_WIDGET_NAME);
		if (settingInfo)
			m_SettingInfo = CSI_SettingInfo.Cast(settingInfo.FindHandler(CSI_SettingInfo));

		if (m_SettingInfo)
			m_SettingInfo.SetSettingsRoot(m_wRoot);
		else
			PrintFormat("CSI_SettingsSubMenu: '%1' with CSI_SettingInfo not found", SETTING_INFO_WIDGET_NAME, level: LogLevel.WARNING);

		Widget serverAdmin = m_wRoot.FindAnyWidget(SERVER_ADMIN_WIDGET_NAME);
		if (serverAdmin)
			m_ServerAdmin = CSI_ServerAdmin.Cast(serverAdmin.FindHandler(CSI_ServerAdmin));

		if (m_ServerAdmin)
			m_ServerAdmin.SetSettingsRoot(m_wRoot);
		else
			PrintFormat("CSI_SettingsSubMenu: '%1' with CSI_ServerAdmin not found", SERVER_ADMIN_WIDGET_NAME, level: LogLevel.WARNING);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTING BINDINGS AND SERVER OVERRIDES
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Binds each setting's row to its user setting, except settings the server overrides: those show the forced value
	//! unbound, so the player's own value isn't overwritten, and are locked for non-admins.
	protected void SetupSettingBindings()
	{
		m_aSettingsBindings.Clear();

		Widget serverOverrides = m_wRoot.FindAnyWidget(SERVER_OVERRIDES_WIDGET_NAME);
		if (!serverOverrides)
			PrintFormat("CSI_SettingsSubMenu: server override column '%1' not found", SERVER_OVERRIDES_WIDGET_NAME, level: LogLevel.WARNING);

		// The authority array can be empty before replication arrives, or shorter if the server runs an older version
		TIntArray serverSettings;
		if (m_SettingsManager)
			serverSettings = m_SettingsManager.GetServerSettingsArray();

		bool isAdmin = SCR_Global.IsAdmin();
		foreach (int i, string setting : CSI_GameSettings.GetSettingsArray())
		{
			Widget row = m_wRoot.FindAnyWidget(setting);
			if (!row)
				PrintFormat("CSI_SettingsSubMenu: row for setting '%1' not found", setting, level: LogLevel.WARNING);

			// Negative = overridden by the server (see CSI_SettingsManager); personal settings never are
			bool overridden = serverSettings && serverSettings.IsIndexValid(i) && serverSettings[i] < 0;
			if (!overridden || CSI_GameSettings.SETTINGS_WITHOUT_SERVER_OVERRIDE.Contains(setting))
			{
				m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay(SETTINGS_MODULE_NAME, setting, setting));
				continue;
			}

			if (row)
			{
				CSI_SettingInfo.SetWidgetValue(m_wRoot, setting, m_SettingsManager.GetSettingInt(setting));

				// Admins can still edit it, to change the override
				if (!isAdmin)
					row.SetEnabled(false);
			}

			CheckBoxWidget checkBox = GetServerOverrideCheckBox(serverOverrides, setting);
			if (checkBox)
				checkBox.SetChecked(true);
		}

		// Override checkboxes are admin only
		if (serverOverrides && !isAdmin)
		{
			serverOverrides.SetVisible(false);
			serverOverrides.SetEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sends every setting with an override checkbox: checked = override with the row's value, unchecked = clear it.
	protected void SendServerOverrides()
	{
		Widget serverOverrides = m_wRoot.FindAnyWidget(SERVER_OVERRIDES_WIDGET_NAME);
		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			CheckBoxWidget checkBox = GetServerOverrideCheckBox(serverOverrides, setting);
			int value;
			if (!checkBox || !CSI_SettingInfo.GetWidgetValue(m_wRoot, setting, value))
				continue;

			if (checkBox.IsChecked())
				m_RplToAuthorityManager.Owner_ChangeAuthoritySetting(setting, value, true);
			else
				m_RplToAuthorityManager.Owner_ChangeAuthoritySetting(setting, 0, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] serverOverrides the tab's server override column
	//! \param[in] setting CSI_GameSettings name, also the name of its row in that column
	//! \return the setting's server override checkbox, null if the column or row is missing or the setting has none
	protected CheckBoxWidget GetServerOverrideCheckBox(Widget serverOverrides, string setting)
	{
		if (!serverOverrides)
			return null;

		Widget overrideRow = serverOverrides.FindAnyWidget(setting);
		if (!overrideRow)
			return null;

		return CheckBoxWidget.Cast(overrideRow.FindAnyWidget(SERVER_OVERRIDE_CHECKBOX_NAME));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 NESTED SETTINGS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Nests the settings that only matter while a switch is on under it: indents their labels, follows the switches'
	//! rows as they change (as vanilla SCR_InterfaceSettingsSubMenu follows its master switch) and draws the tree.
	protected void SetupNestedSettings()
	{
		m_wSettingsTree = CanvasWidget.Cast(m_wRoot.FindAnyWidget(SETTINGS_TREE_WIDGET_NAME));
		if (!m_wSettingsTree)
			PrintFormat("CSI_SettingsSubMenu: tree canvas '%1' not found, nested settings won't have tree lines", SETTINGS_TREE_WIDGET_NAME, level: LogLevel.WARNING);

		RegisterDependentSettings();
		foreach (string dependentSetting, string parentSetting : m_mSettingParents)
		{
			IndentSettingLabel(dependentSetting, GetNestingDepth(dependentSetting));

			SCR_SpinBoxComponent parentSpinBox = SCR_SpinBoxComponent.GetSpinBoxComponent(parentSetting, m_wRoot);
			if (!parentSpinBox)
			{
				PrintFormat("CSI_SettingsSubMenu: switch '%1' has no spin box, '%2' won't follow it", parentSetting, dependentSetting, level: LogLevel.WARNING);
				continue;
			}

			if (m_aParentSpinBoxes.Contains(parentSpinBox))
				continue;

			parentSpinBox.m_OnChanged.Insert(UpdateDependentSettingsVisibility);
			m_aParentSpinBoxes.Insert(parentSpinBox);
		}

		UpdateDependentSettingsVisibility();
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_mSettingParents: each setting that only matters while an on/off setting above it is on, nested under it.
	//! A nested switch can have its own children (Role icons under Nametags). Add new nested
	//! settings here.
	protected void RegisterDependentSettings()
	{
		m_mSettingParents = new map<string, string>();

		TStringArray autoHideHud = {CSI_GameSettings.REVEAL_UI_MODE};
		NestSettings(CSI_GameSettings.AUTO_HIDE_HUD, autoHideHud);

		TStringArray nametags = {
			CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE,
			CSI_GameSettings.NAMETAG_LOS_VISIBLE,
			CSI_GameSettings.NAMETAG_POSITION,
			CSI_GameSettings.NAMETAG_POSITION_OFFSET,
			CSI_GameSettings.NAMETAG_RANGE,
			CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED,
			CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION,
			CSI_GameSettings.NAMETAG_FIELD_OF_VIEW,
			CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED,
			CSI_GameSettings.NAMTEAG_SCALE,
			CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE,
			CSI_GameSettings.NAMETAG_BRIGHTNESS_DIMMING};
		NestSettings(CSI_GameSettings.NAMETAG_VISIBLE, nametags);

		TStringArray roleIcon = {CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION};
		NestSettings(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE, roleIcon);

		TStringArray compass = {CSI_GameSettings.COMPASS_THEME};
		NestSettings(CSI_GameSettings.COMPASS_VISIBLE, compass);

		TStringArray radar = {
			CSI_GameSettings.RADAR_RANGE_ON_FOOT,
			CSI_GameSettings.RADAR_RANGE_IN_VEHICLE,
			CSI_GameSettings.RADAR_ICON_SIZE,
			CSI_GameSettings.RADAR_FIELD_OF_VIEW,
			CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE};
		NestSettings(CSI_GameSettings.RADAR_VISIBLE, radar);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] parentSetting on/off CSI_GameSettings name
	//! \param[in] dependentSettings settings only shown while it's on
	protected void NestSettings(string parentSetting, notnull TStringArray dependentSettings)
	{
		foreach (string dependentSetting : dependentSettings)
		{
			m_mSettingParents.Set(dependentSetting, parentSetting);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Shows each nested setting's row (and its place in the server override column, keeping the two columns aligned)
	//! only while every switch it's nested under is on. Called on each switch change.
	protected void UpdateDependentSettingsVisibility()
	{
		if (!m_mSettingParents)
			return;

		Widget serverOverrides = m_wRoot.FindAnyWidget(SERVER_OVERRIDES_WIDGET_NAME);
		foreach (string dependentSetting, string parentSetting : m_mSettingParents)
		{
			bool shown = IsSettingShown(dependentSetting);

			// The settings list comes before the override column, so this finds the setting's own row
			Widget row = m_wRoot.FindAnyWidget(dependentSetting);
			if (row)
				row.SetVisible(shown);

			if (!serverOverrides)
				continue;

			Widget overrideRow = serverOverrides.FindAnyWidget(dependentSetting);
			if (overrideRow)
				overrideRow.SetVisible(shown);
		}

		m_iSettingsTreeRedraws = TREE_REDRAW_FRAMES;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \return true if the setting's row should show: not nested, or its switch is shown and on
	protected bool IsSettingShown(string setting)
	{
		string parentSetting;
		if (!m_mSettingParents.Find(setting, parentSetting))
			return true;

		return IsSettingShown(parentSetting) && IsSettingRowOn(parentSetting);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting an on/off CSI_GameSettings name
	//! \return true if the setting's row shows on (its shown value, so a server-forced one counts too)
	protected bool IsSettingRowOn(string setting)
	{
		int value;
		if (CSI_SettingInfo.GetWidgetValue(m_wRoot, setting, value))
			return value > 0;

		return m_SettingsManager && m_SettingsManager.GetSettingBool(setting);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \return how many switches the setting is nested under (0 for top-level settings)
	protected int GetNestingDepth(string setting)
	{
		int depth;
		string parentSetting;
		while (m_mSettingParents.Find(setting, parentSetting))
		{
			depth++;
			setting = parentSetting;
		}

		return depth;
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a setting's label right by NESTED_SETTING_INDENT per nesting level. The padding goes on the text's frame
	//! inside the label, not on the label itself, so the label keeps its share of the row and the setting's control
	//! stays lined up with the others.
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] depth nesting level
	protected void IndentSettingLabel(string setting, int depth)
	{
		Widget labelFrame;
		Widget labelRoot = GetLabelRoot(m_wRoot.FindAnyWidget(setting));
		if (labelRoot)
			labelFrame = labelRoot.FindAnyWidget(LABEL_FRAME_WIDGET_NAME);

		if (!labelFrame)
		{
			PrintFormat("CSI_SettingsSubMenu: label of nested setting '%1' not found, it won't be indented", setting, level: LogLevel.WARNING);
			return;
		}

		AlignableSlot.SetPadding(labelFrame, depth * NESTED_SETTING_INDENT, 0, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] row setting row
	//! \return the row's label (vanilla WLib_Label root), null if the row or label is missing
	protected Widget GetLabelRoot(Widget row)
	{
		if (!row)
			return null;

		return row.FindAnyWidget(LABEL_ROOT_WIDGET_NAME);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTINGS TREE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Draws the tree lines on the SettingsTree canvas, like a file tree: from under each switch's label a line runs down
	//! to its last shown nested setting, with a branch across to each nested label. Positions are measured from the labels
	//! on screen, relative to the canvas, which scrolls with the settings list. Lines on canvases as in vanilla SCR_SpiderNet.
	//! \return false if the list isn't laid out yet
	protected bool DrawSettingsTree()
	{
		if (!m_wSettingsTree || !m_mSettingParents)
			return true;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget settingsList = m_wRoot.FindAnyWidget(SETTINGS_LIST_WIDGET_NAME);
		if (!workspace || !settingsList)
			return true;

		float canvasX, canvasY, canvasWidth, canvasHeight;
		m_wSettingsTree.GetScreenPos(canvasX, canvasY);
		m_wSettingsTree.GetScreenSize(canvasWidth, canvasHeight);
		if (canvasWidth <= 0 || canvasHeight <= 0)
			return false;

		map<string, ref array<Widget>> branches = GetShownNestedRows(settingsList);

		float lineWidth = workspace.DPIScale(TREE_LINE_WIDTH);
		float inset = workspace.DPIScale(TREE_LINE_INSET);
		float topGap = workspace.DPIScale(TREE_LINE_TOP_GAP);
		float branchGap = workspace.DPIScale(TREE_BRANCH_GAP);
		int color = TREE_LINE_COLOR.PackToInt();

		m_aSettingsTreeCommands.Clear();
		foreach (string parentSetting, array<Widget> nestedRows : branches)
		{
			float parentX, parentY, parentWidth, parentHeight;
			if (!GetLabelTextRect(m_wRoot.FindAnyWidget(parentSetting), parentX, parentY, parentWidth, parentHeight))
				continue;

			float lineX = parentX - canvasX + inset;
			float lineTop = parentY - canvasY + parentHeight + topGap;
			float lineBottom = lineTop;

			foreach (Widget nestedRow : nestedRows)
			{
				float labelX, labelY, labelWidth, labelHeight;
				if (!GetLabelTextRect(nestedRow, labelX, labelY, labelWidth, labelHeight))
					continue;

				float branchY = labelY - canvasY + labelHeight * 0.5;
				float branchEnd = labelX - canvasX - branchGap;
				if (branchEnd > lineX)
					AddSettingsTreeLine(lineX, branchY, branchEnd, branchY, lineWidth, color);

				lineBottom = branchY;
			}

			if (lineBottom > lineTop)
				AddSettingsTreeLine(lineX, lineTop, lineX, lineBottom, lineWidth, color);
		}

		m_wSettingsTree.SetDrawCommands(m_aSettingsTreeCommands);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] settingsList the settings list widget, whose children are the rows in the order shown
	//! \return each switch's shown nested rows, in that order
	protected map<string, ref array<Widget>> GetShownNestedRows(notnull Widget settingsList)
	{
		map<string, ref array<Widget>> branches = new map<string, ref array<Widget>>();

		Widget row = settingsList.GetChildren();
		while (row)
		{
			string parentSetting;
			if (row.IsVisible() && m_mSettingParents.Find(row.GetName(), parentSetting))
			{
				array<Widget> nestedRows = branches.Get(parentSetting);
				if (!nestedRows)
				{
					nestedRows = {};
					branches.Set(parentSetting, nestedRows);
				}

				nestedRows.Insert(row);
			}

			row = row.GetSibling();
		}

		return branches;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] fromX
	//! \param[in] fromY
	//! \param[in] toX
	//! \param[in] toY
	//! \param[in] width
	//! \param[in] color packed
	protected void AddSettingsTreeLine(float fromX, float fromY, float toX, float toY, float width, int color)
	{
		LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_Vertices = {fromX, fromY, toX, toY};
		m_aSettingsTreeCommands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] row setting row
	//! \param[out] x screen position of the row's label text
	//! \param[out] y
	//! \param[out] width screen size of the row's label text
	//! \param[out] height
	//! \return false if the row or its label text is missing, or not laid out yet
	protected bool GetLabelTextRect(Widget row, out float x, out float y, out float width, out float height)
	{
		Widget labelRoot = GetLabelRoot(row);
		if (!labelRoot)
			return false;

		Widget text = labelRoot.FindAnyWidget(LABEL_TEXT_WIDGET_NAME);
		if (!text)
			return false;

		text.GetScreenPos(x, y);
		text.GetScreenSize(width, height);
		return height > 0;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 USER SETTINGS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Writes the local settings JSON, but only once the player has their own settings or changed something here, so a
	//! server's defaults never end up in their file just from looking at this tab.
	protected void SaveLocalSettings()
	{
		// No player controller exists when settings are opened from the main menu
		if (!m_PlayerControllerManager || !m_PlayerControllerManager.GetLocalSettingsJson())
		{
			CSI_SettingsJson settingsJson = new CSI_SettingsJson();
			settingsJson.SaveToFile();
			return;
		}

		CSI_SettingsJson localSettingsJson = m_PlayerControllerManager.GetLocalSettingsJson();
		if (localSettingsJson.HasSavedSettings() || !m_aValuesOnOpen || !SCR_ArrayHelperT<int>.AreEqual(GetUserSettingValues(), m_aValuesOnOpen))
		{
			localSettingsJson.SaveToFile();
			m_bShowingServerDefaults = false;
		}
		else if (m_bShowingServerDefaults)
		{
			SetUserSettingValues(m_aValuesBeforeServerDefaults);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when a player controller exists (not the main menu) and the player has no saved settings
	protected bool IsInSessionWithoutSavedSettings()
	{
		return m_PlayerControllerManager && m_PlayerControllerManager.GetLocalSettingsJson() && !m_PlayerControllerManager.GetLocalSettingsJson().HasSavedSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! \return the CSI_GameSettings user settings values, by settings index
	protected TIntArray GetUserSettingValues()
	{
		TIntArray values = {};
		UserSettings userSettings = GetGame().GetGameUserSettings().GetModule(SETTINGS_MODULE_NAME);
		if (!userSettings)
			return values;

		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			int value;
			userSettings.Get(setting, value);
			values.Insert(value);
		}

		return values;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] values by settings index, as returned by GetUserSettingValues
	protected void SetUserSettingValues(TIntArray values)
	{
		UserSettings userSettings = GetGame().GetGameUserSettings().GetModule(SETTINGS_MODULE_NAME);
		if (!userSettings || !values)
			return;

		foreach (int i, string setting : CSI_GameSettings.GetSettingsArray())
		{
			if (values.IsIndexValid(i))
				userSettings.Set(setting, values[i]);
		}
	}
}
