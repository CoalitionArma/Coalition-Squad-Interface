//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Saves a widget's FrameSlot anchors, position and size so the settings examples can restore it.
//!
//------------------------------------------------------------------------------------------------
class CSI_FrameSlotSnapshot
{
	protected vector m_vAnchorMin;
	protected vector m_vAnchorMax;
	protected vector m_vPos;
	protected vector m_vSize;

	//------------------------------------------------------------------------------------------------
	//! \param[in] widget a widget in a FrameWidget
	void Save(notnull Widget widget)
	{
		m_vAnchorMin = FrameSlot.GetAnchorMin(widget);
		m_vAnchorMax = FrameSlot.GetAnchorMax(widget);
		m_vPos = FrameSlot.GetPos(widget);
		m_vSize = FrameSlot.GetSize(widget);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] widget the widget it was saved from
	void Restore(notnull Widget widget)
	{
		FrameSlot.SetAnchorMin(widget, m_vAnchorMin[0], m_vAnchorMin[1]);
		FrameSlot.SetAnchorMax(widget, m_vAnchorMax[0], m_vAnchorMax[1]);
		FrameSlot.SetPos(widget, m_vPos[0], m_vPos[1]);
		FrameSlot.SetSize(widget, m_vSize[0], m_vSize[1]);
	}

