//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	CSI tab in Settings: server overrides (set by admins, locked for others), server defaults for players
//! 				without saved settings, settings nested under their switch, and the Setting Info and Server Admin panels.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingsSubMenu: SCR_SettingsSubMenuBase
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	// Widgets in CSI_SettingsSubMenu.layout; rows are named after their setting
	protected const string SETTINGS_LIST_WIDGET_NAME = "Content";				//!< the setting rows
	protected const string SETTINGS_TREE_WIDGET_NAME = "SettingsTree";			//!< canvas over the setting rows
	protected const string SERVER_OVERRIDES_WIDGET_NAME = "ServerOverrides";	//!< column of server override checkboxes
	protected const string SERVER_OVERRIDE_CHECKBOX_NAME = "ServerOverride";	//!< checkbox in each server override row
	protected const string SETTING_INFO_WIDGET_NAME = "SettingInfo";
	protected const string SERVER_ADMIN_WIDGET_NAME = "ServerAdmin";
	protected const string NAMETAG_LOOK_SEPARATOR_NAME = "SeparatorNametagLook";	//!< line above the nametag look block, in both columns

	// Inside each setting row's label (WLib_Label.layout)
	protected const string LABEL_ROOT_WIDGET_NAME = "LabelRoot";
	protected const string LABEL_FRAME_WIDGET_NAME = "Frame0";				//!< frame around the text, padded to indent it
	protected const string LABEL_TEXT_WIDGET_NAME = "Text";

	protected const string SETTINGS_MODULE_NAME = "CSI_GameSettings";

	protected static const float NESTED_SETTING_INDENT = 24;		//!< px per nesting level

	protected static const float TREE_LINE_WIDTH = 2;				//!< px
	protected static const float TREE_LINE_INSET = 7;				//!< px from a switch's label start to its line
	protected static const float TREE_LINE_TOP_GAP = 3;				//!< px between a switch's label and its line
	protected static const float TREE_BRANCH_GAP = 5;				//!< px between a branch and its nested label
	protected static const int TREE_REDRAW_FRAMES = 5;				//!< frames to redraw the tree after a change
	protected static const ref Color TREE_LINE_COLOR = new Color(1, 1, 1, 0.35);

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_RplToAuthorityManager m_RplToAuthorityManager;
	protected CSI_PlayerControllerManager m_PlayerControllerManager;

	protected ref TIntArray m_aValuesOnOpen;				//!< user settings when the tab opened, to detect changes
	protected ref TIntArray m_aValuesBeforeServerDefaults;	//!< user settings before showing server defaults
	protected bool m_bShowingServerDefaults;

	// Server override rows as loaded or last sent, so only changes are sent (SendServerOverrides)
	protected ref map<string, bool> m_mSentServerOverrides = new map<string, bool>();	//!< setting -> checkbox checked
	protected ref map<string, int> m_mSentServerOverrideValues = new map<string, int>();	//!< setting -> row value
	protected ref array<string> m_aServerForcedSettings = {};	//!< rows showing a server override, not the player's own value

	// Rows whose changes cross out clashing colors (UpdateBlockedColors)
	protected SCR_ComboBoxComponent m_ColorTeamSetComboBox;
	protected CSI_ColorPickerComponent m_VONColorPicker;

	// Right side panels, owned by their widgets
	protected CSI_SettingInfo m_SettingInfo;
	protected CSI_ServerAdmin m_ServerAdmin;

	// Nested settings (RegisterDependentSettings)
	protected ref map<string, string> m_mSettingParents;				//!< setting -> its switch
	protected ref map<string, string> m_mSeparatorParents;				//!< line between nested blocks -> the switch it hides with
	protected ref array<SCR_SpinBoxComponent> m_aParentSpinBoxes = {};	//!< the switches' spin boxes

	// Tree lines from each switch to its nested settings (DrawSettingsTree)
	protected CanvasWidget m_wSettingsTree;
	protected ref array<ref CanvasWidgetCommand> m_aSettingsTreeCommands = {};	//!< kept while the canvas draws them
	protected int m_iSettingsTreeRedraws;										//!< frames left to redraw the tree
	protected float m_fSettingsTreeWidth;										//!< canvas screen size at the last draw
	protected float m_fSettingsTreeHeight;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 TAB LIFECYCLE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		FindManagers();

		// Players without saved settings see (and can edit) the server's defaults
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
		SnapshotServerOverrides();

		SetupSidePanels();
		SetupNestedSettings();
		SetupBlockedColors();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		m_iSettingsTreeRedraws = TREE_REDRAW_FRAMES;

		// OnTabHide restored the player's values; the widgets still show the defaults
		if (m_bShowingServerDefaults && m_SettingsManager)
			m_SettingsManager.ApplyServerDefaultsToUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! Saves local settings, sends admin changes and updates the effective settings.
	override void OnTabHide()
	{
		// Closing the menu hides every tab, also ones a tab switch already hid
		if (!GetShown())
		{
			super.OnTabHide();
			return;
		}

		// Before super saves the user settings, so it saves the player's own values
		FindManagers();
		SaveLocalSettings();

		super.OnTabHide();

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

		if (m_ColorTeamSetComboBox)
			m_ColorTeamSetComboBox.m_OnChanged.Remove(OnColorTeamSetChanged);

		if (m_VONColorPicker)
			m_VONColorPicker.m_OnChanged.Remove(OnVONColorPicked);

		super.OnTabRemove();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		// The rows move when the screen size or aspect ratio changes
		if (HasSettingsTreeResized())
			m_iSettingsTreeRedraws = TREE_REDRAW_FRAMES;

		// Redraw for a few frames: rows only move once the layout updates
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
	//! Finds the Setting Info and Server Admin panels and hands them the tab root.
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
	//! Crosses out colors that clash, now and whenever the fire team set or the Nametag VON color changes.
	protected void SetupBlockedColors()
	{
		m_ColorTeamSetComboBox = SCR_ComboBoxComponent.GetComboBoxComponent(CSI_GameSettings.COLOR_TEAM_SET, m_wRoot);
		if (m_ColorTeamSetComboBox)
			m_ColorTeamSetComboBox.m_OnChanged.Insert(OnColorTeamSetChanged);
		else
			PrintFormat("CSI_SettingsSubMenu: '%1' has no combo box, colors that match a fire team color won't be crossed out", CSI_GameSettings.COLOR_TEAM_SET, level: LogLevel.WARNING);

		m_VONColorPicker = CSI_ColorPickerComponent.GetColorPicker(CSI_GameSettings.NAMETAG_VON_COLOR, m_wRoot);
		if (m_VONColorPicker)
			m_VONColorPicker.m_OnChanged.Insert(OnVONColorPicked);
		else
			PrintFormat("CSI_SettingsSubMenu: '%1' has no color picker, other nametag colors won't avoid the talking color", CSI_GameSettings.NAMETAG_VON_COLOR, level: LogLevel.WARNING);

		UpdateBlockedColors(GetShownColorTeamSet(), GetColorSettingValue(CSI_GameSettings.NAMETAG_VON_COLOR));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnColorTeamSetChanged(SCR_ComboBoxComponent comboBox, int teamSet)
	{
		UpdateBlockedColors(teamSet, GetColorSettingValue(CSI_GameSettings.NAMETAG_VON_COLOR));
	}

	//------------------------------------------------------------------------------------------------
	//! Resets the colors (VON_EXCLUSIVE_COLOR_SETTINGS) that use the new talking color to their defaults.
	//! \param[in] colorPicker
	//! \param[in] vonColor the picked CSI_EColorPreset; the picker reports it before selecting it
	protected void OnVONColorPicked(SCR_ToolboxComponent colorPicker, int vonColor)
	{
		int teamSet = GetShownColorTeamSet();
		BaseContainer userSettings = GetGame().GetGameUserSettings().GetModule(SETTINGS_MODULE_NAME);
		foreach (string setting : CSI_GameSettings.VON_EXCLUSIVE_COLOR_SETTINGS)
		{
			CSI_ColorPickerComponent otherPicker = CSI_ColorPickerComponent.GetColorPicker(setting, m_wRoot);
			if (!otherPicker || !otherPicker.IsEnabled() || m_aServerForcedSettings.Contains(setting) || otherPicker.GetCurrentIndex() != vonColor)
				continue;

			int resetColor = GetDefaultColor(setting, teamSet, vonColor);

			// Written directly, so Setting Info stays on the VON row
			if (userSettings)
				userSettings.Set(setting, resetColor);

			otherPicker.SetCurrentItem(resetColor);
		}

		UpdateBlockedColors(teamSet, vonColor);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting a CSI_GameSettings.VON_EXCLUSIVE_COLOR_SETTINGS name
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \param[in] vonColor CSI_EColorPreset value
	//! \return the setting's default color, or the first color that clashes with neither if the default does
	protected int GetDefaultColor(string setting, int teamSet, int vonColor)
	{
		int defaultColor = -1;
		string defaultValue;
		BaseContainer userSettings = GetGame().GetGameUserSettings().GetModule(SETTINGS_MODULE_NAME);
		if (userSettings && userSettings.GetDefaultAsString(setting, defaultValue))
			defaultColor = defaultValue.ToInt();

		if (defaultColor >= 0 && defaultColor != vonColor && !CSI_ColorPresets.ClashesWithColorTeams(defaultColor, teamSet))
			return defaultColor;

		for (int preset, count = CSI_ColorPresets.Count(); preset < count; preset++)
		{
			if (preset != vonColor && !CSI_ColorPresets.ClashesWithColorTeams(preset, teamSet))
				return preset;
		}

		return CSI_EColorPreset.GRAY;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the fire team set shown in the tab, CLASSIC without its row
	protected int GetShownColorTeamSet()
	{
		if (!m_ColorTeamSetComboBox)
			return CSI_EColorTeamSet.CLASSIC;

		return m_ColorTeamSetComboBox.GetCurrentIndex();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting a CSI_GameSettings.NON_TEAM_COLOR_SETTINGS name
	//! \return the forced value on an override row, else the player's saved value; -1 if unknown
	protected int GetColorSettingValue(string setting)
	{
		if (m_aServerForcedSettings.Contains(setting))
		{
			CSI_ColorPickerComponent colorPicker = CSI_ColorPickerComponent.GetColorPicker(setting, m_wRoot);
			if (colorPicker)
				return colorPicker.GetCurrentIndex();
		}

		int value = -1;
		BaseContainer userSettings = GetGame().GetGameUserSettings().GetModule(SETTINGS_MODULE_NAME);
		if (userSettings)
			userSettings.Get(setting, value);

		return value;
	}

	//------------------------------------------------------------------------------------------------
	//! Crosses out the color boxes (NON_TEAM_COLOR_SETTINGS) that match the set's fire team colors; in
	//! VON_EXCLUSIVE_COLOR_SETTINGS the talking color; and in the VON row the colors server-forced
	//! VON_EXCLUSIVE_COLOR_SETTINGS are drawn in. Each row shows the color it's drawn in: Gray for a color that
	//! matches a fire team, CSI_ColorPresets.GetVONPreset for the VON row. The player's own pick stays saved and
	//! comes back once it can be used.
	//! \param[in] teamSet CSI_EColorTeamSet value
	//! \param[in] vonColor the Nametag VON color setting's value (CSI_EColorPreset), -1 for none
	protected void UpdateBlockedColors(int teamSet, int vonColor)
	{
		int standardSquadColor = CSI_ColorPresets.GetNonClashingPreset(GetColorSettingValue(CSI_GameSettings.STANDARD_SQUAD_COLOR), teamSet);
		int nametagColor = CSI_ColorPresets.GetNonClashingPreset(GetColorSettingValue(CSI_GameSettings.NAMETAG_COLOR), teamSet);

		int shownVONColor = -1;
		if (vonColor >= 0)
			shownVONColor = CSI_ColorPresets.GetVONPreset(vonColor, teamSet, standardSquadColor, nametagColor);

		// The VON row can't take a forced row's color; a player's own row is reset instead (OnVONColorPicked)
		int forcedStandardSquadColor = -1;
		if (m_aServerForcedSettings.Contains(CSI_GameSettings.STANDARD_SQUAD_COLOR))
			forcedStandardSquadColor = standardSquadColor;

		int forcedNametagColor = -1;
		if (m_aServerForcedSettings.Contains(CSI_GameSettings.NAMETAG_COLOR))
			forcedNametagColor = nametagColor;

		BaseContainer userSettings = GetGame().GetGameUserSettings().GetModule(SETTINGS_MODULE_NAME);
		foreach (string setting : CSI_GameSettings.NON_TEAM_COLOR_SETTINGS)
		{
			// Disabled when the server forces it (non-admins)
			CSI_ColorPickerComponent colorPicker = CSI_ColorPickerComponent.GetColorPicker(setting, m_wRoot);
			if (!colorPicker || !colorPicker.IsEnabled())
				continue;

			bool isVON = setting == CSI_GameSettings.NAMETAG_VON_COLOR;
			bool avoidsVON = CSI_GameSettings.VON_EXCLUSIVE_COLOR_SETTINGS.Contains(setting);
			for (int preset, count = CSI_ColorPresets.Count(); preset < count; preset++)
			{
				bool blocked = CSI_ColorPresets.ClashesWithColorTeams(preset, teamSet)
					|| (avoidsVON && preset == shownVONColor)
					|| (isVON && (preset == forcedStandardSquadColor || preset == forcedNametagColor));
				colorPicker.SetColorBlocked(preset, blocked);
			}

			// An override row keeps showing the forced value
			if (m_aServerForcedSettings.Contains(setting))
				continue;

			int value;
			if (isVON)
			{
				if (shownVONColor >= 0)
					colorPicker.SetCurrentItem(shownVONColor);
			}
			else if (userSettings && userSettings.Get(setting, value))
			{
				colorPicker.SetCurrentItem(CSI_ColorPresets.GetNonClashingPreset(value, teamSet));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Binds rows to user settings. Overridden rows show the forced value unbound, keeping the player's own.
	protected void SetupSettingBindings()
	{
		m_aSettingsBindings.Clear();
		m_aServerForcedSettings.Clear();

		Widget serverOverrides = m_wRoot.FindAnyWidget(SERVER_OVERRIDES_WIDGET_NAME);
		if (!serverOverrides)
			PrintFormat("CSI_SettingsSubMenu: server override column '%1' not found", SERVER_OVERRIDES_WIDGET_NAME, level: LogLevel.WARNING);

		// May be empty before replication, or shorter on an older server version
		TIntArray serverSettings;
		if (m_SettingsManager)
			serverSettings = m_SettingsManager.GetServerSettingsArray();

		bool isAdmin = SCR_Global.IsAdmin();
		foreach (int i, string setting : CSI_GameSettings.GetSettingsArray())
		{
			Widget row = m_wRoot.FindAnyWidget(setting);
			if (!row)
				PrintFormat("CSI_SettingsSubMenu: row for setting '%1' not found", setting, level: LogLevel.WARNING);

			// Negative = server override (CSI_SettingsManager); personal settings have none
			bool overridden = serverSettings && serverSettings.IsIndexValid(i) && serverSettings[i] < 0;
			if (!overridden || CSI_GameSettings.SETTINGS_WITHOUT_SERVER_OVERRIDE.Contains(setting))
			{
				// Also binds the color rows' toolboxes
				m_aSettingsBindings.Insert(new CSI_SettingBinding(SETTINGS_MODULE_NAME, setting, setting));
				continue;
			}

			m_aServerForcedSettings.Insert(setting);

			if (row)
			{
				CSI_SettingInfo.SetWidgetValue(m_wRoot, setting, m_SettingsManager.GetSettingInt(setting));

				// Admins can still edit it, to change the override
				if (!isAdmin)
				{
					// Through the row's component, which also dims it
					SCR_WLibComponentBase rowComponent = SCR_WLibComponentBase.Cast(row.FindHandler(SCR_WLibComponentBase));
					if (rowComponent)
						rowComponent.SetEnabled(false, false);
					else
						row.SetEnabled(false);
				}
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
	//! Remembers each override checkbox and its row's value, for SendServerOverrides.
	protected void SnapshotServerOverrides()
	{
		m_mSentServerOverrides.Clear();
		m_mSentServerOverrideValues.Clear();

		Widget serverOverrides = m_wRoot.FindAnyWidget(SERVER_OVERRIDES_WIDGET_NAME);
		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			CheckBoxWidget checkBox = GetServerOverrideCheckBox(serverOverrides, setting);
			int value;
			if (!checkBox || !CSI_SettingInfo.GetWidgetValue(m_wRoot, setting, value))
				continue;

			m_mSentServerOverrides.Set(setting, checkBox.IsChecked());
			m_mSentServerOverrideValues.Set(setting, value);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the override checkboxes changed since the snapshot: checked with the row's value, unchecked clears it.
	protected void SendServerOverrides()
	{
		Widget serverOverrides = m_wRoot.FindAnyWidget(SERVER_OVERRIDES_WIDGET_NAME);
		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			CheckBoxWidget checkBox = GetServerOverrideCheckBox(serverOverrides, setting);
			int value;
			if (!checkBox || !CSI_SettingInfo.GetWidgetValue(m_wRoot, setting, value))
				continue;

			// Unchanged, or only the player's own value changed
			bool overridden = checkBox.IsChecked();
			if (overridden == m_mSentServerOverrides.Get(setting) && (!overridden || value == m_mSentServerOverrideValues.Get(setting)))
				continue;

			if (overridden)
				m_RplToAuthorityManager.Owner_ChangeAuthoritySetting(setting, value, true);
			else
				m_RplToAuthorityManager.Owner_ChangeAuthoritySetting(setting, 0, false);

			m_mSentServerOverrides.Set(setting, overridden);
			m_mSentServerOverrideValues.Set(setting, value);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] serverOverrides server override column
	//! \param[in] setting CSI_GameSettings name
	//! \return null if missing
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
	//! Indents nested settings and shows or hides them as their switches change.
	protected void SetupNestedSettings()
	{
		m_wSettingsTree = CanvasWidget.Cast(m_wRoot.FindAnyWidget(SETTINGS_TREE_WIDGET_NAME));
		if (!m_wSettingsTree)
			PrintFormat("CSI_SettingsSubMenu: tree canvas '%1' not found, nested settings won't have tree lines", SETTINGS_TREE_WIDGET_NAME, level: LogLevel.WARNING);

		RegisterDependentSettings();
		foreach (string separator, string separatorParent : m_mSeparatorParents)
		{
			if (!m_wRoot.FindAnyWidget(separator))
				PrintFormat("CSI_SettingsSubMenu: separator '%1' not found, it won't hide with '%2'", separator, separatorParent, level: LogLevel.WARNING);
		}

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
	//! Fills m_mSettingParents (setting -> its switch) and m_mSeparatorParents. Add new nested settings here.
	protected void RegisterDependentSettings()
	{
		m_mSettingParents = new map<string, string>();
		m_mSeparatorParents = new map<string, string>();

		TStringArray autoHideHud = {CSI_GameSettings.REVEAL_UI_MODE};
		NestSettings(CSI_GameSettings.AUTO_HIDE_HUD, autoHideHud);

		// Then the look block, below its own line
		TStringArray nametags = {
			CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE,
			CSI_GameSettings.NAMETAG_LOS_VISIBLE,
			CSI_GameSettings.NAMETAG_RANGE,
			CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION,
			CSI_GameSettings.NAMETAG_FIELD_OF_VIEW,
			CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE,
			CSI_GameSettings.NAMETAG_COLOR,
			CSI_GameSettings.NAMETAG_VON_COLOR,
			CSI_GameSettings.NAMTEAG_SCALE,
			CSI_GameSettings.NAMETAG_POSITION,
			CSI_GameSettings.NAMETAG_POSITION_OFFSET,
			CSI_GameSettings.NAMETAG_BRIGHTNESS_DIMMING};
		NestSettings(CSI_GameSettings.NAMETAG_VISIBLE, nametags);
		m_mSeparatorParents.Set(NAMETAG_LOOK_SEPARATOR_NAME, CSI_GameSettings.NAMETAG_VISIBLE);

		TStringArray groupNames = {CSI_GameSettings.SQUADMATE_GROUP_IN_NAMETAG_VISIBLE};
		NestSettings(CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE, groupNames);

		// The icon-only settings only work with role icons shown
		TStringArray roleIcon = {
			CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION,
			CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED,
			CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED};
		NestSettings(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE, roleIcon);

		TStringArray compass = {CSI_GameSettings.COMPASS_THEME, CSI_GameSettings.COMPASS_COLOR};
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
	//! Shows nested rows, separators and their override rows only while every switch above them is on.
	protected void UpdateDependentSettingsVisibility()
	{
		if (!m_mSettingParents || !m_mSeparatorParents)
			return;

		Widget serverOverrides = m_wRoot.FindAnyWidget(SERVER_OVERRIDES_WIDGET_NAME);
		foreach (string dependentSetting, string parentSetting : m_mSettingParents)
		{
			SetRowVisible(dependentSetting, IsSettingShown(dependentSetting), serverOverrides);
		}

		foreach (string separator, string separatorParent : m_mSeparatorParents)
		{
			SetRowVisible(separator, IsSettingShown(separatorParent) && IsSettingRowOn(separatorParent), serverOverrides);
		}

		m_iSettingsTreeRedraws = TREE_REDRAW_FRAMES;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] name setting or separator, the name of both its row and its override row
	//! \param[in] visible
	//! \param[in] serverOverrides server override column
	protected void SetRowVisible(string name, bool visible, Widget serverOverrides)
	{
		// Finds the setting row, not its override row (the list comes first)
		Widget row = m_wRoot.FindAnyWidget(name);
		if (row)
			row.SetVisible(visible);

		if (!serverOverrides)
			return;

		Widget overrideRow = serverOverrides.FindAnyWidget(name);
		if (overrideRow)
			overrideRow.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \return true if not nested, or its switch is shown and on
	protected bool IsSettingShown(string setting)
	{
		string parentSetting;
		if (!m_mSettingParents.Find(setting, parentSetting))
			return true;

		return IsSettingShown(parentSetting) && IsSettingRowOn(parentSetting);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting an on/off CSI_GameSettings name
	//! \return true if the row shows on, server-forced values included
	protected bool IsSettingRowOn(string setting)
	{
		int value;
		if (CSI_SettingInfo.GetWidgetValue(m_wRoot, setting, value))
			return value > 0;

		return m_SettingsManager && m_SettingsManager.GetSettingBool(setting);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \return 0 for top-level settings
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
	//! Pads the label's inner frame, not the label, so the row's control stays lined up.
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
	//! \return null if missing
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
	//! Draws a line down from each switch's label with a branch to each shown nested label.
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
		m_fSettingsTreeWidth = canvasWidth;
		m_fSettingsTreeHeight = canvasHeight;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the tree canvas's screen size changed since the tree was drawn
	protected bool HasSettingsTreeResized()
	{
		if (!m_wSettingsTree)
			return false;

		float width, height;
		m_wSettingsTree.GetScreenSize(width, height);
		return width != m_fSettingsTreeWidth || height != m_fSettingsTreeHeight;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] settingsList settings list, whose children are the rows
	//! \return switch -> its shown nested rows, in list order
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
	//! \param[out] x screen position
	//! \param[out] y
	//! \param[out] width screen size
	//! \param[out] height
	//! \return false if missing or not laid out yet
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
	//! Saves the settings JSON only if the player already has one or changed something here.
	protected void SaveLocalSettings()
	{
		// Main menu: no player controller
		if (!m_PlayerControllerManager || !m_PlayerControllerManager.GetLocalSettingsJson())
		{
			if (CSI_SettingsJson.HasSettingsFile() || !m_aValuesOnOpen || !SCR_ArrayHelperT<int>.AreEqual(GetUserSettingValues(), m_aValuesOnOpen))
			{
				CSI_SettingsJson settingsJson = new CSI_SettingsJson();
				settingsJson.SaveToFile();
			}

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
			GetGame().UserSettingsChanged();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsInSessionWithoutSavedSettings()
	{
		return m_PlayerControllerManager && m_PlayerControllerManager.GetLocalSettingsJson() && !m_PlayerControllerManager.GetLocalSettingsJson().HasSavedSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! \return values by settings index
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
	//! \param[in] values by settings index
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
