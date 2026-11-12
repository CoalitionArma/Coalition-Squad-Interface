//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	HUD examples (CSI_SettingInfoHud.layout) for the General and Compass & Radar settings. The layout holds
//! 				the real CSI_HUD.layout, whose components are never updated; this sets its widgets the way CSI_Compass,
//! 				CSI_Group, CSI_Stamina and CSI_Radar would, with the values selected in the settings tab, and adds the
//! 				radar's field of view edges.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfoHudExamples : SCR_ScriptedWidgetComponent
{
	protected static const int PREVIEW_HEADING = 45;			//!< compass heading the examples show
	protected static const float PREVIEW_STAMINA = 0.75;
	protected static const float AUTO_HIDDEN_OPACITY = 0.2;
	protected static const int GROUP_ROWS = 3;					//!< group display rows the examples fill
	protected static const float RADAR_ICON_SIZE = 16;			//!< px at 100% RADAR_ICON_SIZE, same as CSI_Radar.ICON_WIDTH_AND_HEIGHT
	protected static const int RADAR_ICON_COUNT = 4;			//!< radar icons the examples can show at once
	protected static const float ARROW_DEMO_ICON_SIZE = 40;
	protected static const float ARROW_DEMO_HEADING = 60;

	// Squadmates in the radar range and field of view examples: facings and placeholder colour teams
	protected static const ref array<float> DEMO_MATE_FACINGS = {10, -30, 120, 200};
	protected static const ref array<CSI_EColorTeam> DEMO_MATE_TEAMS = {CSI_EColorTeam.RED, CSI_EColorTeam.BLUE, CSI_EColorTeam.GREEN, CSI_EColorTeam.YELLOW};
	protected static const float RADAR_RANGE_PX = 48.44;		//!< px from the centre CSI_Radar puts the edge of its range (132 / 2.725)

	// Radar range examples: squadmates at these bearings (degrees from straight ahead, + to the right) and real distances
	// (metres), placed like CSI_Radar for the selected range; those beyond it aren't on the radar
	protected static const ref array<float> RANGE_MATE_BEARINGS = {-30, 50, 160, -110};
	protected static const ref array<float> RANGE_MATE_DISTANCES_ON_FOOT = {8, 18, 35, 70};
	protected static const ref array<float> RANGE_MATE_DISTANCES_IN_VEHICLE = {1, 2.5, 5, 8};

	// Radar field of view example: squadmates at these bearings and distances (px from the centre)
	protected static const ref array<float> FOV_MATE_BEARINGS = {0, -40, 95, 165};
	protected static const ref array<float> FOV_MATE_DISTANCES = {34, 26, 30, 36};
	protected static const float FOV_FADED_OPACITY = 0.15;		//!< squadmates outside the field of view, shown fading out
	protected static const float FOV_EDGE_START = 8;			//!< px from the centre to the first dot of each edge
	protected static const float FOV_EDGE_LENGTH = 46;			//!< px from the centre to the last dot, just inside the radar's range (CSI_Radar puts the range ~48 px out)
	protected static const int FOV_EDGE_DOTS = 8;				//!< dots per edge
	protected static const float FOV_EDGE_DOT_SIZE = 3;			//!< px
	protected static const ResourceName FOV_EDGE_DOT_TEXTURE = "{73B3D8BBB785B5B9}UI/Textures/Common/circleFull.edds";	//!< vanilla filled circle

	protected Widget m_wSettingsRoot;	//!< CSI settings tab root, for the selected values

	protected Widget m_wHud;
	protected Widget m_wCompass;
	protected ImageWidget m_wCompassImage;
	protected TextWidget m_wBearing;
	protected Widget m_wGroup;
	protected Widget m_wStamina;
	protected ProgressBarWidget m_wStaminaBar;
	protected CSI_Stamina m_Stamina;
	protected Widget m_wRadarFrame;
	protected ref array<Widget> m_aGroupRows = {};
	protected ref array<Widget> m_aRadarIcons = {};				//!< created in the HUD's radar frame, like CSI_Radar does
	protected ref array<Widget> m_aRadarFieldOfViewDots = {};	//!< both field of view edges' dots, created on first show

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Same lookups as CSI_HUD and its elements
		m_wHud = w.FindAnyWidget("Hud");
		if (!m_wHud)
			return;

		m_wStamina = m_wHud.FindAnyWidget("Stamina");
		if (m_wStamina)
		{
			m_wStaminaBar = ProgressBarWidget.Cast(m_wStamina.FindAnyWidget("StamBar"));
			m_Stamina = CSI_Stamina.Cast(m_wStamina.FindHandler(CSI_Stamina));
		}

		m_wCompass = m_wHud.FindAnyWidget("Compass");
		if (m_wCompass)
		{
			m_wCompassImage = ImageWidget.Cast(m_wCompass.FindAnyWidget("Compass"));
			m_wBearing = TextWidget.Cast(m_wCompass.FindAnyWidget("Bearing"));
			m_wRadarFrame = m_wCompass.FindAnyWidget("RadarFrame");
		}

		m_wGroup = m_wHud.FindAnyWidget("Group");
		if (m_wGroup)
		{
			for (int rowIndex = 0; rowIndex < GROUP_ROWS; rowIndex++)
			{
				Widget row = m_wGroup.FindAnyWidget("Player" + rowIndex);
				if (row)
					m_aGroupRows.Insert(row);
			}
		}

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!m_wRadarFrame || !workspace)
			return;

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
	//! \param[in] settingsRoot CSI settings tab root
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Shows the example for a setting, if this layout has one.
	//! \param[in] setting CSI_GameSettings name
	//! \param[out] caption text under the example (may be empty)
	//! \return false if this layout has no example for the setting
	bool ShowExample(string setting, out string caption)
	{
		int value = GetValue(setting);

		// General
		if (setting == CSI_GameSettings.AUTO_HIDE_HUD)
		{
			if (value)
			{
				ShowConfiguredHud(AUTO_HIDDEN_OPACITY);
				caption = "Hidden until you hold Reveal UI";
				return true;
			}

			ShowConfiguredHud(1);
			caption = "Always shown";
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
			ShowHud(true, value, false, 1);
			if (!value)
				caption = "Hidden";

			return true;
		}

		// Compass & Radar
		if (setting == CSI_GameSettings.COMPASS_THEME || setting == CSI_GameSettings.COMPASS_VISIBLE || setting == CSI_GameSettings.BEARING_VISIBLE)
		{
			ShowHud(true, false, false, 1);
			if (setting != CSI_GameSettings.COMPASS_THEME && !value)
				caption = "Hidden";

			return true;
		}

		if (setting == CSI_GameSettings.RADAR_VISIBLE || setting == CSI_GameSettings.RADAR_ICON_SIZE)
		{
			ShowHud(true, false, false, 1);
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
			ShowHud(true, false, false, 1);
			if (!GetValue(CSI_GameSettings.RADAR_VISIBLE))
			{
				caption = "Needs Radar Visible";
				return true;
			}

			bool inVehicle = setting == CSI_GameSettings.RADAR_RANGE_IN_VEHICLE;
			int range;
			if (inVehicle)
				range = Math.ClampInt(value, CSI_GameSettings.RADAR_RANGE_IN_VEHICLE_MIN, CSI_GameSettings.RADAR_RANGE_IN_VEHICLE_MAX);
			else
				range = Math.ClampInt(value, CSI_GameSettings.RADAR_RANGE_ON_FOOT_MIN, CSI_GameSettings.RADAR_RANGE_ON_FOOT_MAX);

			ShowRadarRange(range, inVehicle);
			if (inVehicle)
				caption = string.Format("In vehicles, squadmates within %1 m show on the radar", range);
			else
				caption = string.Format("On foot, squadmates within %1 m show on the radar", range);

			return true;
		}

		if (setting == CSI_GameSettings.RADAR_FIELD_OF_VIEW)
		{
			ShowHud(true, false, false, 1);
			if (!GetValue(CSI_GameSettings.RADAR_VISIBLE))
			{
				caption = "Needs Radar Visible";
				return true;
			}

			int fieldOfView = Math.ClampInt(value, CSI_GameSettings.RADAR_FIELD_OF_VIEW_MIN, CSI_GameSettings.RADAR_FIELD_OF_VIEW_MAX);
			ShowRadarFieldOfView(fieldOfView);
			if (fieldOfView >= CSI_GameSettings.RADAR_FIELD_OF_VIEW_MAX)
				caption = "Every squadmate on the radar is shown";
			else
				caption = string.Format("Squadmates outside the %1° ahead slowly fade out", fieldOfView);

			return true;
		}

		if (setting == CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE)
		{
			ShowHud(true, false, false, 1);
			ShowArrowsRotateDemo(value);
			if (value)
				caption = "Only the arrow turns; the icon stays upright";
			else
				caption = "The whole icon turns";

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void HideExample()
	{
		if (m_wRoot)
			m_wRoot.SetVisible(false);

		foreach (Widget radarIcon : m_aRadarIcons)
			radarIcon.SetVisible(false);

		foreach (Widget dot : m_aRadarFieldOfViewDots)
			dot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! The HUD as the tab sets it up: compass, plus group display and stamina bar if they're enabled.
	//! \param[in] opacity
	protected void ShowConfiguredHud(float opacity)
	{
		ShowHud(true, GetValue(CSI_GameSettings.GROUP_VISIBLE), GetValue(CSI_GameSettings.STAMINA_VISIBLE), opacity);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows HUD elements filled in with the tab's settings.
	//! \param[in] showCompass compass element (compass and bearing follow their own settings)
	//! \param[in] showGroup
	//! \param[in] showStamina
	//! \param[in] opacity
	protected void ShowHud(bool showCompass, bool showGroup, bool showStamina, float opacity)
	{
		if (!m_wHud)
			return;

		m_wRoot.SetVisible(true);
		m_wHud.SetOpacity(opacity);

		if (m_wCompass)
		{
			m_wCompass.SetVisible(showCompass);
			if (showCompass)
				UpdateCompass();
		}

		if (m_wGroup)
		{
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
	//! Sets what CSI_Compass.Update would for a PREVIEW_HEADING heading, with the tab's compass and bearing settings.
	protected void UpdateCompass()
	{
		if (m_wCompassImage)
		{
			m_wCompassImage.LoadImageTexture(0, CSI_UIHelper.CSI_STANDARD_COMPASS);	// only the standard compass exists so far
			m_wCompassImage.SetRotation(-PREVIEW_HEADING);
			m_wCompassImage.SetVisible(GetValue(CSI_GameSettings.COMPASS_VISIBLE));
		}

		if (!m_wBearing)
			return;

		int heading = PREVIEW_HEADING;
		if (GetValue(CSI_GameSettings.BEARING_VISIBLE))
			m_wBearing.SetText(heading.ToString(3));
		else
			m_wBearing.SetText(string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills the first group display rows like CSI_Player and CSI_Icon do: the local group's members (or the local
	//! player and placeholders outside a group), each with their icon and colour team.
	protected void UpdateGroup()
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, GROUP_ROWS);

		// Placeholder colour teams show how teams are coloured when there's no real group
		array<CSI_EColorTeam> placeholderTeams = {CSI_EColorTeam.RED, CSI_EColorTeam.RED, CSI_EColorTeam.BLUE};

		foreach (int i, Widget row : m_aGroupRows)
		{
			int playerID = 0;
			if (memberIDs.IsIndexValid(i))
				playerID = memberIDs[i];

			string fallbackName = "Squadmate";
			if (i == 0)
				fallbackName = SCR_Global.GetProfileName();

			CSI_EColorTeam colorTeam = CSI_SettingInfo.GetPlayerColorTeam(playerID, placeholderTeams[i]);

			TextWidget name = TextWidget.Cast(row.FindAnyWidget("PlayerName"));
			if (name)
			{
				name.SetText(CSI_SettingInfo.GetPreviewName(m_wSettingsRoot, playerID, fallbackName));
				name.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));
			}

			// Keep the row's own icon size (0)
			CSI_SettingInfo.ShowIcon(m_wSettingsRoot, row.FindAnyWidget("Icon"), 0, CSI_SettingInfo.GetPlayerIcon(playerID), colorTeam, 0, 0, 1);
			row.SetVisible(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the stamina bar like CSI_Stamina does at PREVIEW_STAMINA, fully faded in.
	protected void UpdateStamina()
	{
		if (m_Stamina)
			m_Stamina.OnStaminaChange(PREVIEW_STAMINA);

		if (m_wStaminaBar)
			m_wStaminaBar.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Two group members on the radar, placed and sized like CSI_Radar with the tab's radar icon size.
	protected void ShowRadarIcons()
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, GROUP_ROWS);

		// Leave the local player out, they're the centre of the radar
		memberIDs.RemoveItem(SCR_PlayerController.GetLocalPlayerId());

		float iconSize = RADAR_ICON_SIZE * GetValue(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01;
		bool onlyArrowsRotate = GetValue(CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE);
		array<float> positionsX = {-22, 19};
		array<float> positionsY = {-17, 14};
		array<float> rotations = {30, -60};
		array<CSI_EColorTeam> placeholderTeams = {CSI_EColorTeam.RED, CSI_EColorTeam.BLUE};

		for (int i = 0; i < 2; i++)
		{
			int playerID = 0;
			if (memberIDs.IsIndexValid(i))
				playerID = memberIDs[i];

			float iconRotation = rotations[i];
			if (onlyArrowsRotate)
				iconRotation = 0;

			Widget icon = GetRadarIcon(i);
			PlaceInRadar(icon, positionsX[i], positionsY[i]);
			CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, iconSize, CSI_SettingInfo.GetPlayerIcon(playerID), CSI_SettingInfo.GetPlayerColorTeam(playerID, placeholderTeams[i]), iconRotation, rotations[i], 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Squadmates at set real distances around the radar, placed and faded like CSI_Radar for the given range: the range
	//! always reaches the radar's edge, so a longer range draws everyone closer to the centre. Those beyond it aren't shown.
	//! \param[in] range metres
	//! \param[in] inVehicle use the in-vehicle distances (a few metres) instead of the on-foot ones
	protected void ShowRadarRange(int range, bool inVehicle)
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, RADAR_ICON_COUNT + 1);

		// Leave the local player out, they're the centre of the radar
		memberIDs.RemoveItem(SCR_PlayerController.GetLocalPlayerId());

		array<float> distances = RANGE_MATE_DISTANCES_ON_FOOT;
		if (inVehicle)
			distances = RANGE_MATE_DISTANCES_IN_VEHICLE;

		float iconSize = RADAR_ICON_SIZE * GetValue(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01;
		bool onlyArrowsRotate = GetValue(CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE);

		// Same distance fade as CSI_Radar: over the last 20% of the range
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

			int playerID = 0;
			if (memberIDs.IsIndexValid(mateIndex))
				playerID = memberIDs[mateIndex];

			float iconRotation = DEMO_MATE_FACINGS[mateIndex];
			if (onlyArrowsRotate)
				iconRotation = 0;

			float opacity = Math.Clamp(Math.Map(distance, fadeStart, range, 0.6, 0), 0, 1);

			vector position = GetRadarPoint(bearing, distance * RADAR_RANGE_PX / range);
			PlaceInRadar(icon, position[0], position[1]);
			CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, iconSize, CSI_SettingInfo.GetPlayerIcon(playerID), CSI_SettingInfo.GetPlayerColorTeam(playerID, DEMO_MATE_TEAMS[mateIndex]), iconRotation, DEMO_MATE_FACINGS[mateIndex], opacity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Squadmates around the radar with dotted edges at half the field of view either side of straight ahead (the top,
	//! like CSI_Radar); those outside are shown part way through fading out.
	//! \param[in] fieldOfView RADAR_FIELD_OF_VIEW value (degrees)
	protected void ShowRadarFieldOfView(int fieldOfView)
	{
		array<int> memberIDs = {};
		CSI_SettingInfo.GetPreviewGroupMembers(memberIDs, RADAR_ICON_COUNT + 1);

		// Leave the local player out, they're the centre of the radar
		memberIDs.RemoveItem(SCR_PlayerController.GetLocalPlayerId());

		float halfFieldOfView = fieldOfView * 0.5;
		bool everyoneShown = fieldOfView >= CSI_GameSettings.RADAR_FIELD_OF_VIEW_MAX;
		float iconSize = RADAR_ICON_SIZE * GetValue(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01;
		bool onlyArrowsRotate = GetValue(CSI_GameSettings.RADAR_ICON_ARROWS_ROTATE);

		foreach (int mateIndex, float bearing : FOV_MATE_BEARINGS)
		{
			int playerID = 0;
			if (memberIDs.IsIndexValid(mateIndex))
				playerID = memberIDs[mateIndex];

			float iconRotation = DEMO_MATE_FACINGS[mateIndex];
			if (onlyArrowsRotate)
				iconRotation = 0;

			float opacity = 1;
			if (!everyoneShown && Math.AbsFloat(bearing) > halfFieldOfView)
				opacity = FOV_FADED_OPACITY;

			vector position = GetRadarPoint(bearing, FOV_MATE_DISTANCES[mateIndex]);
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

			float distance = Math.Lerp(FOV_EDGE_START, FOV_EDGE_LENGTH, indexOnEdge / (FOV_EDGE_DOTS - 1.0));
			vector dotPos = GetRadarPoint(side * halfFieldOfView, distance);
			FrameSlot.SetPos(dot, dotPos[0], dotPos[1]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates the radar field of view edge dots once, in the radar frame like the icons, the way vanilla
	//! SCR_BaseAnalogGauge.CreateRingMark creates its marks.
	protected void CreateRadarFieldOfViewDots()
	{
		if (!m_aRadarFieldOfViewDots.IsEmpty() || !m_wRadarFrame)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		// Same orange as the nametag field of view example's edges
		Color edgeColor = new Color(0.7605, 0.3916, 0.0782, 1);

		for (int i = 0; i < FOV_EDGE_DOTS * 2; i++)
		{
			ImageWidget dot = ImageWidget.Cast(workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.BLEND | WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.INHERIT_CLIPPING, edgeColor, 0, m_wRadarFrame));
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
	//! \param[in] bearing degrees from straight ahead (the top of the radar), positive to the right
	//! \param[in] distance px from the radar's centre
	//! \return the point (x, y) from the radar's centre
	protected vector GetRadarPoint(float bearing, float distance)
	{
		float radians = bearing * Math.DEG2RAD;
		return Vector(Math.Sin(radians) * distance, -Math.Cos(radians) * distance, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! A large AT icon in the middle of the radar facing ARROW_DEMO_HEADING, rotated the way CSI_Icon.SetRotation does.
	//! \param[in] onlyArrowsRotate
	protected void ShowArrowsRotateDemo(bool onlyArrowsRotate)
	{
		float iconRotation = ARROW_DEMO_HEADING;
		if (onlyArrowsRotate)
			iconRotation = 0;

		// AT's launcher shows clearly which way the icon is turned (MAN looks the same at any angle)
		Widget icon = GetRadarIcon(0);
		PlaceInRadar(icon, 0, 0);
		CSI_EColorTeam colorTeam = CSI_SettingInfo.GetPlayerColorTeam(SCR_PlayerController.GetLocalPlayerId(), CSI_EColorTeam.NONE);
		CSI_SettingInfo.ShowIcon(m_wSettingsRoot, icon, ARROW_DEMO_ICON_SIZE, CSI_EIcon.AT, colorTeam, iconRotation, ARROW_DEMO_HEADING, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] icon radar icon
	//! \param[in] x px from the radar's centre
	//! \param[in] y px from the radar's centre
	protected void PlaceInRadar(Widget icon, float x, float y)
	{
		if (!icon)
			return;

		FrameSlot.SetAnchor(icon, 0.5, 0.5);
		FrameSlot.SetAlignment(icon, 0.5, 0.5);
		FrameSlot.SetPos(icon, x, y);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	//! \return the radar icon widget, null if it couldn't be created
	protected Widget GetRadarIcon(int index)
	{
		if (!m_aRadarIcons.IsIndexValid(index))
			return null;

		return m_aRadarIcons[index];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting
	//! \return the value selected in the settings tab
	protected int GetValue(string setting)
	{
		return CSI_SettingInfo.GetValue(m_wSettingsRoot, setting);
	}
}