	//------------------------------------------------------------------------------------------------
	vector GetPos()
	{
		return m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	vector GetSize()
	{
		return m_vSize;
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD examples (CSI_SettingInfoHud.layout) for the General, Compass and Radar settings and the fire team
//! 				colors: sets the embedded CSI_HUD.layout's widgets from the tab's values and creates its own group rows
//! 				and radar icons.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfoHudExamples : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	// General. Pixel values below are 1.4x the real HUD's, the layout's HUD scale
	protected static const int PREVIEW_HEADING = 45;			//!< compass heading the examples show
	protected static const float PREVIEW_STAMINA = 0.75;
	protected static const float AUTO_HIDDEN_OPACITY = 0.2;

	// Group display
	protected static const int GROUP_ROWS = 3;					//!< group display rows the Auto-hide HUD example fills
	protected static const int GROUP_GRID_ROWS = 5;				//!< rows per column of CSI_Group.layout's grid
	protected static const int GROUP_GRID_COLUMNS = 5;			//!< columns of CSI_Group.layout's grid
	protected static const int GROUP_NAME_FONT_SIZE = 14;		//!< CSI_Player.layout's name size (10) x 1.4
	protected static const float SQUAD_VIEW_MARGIN = 0.03;		//!< squad view: share of the box's width left free past the squad

	// Group display example: a 20 player squad led by the local player, a column per team
	protected static const ref array<CSI_EIcon> DEMO_SQUAD_ROLES = {
		CSI_EIcon.SL, CSI_EIcon.TL, CSI_EIcon.RTO, CSI_EIcon.MG, CSI_EIcon.MEDIC,
		CSI_EIcon.TL, CSI_EIcon.MG, CSI_EIcon.GREN, CSI_EIcon.AT, CSI_EIcon.MAN,
		CSI_EIcon.TL, CSI_EIcon.MG, CSI_EIcon.GREN, CSI_EIcon.ENG, CSI_EIcon.MEDIC,
		CSI_EIcon.TL, CSI_EIcon.SNIPER, CSI_EIcon.GREN, CSI_EIcon.DEMO, CSI_EIcon.AT};
	protected static const ref array<CSI_EColorTeam> DEMO_SQUAD_TEAMS = {
		CSI_EColorTeam.NONE, CSI_EColorTeam.RED, CSI_EColorTeam.RED, CSI_EColorTeam.RED, CSI_EColorTeam.RED,
		CSI_EColorTeam.BLUE, CSI_EColorTeam.BLUE, CSI_EColorTeam.BLUE, CSI_EColorTeam.BLUE, CSI_EColorTeam.BLUE,
		CSI_EColorTeam.YELLOW, CSI_EColorTeam.YELLOW, CSI_EColorTeam.YELLOW, CSI_EColorTeam.YELLOW, CSI_EColorTeam.YELLOW,
		CSI_EColorTeam.GREEN, CSI_EColorTeam.GREEN, CSI_EColorTeam.GREEN, CSI_EColorTeam.GREEN, CSI_EColorTeam.GREEN};
	//! Placeholder squadmate names, shuffled when the tab opens
	protected static const ref array<string> DEMO_SQUAD_NAMES = {
		"Salami", "RandAlThor", "Shimavitz", "Tanaka", "Mike", "Klub", "Dagger", "Kaltag", "Lazz", "Embrodak",
		"Reckless", "Jlaw", "SkurtBird", "Brooks", "RebelCid", "Arkangel", "The Canadian", "Fontaine", "Trist", "Svir"};

	// Compass and radar
	protected static const float COMPASS_FOCUS_ZOOM = 1.25;		//!< compass zoom in the Compass and Radar examples
	protected static const float RADAR_ICON_SIZE = 22.4;		//!< px at 100% RADAR_ICON_SIZE: CSI_Radar.ICON_WIDTH_AND_HEIGHT (16) x 1.4
	protected static const int RADAR_ICON_COUNT = 4;			//!< radar icons the examples can show at once
	protected static const float RADAR_RANGE_PX = 67.82;		//!< px from the center to the range edge: (132 / 2.725) x 1.4
	protected static const float ARROW_DEMO_ICON_SIZE = 56;
	protected static const float ARROW_DEMO_HEADING = 60;

	// Squadmates in the radar examples: facings and placeholder color teams
	protected static const ref array<float> DEMO_MATE_FACINGS = {10, -30, 120, 200};
	protected static const ref array<CSI_EColorTeam> DEMO_MATE_TEAMS = {CSI_EColorTeam.RED, CSI_EColorTeam.BLUE, CSI_EColorTeam.GREEN, CSI_EColorTeam.YELLOW};

	// Radar range examples: squadmate bearings (degrees, + is right) and distances (m)
	protected static const ref array<float> RANGE_MATE_BEARINGS = {-30, 50, 160, -110};
	protected static const ref array<float> RANGE_MATE_DISTANCES_ON_FOOT = {8, 18, 35, 70};
	protected static const ref array<float> RANGE_MATE_DISTANCES_IN_VEHICLE = {1, 2.5, 5, 8};

	// Radar field of view example: squadmate bearings, distances (px) and dotted edges
	protected static const ref array<float> FOV_MATE_BEARINGS = {0, -40, 95, 165};
	protected static const ref array<float> FOV_MATE_DISTANCES = {47.6, 36.4, 42, 50.4};
	protected static const float FOV_FADED_OPACITY = 0.15;		//!< squadmates outside the field of view
	protected static const float FOV_EDGE_START = 11.2;			//!< px from the center to the first dot of each edge
	protected static const float FOV_EDGE_LENGTH = 64.4;		//!< px from the center to the last dot, inside RADAR_RANGE_PX
	protected static const int FOV_EDGE_DOTS = 8;				//!< dots per edge
	protected static const float FOV_EDGE_DOT_SIZE = 4.2;		//!< px
	protected static const ResourceName FOV_EDGE_DOT_TEXTURE = "{73B3D8BBB785B5B9}UI/Textures/Common/circleFull.edds";

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected Widget m_wSettingsRoot;	//!< CSI settings tab root, for the selected values

	// The HUD's widgets
	protected Widget m_wHud;
	protected Widget m_wCompass;
	protected ImageWidget m_wCompassImage;
	protected TextWidget m_wBearing;
	protected Widget m_wRadarFrame;
	protected Widget m_wGroup;
	protected Widget m_wStamina;
	protected ProgressBarWidget m_wStaminaBar;
	protected CSI_Stamina m_Stamina;

	// Compass and group display layout slots, restored after an example moves them
	protected ref CSI_FrameSlotSnapshot m_CompassLayoutSlot = new CSI_FrameSlotSnapshot();
	protected ref CSI_FrameSlotSnapshot m_GroupLayoutSlot = new CSI_FrameSlotSnapshot();
	protected float m_fCompassZoom = 1;		//!< COMPASS_FOCUS_ZOOM while focused, else 1; scales radar px values

	protected ref array<Widget> m_aGroupRows = {};				//!< rows in the HUD's group grid
	protected ref array<string> m_aDemoSquadNames = {};			//!< DEMO_SQUAD_NAMES shuffled when the tab opens
	protected ref array<Widget> m_aRadarIcons = {};				//!< icons in the HUD's radar frame
	protected ref array<Widget> m_aRadarFieldOfViewDots = {};	//!< both field of view edges' dots, created on first show

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!FindHudWidgets(w))
			return;

		ShuffleDemoSquadNames();

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		CreateGroupRows(workspace);
		CreateRadarIcons(workspace);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		// A pending retry of PlaceSquadView
		if (GetGame())
			GetGame().GetCallqueue().Remove(PlaceSquadView);

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the HUD's widgets and saves the compass and group display layout slots.
	//! \param[in] w this component's root
	//! \return false if the layout has no HUD
	protected bool FindHudWidgets(notnull Widget w)
	{
		m_wHud = w.FindAnyWidget("Hud");
		if (!m_wHud)
		{
			Print("CSI_SettingInfoHudExamples: 'Hud' (CSI_HUD.layout) not found, the HUD examples won't show", LogLevel.WARNING);
			return false;
		}

		m_wStamina = m_wHud.FindAnyWidget("Stamina");
		if (m_wStamina)
		{
			m_wStaminaBar = ProgressBarWidget.Cast(m_wStamina.FindAnyWidget("StamBar"));
			m_Stamina = CSI_Stamina.Cast(m_wStamina.FindHandler(CSI_Stamina));
		}
		else
		{
			Print("CSI_SettingInfoHudExamples: HUD 'Stamina' not found", LogLevel.WARNING);
		}

		m_wCompass = m_wHud.FindAnyWidget("Compass");
		if (m_wCompass)
		{
			m_wCompassImage = ImageWidget.Cast(m_wCompass.FindAnyWidget("Compass"));
			m_wBearing = TextWidget.Cast(m_wCompass.FindAnyWidget("Bearing"));
			m_wRadarFrame = m_wCompass.FindAnyWidget("RadarFrame");
			m_CompassLayoutSlot.Save(m_wCompass);
		}
		else
		{
			Print("CSI_SettingInfoHudExamples: HUD 'Compass' not found", LogLevel.WARNING);
		}

		m_wGroup = m_wHud.FindAnyWidget("Group");
		if (m_wGroup)
			m_GroupLayoutSlot.Save(m_wGroup);
		else
			Print("CSI_SettingInfoHudExamples: HUD 'Group' not found", LogLevel.WARNING);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates enough rows for the Auto-hide HUD and Group display examples.
	//! \param[in] workspace
	protected void CreateGroupRows(notnull WorkspaceWidget workspace)
	{
		Widget groupGrid;
		if (m_wGroup)
			groupGrid = m_wGroup.FindAnyWidget("GroupGridLayout");

		if (!groupGrid)
		{
			Print("CSI_SettingInfoHudExamples: group grid 'GroupGridLayout' not found, the group examples won't show", LogLevel.WARNING);
			return;
		}

		int rowCount = Math.Max(GROUP_ROWS, DEMO_SQUAD_ROLES.Count());
		for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
		{
			Widget row = workspace.CreateWidgets(CSI_UIHelper.CSI_PLAYER_LAYOUT, groupGrid);
			if (!row)
				continue;

			GridSlot.SetRow(row, rowIndex % GROUP_GRID_ROWS);
			GridSlot.SetColumn(row, rowIndex / GROUP_GRID_ROWS);
			m_aGroupRows.Insert(row);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates RADAR_ICON_COUNT hidden icons in the radar frame.
	//! \param[in] workspace
	protected void CreateRadarIcons(notnull WorkspaceWidget workspace)
	{
		if (!m_wRadarFrame)
		{
			Print("CSI_SettingInfoHudExamples: compass 'RadarFrame' not found, the radar examples won't show icons", LogLevel.WARNING);
			return;
		}

		for (int radarIndex = 0; radarIndex < RADAR_ICON_COUNT; radarIndex++)
		{
			Widget radarIcon = workspace.CreateWidgets(CSI_UIHelper.CSI_ICON_LAYOUT, m_wRadarFrame);
			if (!radarIcon)
				continue;

			radarIcon.SetVisible(false);
			m_aRadarIcons.Insert(radarIcon);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShuffleDemoSquadNames()
	{
		m_aDemoSquadNames.Copy(DEMO_SQUAD_NAMES);
		SCR_ArrayHelperT<string>.Shuffle(m_aDemoSquadNames);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 EXAMPLES
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[out] caption text under the example (may be empty)
	//! \return false if this layout has no example for the setting
	bool ShowExample(string setting, out string caption)
	{
		int value = GetValue(setting);
		if (ShowGeneralExample(setting, value, caption))
			return true;

		return ShowCompassAndRadarExample(setting, value, caption);
	}

	//------------------------------------------------------------------------------------------------
	void HideExample()
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(PlaceSquadView);

		if (m_wRoot)
			m_wRoot.SetVisible(false);

		foreach (Widget radarIcon : m_aRadarIcons)
		{
			radarIcon.SetVisible(false);
		}

		foreach (Widget dot : m_aRadarFieldOfViewDots)
		{
			dot.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value the setting's value in the tab
	//! \param[out] caption
	//! \return false if the setting isn't one of the General settings shown here
	protected bool ShowGeneralExample(string setting, int value, out string caption)
	{
		if (setting == CSI_GameSettings.AUTO_HIDE_HUD)
		{
			if (value)
			{
				ShowConfiguredHud(AUTO_HIDDEN_OPACITY);
				caption = GetRevealCaption();
				return true;
			}

			ShowConfiguredHud(1);
			caption = "Always shown";
			return true;
		}

		if (setting == CSI_GameSettings.REVEAL_UI_MODE)
		{
			ShowConfiguredHud(AUTO_HIDDEN_OPACITY);
			if (!GetValue(CSI_GameSettings.AUTO_HIDE_HUD))
				caption = "Auto-hide HUD is off";
			else
				caption = GetRevealCaption();

			return true;
		}

		if (setting == CSI_GameSettings.STAMINA_VISIBLE)
		{
			if (!value)
			{
				caption = "Hidden";
				return true;
			}

			ShowHud(false, false, true, 1);
			caption = "Bottom left, while you're not at full stamina";
			return true;
		}

		if (setting == CSI_GameSettings.GROUP_VISIBLE)
		{
			if (!value)
			{
				ShowHud(true, false, false, 1);
				caption = "Hidden";
				return true;
			}

			ShowDemoSquad();
			return true;
		}

		if (setting == CSI_GameSettings.COLOR_TEAM_SET)
		{
			ShowDemoSquad();
			caption = CSI_ColorPresets.GetColorTeamSetName(value);
			return true;
		}

		if (setting == CSI_GameSettings.STANDARD_SQUAD_COLOR)
		{
			ShowDemoSquad();
			int preset = CSI_ColorPresets.GetNonClashingPreset(value, GetValue(CSI_GameSettings.COLOR_TEAM_SET));
			caption = string.Format("Squadmates without a fire team: %1", CSI_ColorPresets.GetName(preset));
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value the setting's value in the tab
	//! \param[out] caption
	//! \return false if the setting isn't one of the Compass or Radar settings
	protected bool ShowCompassAndRadarExample(string setting, int value, out string caption)
	{
		if (setting == CSI_GameSettings.COMPASS_THEME || setting == CSI_GameSettings.COMPASS_VISIBLE || setting == CSI_GameSettings.BEARING_VISIBLE)
		{
			ShowHud(true, false, false, 1, true);
			if (setting != CSI_GameSettings.COMPASS_THEME && !value)
				caption = "Hidden";

			return true;
		}

		if (setting == CSI_GameSettings.COMPASS_COLOR)
		{
			ShowHud(true, false, false, 1, true);
			caption = CSI_ColorPresets.GetName(value);
			return true;
		}

		if (setting == CSI_GameSettings.RADAR_VISIBLE || setting == CSI_GameSettings.RADAR_ICON_SIZE)
		{
			ShowHud(true, false, false, 1, true);
			if (setting == CSI_GameSettings.RADAR_VISIBLE && !value)
			{
				caption = "Hidden";
				return true;
			}

			ShowRadarIcons();
			return true;
		}

		if (setting == CSI_GameSettings.RADAR_RANGE_ON_FOOT || setting == CSI_GameSettings.RADAR_RANGE_IN_VEHICLE)
		{
			ShowHud(true, false, false, 1, true);
			if (!GetValue(CSI_GameSettings.RADAR_VISIBLE))
			{
				caption = "Radar is hidden";
				return true;
			}

			bool inVehicle = setting == CSI_GameSettings.RADAR_RANGE_IN_VEHICLE;
			int range = CSI_GameSettings.ClampToLimits(setting, value);

			ShowRadarRange(range, inVehicle);
			if (inVehicle)
				caption = string.Format("In vehicles, squadmates within %1 m show on the radar", range);
			else
				caption = string.Format("On foot, squadmates within %1 m show on the radar", range);

			return true;
		}

		if (setting == CSI_GameSettings.RADAR_FIELD_OF_VIEW)
		{
			ShowHud(true, false, false, 1, true);
			if (!GetValue(CSI_GameSettings.RADAR_VISIBLE))
			{
				caption = "Radar is hidden";
				return true;
			}

			int fieldOfViewMin, fieldOfViewMax;
			CSI_GameSettings.GetSettingLimits(setting, fieldOfViewMin, fieldOfViewMax);
			int fieldOfView = Math.ClampInt(value, fieldOfViewMin, fieldOfViewMax);
			ShowRadarFieldOfView(fieldOfView);
			if (fieldOfView >= fieldOfViewMax)
				caption = "Every squadmate on the radar is shown";
			else
				caption = string.Format("Squadmates outside the %1° ahead slowly fade out", fieldOfView);

			return true;
		}

		if (setting == CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE)
		{
			ShowHud(true, false, false, 1, true);
			ShowArrowsRotateDemo(value);
			if (value)
				caption = "Only the arrow turns; the icon stays upright";
			else
				caption = "The whole icon turns";

			return true;
		}

		return false;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 HUD
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Compass, plus group display and stamina bar if enabled in the tab.
	//! \param[in] opacity
	protected void ShowConfiguredHud(float opacity)
	{
		ShowHud(true, GetValue(CSI_GameSettings.GROUP_VISIBLE), GetValue(CSI_GameSettings.STAMINA_VISIBLE), opacity);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows HUD elements filled in with the tab's settings.
	//! \param[in] showCompass compass element (image and bearing follow their settings)
	//! \param[in] showGroup
	//! \param[in] showStamina
	//! \param[in] opacity
	//! \param[in] focusCompass enlarge the compass and center it in the box
	protected void ShowHud(bool showCompass, bool showGroup, bool showStamina, float opacity, bool focusCompass = false)
	{
		if (!m_wHud)
			return;

		m_wRoot.SetVisible(true);
		m_wHud.SetOpacity(opacity);

		if (m_wCompass)
		{
			FocusCompass(showCompass && focusCompass);
			m_wCompass.SetVisible(showCompass);
			if (showCompass)
				UpdateCompass();
		}

		if (m_wGroup)
		{
			m_GroupLayoutSlot.Restore(m_wGroup);
			m_wGroup.SetVisible(showGroup);
			if (showGroup)
				UpdateGroup();
		}

		if (m_wStamina)
		{
			m_wStamina.SetVisible(showStamina);
			if (showStamina)
				UpdateStamina();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] focus enlarge and center the compass, else restore its layout slot
	protected void FocusCompass(bool focus)
	{
		if (!focus)
		{
			m_fCompassZoom = 1;
			m_CompassLayoutSlot.Restore(m_wCompass);
			return;
		}

		m_fCompassZoom = COMPASS_FOCUS_ZOOM;
		vector layoutSize = m_CompassLayoutSlot.GetSize();
		float width = layoutSize[0] * COMPASS_FOCUS_ZOOM;
		float height = layoutSize[1] * COMPASS_FOCUS_ZOOM;
		FrameSlot.SetAnchorMin(m_wCompass, 0.5, 0.5);
		FrameSlot.SetAnchorMax(m_wCompass, 0.5, 0.5);
		FrameSlot.SetPos(m_wCompass, -width * 0.5, -height * 0.5);
		FrameSlot.SetSize(m_wCompass, width, height);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the compass for PREVIEW_HEADING with the tab's compass settings.
	protected void UpdateCompass()
	{
		Color compassColor = CSI_ColorPresets.GetColor(GetValue(CSI_GameSettings.COMPASS_COLOR));
		if (m_wCompassImage)
		{
			m_wCompassImage.LoadImageTexture(0, CSI_UIHelper.GetCompassTexture(GetValue(CSI_GameSettings.COMPASS_THEME)));
			m_wCompassImage.SetColor(compassColor);
			m_wCompassImage.SetRotation(-PREVIEW_HEADING);
			m_wCompassImage.SetVisible(GetValue(CSI_GameSettings.COMPASS_VISIBLE));
		}

		if (!m_wBearing)
			return;

		m_wBearing.SetColor(compassColor);
		int heading = PREVIEW_HEADING;
		if (GetValue(CSI_GameSettings.BEARING_VISIBLE))
			m_wBearing.SetText(heading.ToString(3));
		else
			m_wBearing.SetText(string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the stamina bar at PREVIEW_STAMINA, fully faded in.
	protected void UpdateStamina()
	{
		if (m_Stamina)
			m_Stamina.OnStaminaChange(PREVIEW_STAMINA);

		if (m_wStaminaBar)
			m_wStaminaBar.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! \return how the tab's REVEAL_UI_MODE brings the auto-hidden HUD back
	protected string GetRevealCaption()
	{
		if (GetValue(CSI_GameSettings.REVEAL_UI_MODE) == CSI_ERevealUIMode.TOGGLE)
			return "Hidden until you press Reveal UI; press it again to hide";

		return "Hidden until you hold Reveal UI";
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GROUP DISPLAY
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Fills the first GROUP_ROWS rows with the local group, or placeholders outside a group.
	protected void UpdateGroup()
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, GROUP_ROWS);

		array<CSI_EColorTeam> placeholderTeams = {CSI_EColorTeam.RED, CSI_EColorTeam.RED, CSI_EColorTeam.BLUE};

		foreach (int i, Widget row : m_aGroupRows)
		{
			// The rest are for the Group display squad
			if (i >= GROUP_ROWS)
			{
				row.SetVisible(false);
				continue;
			}

			PlayerId playerID = 0;
			if (memberIDs.IsIndexValid(i))
				playerID = memberIDs[i];

			CSI_EColorTeam colorTeam = CSI_SettingInfo.GetPlayerColorTeam(playerID, placeholderTeams[i]);
			FillGroupRow(row, playerID, GetPlaceholderName(i), CSI_SettingInfo.GetPlayerIcon(playerID), colorTeam);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Group display: a full squad led by the local player beside half the compass.
	protected void ShowDemoSquad()
	{
		ShowHud(true, true, false, 1);

		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		foreach (int i, Widget row : m_aGroupRows)
		{
			if (!DEMO_SQUAD_ROLES.IsIndexValid(i) || !DEMO_SQUAD_TEAMS.IsIndexValid(i))
			{
				row.SetVisible(false);
				continue;
			}

			// The local player leads the squad
			PlayerId playerID;
			if (i == 0)
				playerID = localPlayerID;

			FillGroupRow(row, playerID, GetPlaceholderName(i), DEMO_SQUAD_ROLES[i], DEMO_SQUAD_TEAMS[i]);
		}

		PlaceSquadView();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] row CSI_Player.layout row
	//! \param[in] playerID 0 for a placeholder
	//! \param[in] fallbackName shown without a player name
	//! \param[in] role
	//! \param[in] colorTeam
	protected void FillGroupRow(notnull Widget row, PlayerId playerID, string fallbackName, CSI_EIcon role, CSI_EColorTeam colorTeam)
	{
		TextWidget name = TextWidget.Cast(row.FindAnyWidget("PlayerName"));
		if (name)
		{
			name.SetText(CSI_SettingInfo.GetPreviewName(m_wSettingsRoot, playerID, fallbackName));
			name.SetExactFontSize(GROUP_NAME_FONT_SIZE);
			name.SetColor(CSI_ColorPresets.GetColorTeamColor(colorTeam, GetValue(CSI_GameSettings.COLOR_TEAM_SET), GetValue(CSI_GameSettings.STANDARD_SQUAD_COLOR)));
		}

		// Keep the row's own icon size (0)
		CSI_SettingInfo.ShowIcon(m_wSettingsRoot, row.FindAnyWidget("Icon"), 0, role, colorTeam, 0, 0, 1);
		row.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rowIndex
	//! \return the profile name for row 0, else a shuffled DEMO_SQUAD_NAMES entry
	protected string GetPlaceholderName(int rowIndex)
	{
		if (rowIndex == 0)
			return CSI_SettingInfo.GetProfileName();

		if (m_aDemoSquadNames.IsEmpty())
			return "Squadmate";

		return m_aDemoSquadNames[(rowIndex - 1) % m_aDemoSquadNames.Count()];
	}

	//------------------------------------------------------------------------------------------------
	//! Fits the compass and squad in the box, with half the compass past its left edge.
	//! Retries until the box is laid out.
	protected void PlaceSquadView()
	{
		GetGame().GetCallqueue().Remove(PlaceSquadView);

		if (!m_wRoot || !m_wRoot.IsVisible() || !m_wCompass || !m_wGroup)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float rootWidth, rootHeight;
		m_wRoot.GetScreenSize(rootWidth, rootHeight);
		float pixelsPerUnit = workspace.DPIScale(1);
		if (rootWidth <= 0 || pixelsPerUnit <= 0)
		{
			GetGame().GetCallqueue().CallLater(PlaceSquadView, 10);
			return;
		}

		vector compassPos = m_CompassLayoutSlot.GetPos();
		vector compassSize = m_CompassLayoutSlot.GetSize();
		vector groupPos = m_GroupLayoutSlot.GetPos();
		vector groupSize = m_GroupLayoutSlot.GetSize();

		// The compass is centered on the group's anchor; the group starts groupPos[0] to its right
		int columns = Math.Ceil(DEMO_SQUAD_ROLES.Count() / (float)GROUP_GRID_ROWS);
		float squadWidth = groupPos[0] + groupSize[0] * columns / GROUP_GRID_COLUMNS;
		float scale = Math.Min(1, rootWidth / pixelsPerUnit * (1 - SQUAD_VIEW_MARGIN) / squadWidth);

		// Center both vertically, keeping their layout offsets
		float top = Math.Min(compassPos[1], groupPos[1]);
		float bottom = Math.Max(compassPos[1] + compassSize[1], groupPos[1] + groupSize[1]);
		float center = (top + bottom) * 0.5;

		FrameSlot.SetAnchorMin(m_wCompass, 0, 0.5);
		FrameSlot.SetAnchorMax(m_wCompass, 0, 0.5);
		FrameSlot.SetSize(m_wCompass, compassSize[0] * scale, compassSize[1] * scale);
		FrameSlot.SetPos(m_wCompass, -compassSize[0] * scale * 0.5, (compassPos[1] - center) * scale);

		FrameSlot.SetAnchorMin(m_wGroup, 0, 0.5);
		FrameSlot.SetAnchorMax(m_wGroup, 0, 0.5);
		FrameSlot.SetSize(m_wGroup, groupSize[0] * scale, groupSize[1] * scale);
		FrameSlot.SetPos(m_wGroup, groupPos[0] * scale, (groupPos[1] - center) * scale);

		int fontSize = Math.Round(GROUP_NAME_FONT_SIZE * scale);
		foreach (Widget row : m_aGroupRows)
		{
			TextWidget name = TextWidget.Cast(row.FindAnyWidget("PlayerName"));
			if (name)
				name.SetExactFontSize(fontSize);
		}
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RADAR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Two group members on the radar at the tab's radar icon size.
	protected void ShowRadarIcons()
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, GROUP_ROWS);

		// Leave the local player out, they're the center of the radar
		memberIDs.RemoveItem(SCR_PlayerController.GetLocalPlayerId());

		float iconSize = GetRadarIconSize();
		bool onlyArrowsRotate = GetValue(CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE);
		array<float> positionsX = {-30.8, 26.6};
		array<float> positionsY = {-23.8, 19.6};
		array<float> rotations = {30, -60};
		array<CSI_EColorTeam> placeholderTeams = {CSI_EColorTeam.RED, CSI_EColorTeam.BLUE};

		for (int i = 0; i < 2; i++)
		{
			PlayerId playerID = 0;
			if (memberIDs.IsIndexValid(i))
				playerID = memberIDs[i];

			float iconRotation = rotations[i];
			if (onlyArrowsRotate)
				iconRotation = 0;

			Widget icon = GetRadarIcon(i);
			PlaceInRadar(icon, positionsX[i] * m_fCompassZoom, positionsY[i] * m_fCompassZoom);
			CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, iconSize, CSI_SettingInfo.GetPlayerIcon(playerID), CSI_SettingInfo.GetPlayerColorTeam(playerID, placeholderTeams[i]), iconRotation, rotations[i], 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Squadmates at set distances, scaled so the range reaches the radar's edge.
	//! \param[in] range meters
	//! \param[in] inVehicle use the in-vehicle distances
	protected void ShowRadarRange(int range, bool inVehicle)
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, RADAR_ICON_COUNT + 1);

		// Leave the local player out, they're the center of the radar
		memberIDs.RemoveItem(SCR_PlayerController.GetLocalPlayerId());

		array<float> distances = RANGE_MATE_DISTANCES_ON_FOOT;
		if (inVehicle)
			distances = RANGE_MATE_DISTANCES_IN_VEHICLE;

		float iconSize = GetRadarIconSize();
		bool onlyArrowsRotate = GetValue(CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE);

		// Fade over the last 20% of the range
		float fadeStart = 0.8 * range;

		foreach (int mateIndex, float bearing : RANGE_MATE_BEARINGS)
		{
			Widget icon = GetRadarIcon(mateIndex);
			if (!icon)
				continue;

			float distance = distances[mateIndex];
			if (distance > range)
			{
				icon.SetVisible(false);
				continue;
			}

			PlayerId playerID = 0;
			if (memberIDs.IsIndexValid(mateIndex))
				playerID = memberIDs[mateIndex];

			float iconRotation = DEMO_MATE_FACINGS[mateIndex];
			if (onlyArrowsRotate)
				iconRotation = 0;

			float opacity = Math.Clamp(Math.Map(distance, fadeStart, range, 0.6, 0), 0, 1);

			vector position = GetRadarPoint(bearing, distance * RADAR_RANGE_PX * m_fCompassZoom / range);
			PlaceInRadar(icon, position[0], position[1]);
			CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, iconSize, CSI_SettingInfo.GetPlayerIcon(playerID), CSI_SettingInfo.GetPlayerColorTeam(playerID, DEMO_MATE_TEAMS[mateIndex]), iconRotation, DEMO_MATE_FACINGS[mateIndex], opacity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Squadmates and dotted field of view edges on the radar; those outside it are faded.
	//! \param[in] fieldOfView RADAR_FIELD_OF_VIEW value (degrees)
	protected void ShowRadarFieldOfView(int fieldOfView)
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, RADAR_ICON_COUNT + 1);

		// Leave the local player out, they're the center of the radar
		memberIDs.RemoveItem(SCR_PlayerController.GetLocalPlayerId());

		int fieldOfViewMin, fieldOfViewMax;
		CSI_GameSettings.GetSettingLimits(CSI_GameSettings.RADAR_FIELD_OF_VIEW, fieldOfViewMin, fieldOfViewMax);

		float halfFieldOfView = fieldOfView * 0.5;
		bool everyoneShown = fieldOfView >= fieldOfViewMax;
		float iconSize = GetRadarIconSize();
		bool onlyArrowsRotate = GetValue(CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE);

		foreach (int mateIndex, float bearing : FOV_MATE_BEARINGS)
		{
			PlayerId playerID = 0;
			if (memberIDs.IsIndexValid(mateIndex))
				playerID = memberIDs[mateIndex];

			float iconRotation = DEMO_MATE_FACINGS[mateIndex];
			if (onlyArrowsRotate)
				iconRotation = 0;

			float opacity = 1;
			if (!everyoneShown && Math.AbsFloat(bearing) > halfFieldOfView)
				opacity = FOV_FADED_OPACITY;

			vector position = GetRadarPoint(bearing, FOV_MATE_DISTANCES[mateIndex] * m_fCompassZoom);
			Widget icon = GetRadarIcon(mateIndex);
			PlaceInRadar(icon, position[0], position[1]);
			CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, iconSize, CSI_SettingInfo.GetPlayerIcon(playerID), CSI_SettingInfo.GetPlayerColorTeam(playerID, DEMO_MATE_TEAMS[mateIndex]), iconRotation, DEMO_MATE_FACINGS[mateIndex], opacity);
		}

		// No edges when everyone is shown
		CreateRadarFieldOfViewDots();
		foreach (int dotIndex, Widget dot : m_aRadarFieldOfViewDots)
		{
			dot.SetVisible(!everyoneShown);
			if (everyoneShown)
				continue;

			// First half of the dots is the left edge, second half the right
			float side = -1;
			int indexOnEdge = dotIndex;
			if (dotIndex >= FOV_EDGE_DOTS)
			{
				side = 1;
				indexOnEdge = dotIndex - FOV_EDGE_DOTS;
			}

			float distance = Math.Lerp(FOV_EDGE_START, FOV_EDGE_LENGTH, indexOnEdge / (FOV_EDGE_DOTS - 1.0)) * m_fCompassZoom;
			vector dotPos = GetRadarPoint(side * halfFieldOfView, distance);
			FrameSlot.SetPos(dot, dotPos[0], dotPos[1]);
			FrameSlot.SetSize(dot, FOV_EDGE_DOT_SIZE * m_fCompassZoom, FOV_EDGE_DOT_SIZE * m_fCompassZoom);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateRadarFieldOfViewDots()
	{
		if (!m_aRadarFieldOfViewDots.IsEmpty() || !m_wRadarFrame)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		for (int i = 0; i < FOV_EDGE_DOTS * 2; i++)
		{
			// Reforger orange
			ImageWidget dot = ImageWidget.Cast(workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.BLEND | WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.INHERIT_CLIPPING, UIColors.CONTRAST_COLOR, 0, m_wRadarFrame));
			if (!dot)
				continue;

			dot.LoadImageTexture(0, FOV_EDGE_DOT_TEXTURE);
			FrameSlot.SetAnchorMin(dot, 0.5, 0.5);
			FrameSlot.SetAnchorMax(dot, 0.5, 0.5);
			FrameSlot.SetAlignment(dot, 0.5, 0.5);
			FrameSlot.SetSize(dot, FOV_EDGE_DOT_SIZE, FOV_EDGE_DOT_SIZE);
			m_aRadarFieldOfViewDots.Insert(dot);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A large AT icon in the middle of the radar, facing ARROW_DEMO_HEADING.
	//! \param[in] onlyArrowsRotate
	protected void ShowArrowsRotateDemo(bool onlyArrowsRotate)
	{
		float iconRotation = ARROW_DEMO_HEADING;
		if (onlyArrowsRotate)
			iconRotation = 0;

		// AT's launcher shows which way the icon is turned
		Widget icon = GetRadarIcon(0);
		PlaceInRadar(icon, 0, 0);
		CSI_EColorTeam colorTeam = CSI_SettingInfo.GetPlayerColorTeam(SCR_PlayerController.GetLocalPlayerId(), CSI_EColorTeam.NONE);
		CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, ARROW_DEMO_ICON_SIZE * m_fCompassZoom, CSI_EIcon.AT, colorTeam, iconRotation, ARROW_DEMO_HEADING, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \return px for the tab's RADAR_ICON_SIZE at the current compass zoom
	protected float GetRadarIconSize()
	{
		return RADAR_ICON_SIZE * GetValue(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01 * m_fCompassZoom;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] icon radar icon
	//! \param[in] x px from the radar's center
	//! \param[in] y px from the radar's center
	protected void PlaceInRadar(Widget icon, float x, float y)
	{
		if (!icon)
			return;

		FrameSlot.SetAnchor(icon, 0.5, 0.5);
		FrameSlot.SetAlignment(icon, 0.5, 0.5);
		FrameSlot.SetPos(icon, x, y);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] bearing degrees from the top of the radar, positive to the right
	//! \param[in] distance px from the radar's center
	//! \return the point (x, y) from the radar's center
	protected vector GetRadarPoint(float bearing, float distance)
	{
		float radians = bearing * Math.DEG2RAD;
		return Vector(Math.Sin(radians) * distance, -Math.Cos(radians) * distance, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected Widget GetRadarIcon(int index)
	{
		if (!m_aRadarIcons.IsIndexValid(index))
			return null;

		return m_aRadarIcons[index];
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 HELPERS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting
	//! \return the value selected in the settings tab
	protected int GetValue(string setting)
	{
		return CSI_SettingInfo.GetValue(m_wSettingsRoot, setting);
	}
}
