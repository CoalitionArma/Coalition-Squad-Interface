//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Nametag examples (CSI_SettingInfoNametags.layout) for the Nametags settings and Rank Visible: shows the
//! 				setting's panel with the local player's nametag, laid out and colored for the tab's values.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfoNametagExamples : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	// Panels in CSI_SettingInfoNametags.layout, one per kind of example
	protected static const string SINGLE_PANEL = "Single";
	protected static const string DAY_NIGHT_PANEL = "DayNight";
	protected static const string RANGE_PANEL = "Range";
	protected static const string SIMPLIFIED_PANEL = "Simplified";
	protected static const string LINE_OF_SIGHT_PANEL = "LineOfSight";
	protected static const string BODY_POSITION_PANEL = "BodyPosition";
	protected static const string FIELD_OF_VIEW_PANEL = "FieldOfView";
	protected static const ref array<string> PANELS = {SINGLE_PANEL, DAY_NIGHT_PANEL, RANGE_PANEL, SIMPLIFIED_PANEL, LINE_OF_SIGHT_PANEL, BODY_POSITION_PANEL, FIELD_OF_VIEW_PANEL};

	// Nametags are drawn this much larger than in game
	protected static const float NAMETAG_PREVIEW_SCALE = 4.9;	//!< Single: a nametag on its own
	protected static const float NAMETAG_COMPARE_SCALE = 2.8;	//!< DayNight: two nametags side by side
	protected static const float NAMETAG_DIAGRAM_SCALE = 1.4;	//!< for the Range, Simplified, LineOfSight and FieldOfView panels
	protected static const float NAMETAG_BODY_SCALE = 2.24;		//!< BodyPosition: over the zoomed-in figure
	protected static const float NIGHT_DIMMED_OPACITY = 0.35;	//!< DayNight: the night tag with brightness dimming on

	// Simplified: positions along the range, as fractions of the panel's width
	protected static const float ZONE_MARGIN = 0.06;			//!< left for the "You" figure at the start
	protected static const float MATE_SPACING = 0.08;			//!< closest a mate stands to the figure before them
	protected static const float FIGURE_HALF_WIDTH = 0.02;

	// BodyPosition: figure (task-man.edds) and bone heights when standing, in meters
	protected static const float FIGURE_HEIGHT = 1.8;			//!< the character in the figure image, head to feet
	protected static const float FIGURE_FEET = 0.986;			//!< feet, as a fraction of the image height from the top
	protected static const float TORSO_CENTER_HEIGHT = 1.07;	//!< between shoulders (1.44 m) and hips (0.69 m)
	protected static const float NECK_BONE_HEIGHT = 1.5;		//!< approximately, the Neck1 bone BODY tags attach to
	protected static const float HEAD_BONE_HEIGHT = 1.62;		//!< approximately, the head bone HEAD tags attach to
	protected static const float BODY_TAG_TOP_MARGIN = 6;		//!< px: closest the tag's top gets to the panel's top

	// FieldOfView, seen from above: px from "You" unless noted
	protected static const float FOV_EDGE_START = 30;			//!< to the first dot of each edge
	protected static const float FOV_EDGE_LENGTH = 215;			//!< to the last dot of each edge (as long as ViewLine)
	protected static const int FOV_EDGE_DOTS = 21;				//!< dots per edge
	protected static const float FOV_EDGE_DOT_SIZE = 5.6;		//!< px: dot width and height
	protected static const ResourceName FOV_EDGE_DOT_TEXTURE = "{73B3D8BBB785B5B9}UI/Textures/Common/circleFull.edds";
	protected static const int FOV_ZONE_ARC_STEPS = 16;			//!< segments per shaded zone's curved edge
	protected static const float FOV_ZONE_OPACITY = 0.3;
	protected static const float FOV_FULL_MATE_DISTANCE = 157;	//!< to the squadmate with a full nametag
	protected static const float FOV_ICON_MATE_DISTANCE = 83.5;	//!< to the icon-only squadmate (tag stays below the other dot)
	protected static const float FOV_TAG_GAP = 9.8;				//!< from a squadmate's dot center up to their nametag

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected Widget m_wSettingsRoot;	//!< CSI settings tab root, for the selected values
	protected ref array<Widget> m_aPanels = {};

	// FieldOfView
	protected ref array<Widget> m_aFieldOfViewEdgeDots = {};	//!< both edges' dots, created on first show
	protected ref array<ref CanvasWidgetCommand> m_aFieldOfViewCanvasCommands = {};	//!< view line and shaded zones, kept while the canvas draws them
	protected int m_iFieldOfViewAngle;					//!< angle the canvas is drawn for (degrees)
	protected int m_iFieldOfViewSimplifiedPercentage;	//!< simplified percentage the canvas is drawn for

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		foreach (string panelName : PANELS)
		{
			Widget panel = w.FindAnyWidget(panelName);
			if (panel)
				m_aPanels.Insert(panel);
			else
				PrintFormat("CSI_SettingInfoNametagExamples: panel '%1' not found, its examples won't show", panelName, level: LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		// Pending retries
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(DrawFieldOfViewCanvas);
			GetGame().GetCallqueue().Remove(PlaceBodyPositionTag);
		}

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
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

		if (setting == CSI_GameSettings.RANK_VISIBLE || setting == CSI_GameSettings.NAMTEAG_SCALE || setting == CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE || setting == CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION)
		{
			ShowSingle();
			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_VISIBLE)
		{
			if (value)
				ShowSingle();
			else
				caption = "Nametags hidden";

			return true;
		}

		if (setting == CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE)
		{
			ShowSingle();
			if (value)
				caption = "Group shown under the name";
			else
				caption = "No group names";

			return true;
		}

		if (setting == CSI_GameSettings.SQUADMATE_GROUP_IN_NAMETAG_VISIBLE)
		{
			ShowSingle(true);
			if (value)
				caption = "A squadmate's nametag, with their group";
			else
				caption = "A squadmate's nametag, without their group";

			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_COLOR)
		{
			ShowSingle();
			caption = string.Format("Players outside your group and AI: %1", CSI_ColorPresets.GetName(GetNonClashingPreset(setting)));
			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_VON_COLOR)
		{
			ShowSingle(false, true);
			caption = string.Format("While talking: %1", CSI_ColorPresets.GetName(GetNonClashingPreset(setting)));
			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_BRIGHTNESS_DIMMING)
		{
			ShowDayNight(value);
			if (value)
				caption = "Faded at night, like the scene around it";
			else
				caption = "Same brightness day and night";

			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_RANGE)
		{
			ShowRange(value);
			caption = string.Format("Shown up to %1 m", value);
			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_MAGNIFICATION_MULTIPLICATION)
		{
			int magnifiedRange = GetValue(CSI_GameSettings.NAMETAG_RANGE) * value / 100;
			ShowRange(magnifiedRange);
			caption = string.Format("Through scopes and binoculars: up to %1 m", magnifiedRange);
			return true;
		}

		if (setting == CSI_GameSettings.NAMTEAG_RANGE_SIMPLIFIED)
		{
			ShowSimplifiedRange(value);
			caption = GetSimplifiedRangeCaption(value);
			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_LOS_VISIBLE)
		{
			ShowLineOfSight(value);
			if (value)
				caption = "Hidden behind hills, walls and cover";
			else
				caption = "Shown through hills, walls and cover";

			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_POSITION || setting == CSI_GameSettings.NAMETAG_POSITION_OFFSET)
		{
			ShowBodyPosition();
			caption = GetTagPositionCaption();
			return true;
		}

		if (setting == CSI_GameSettings.NAMETAG_FIELD_OF_VIEW || setting == CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED)
		{
			int fieldOfView = CSI_GameSettings.ClampToLimits(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW, GetValue(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW));

			// No shaded zones or squadmates: those show the simplified percentage
			if (setting == CSI_GameSettings.NAMETAG_FIELD_OF_VIEW)
			{
				ShowFieldOfView(fieldOfView, 0, false);
				caption = string.Format("Up close: %1° either side of where you look, narrowing with distance", fieldOfView * 0.5);
				return true;
			}

			// Nothing is simplified without role icons
			int simplifiedPercentage = CSI_GameSettings.ClampToLimits(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED, GetValue(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED));
			if (!GetValue(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE))
				simplifiedPercentage = 0;

			ShowFieldOfView(fieldOfView, simplifiedPercentage, true);
			caption = GetFieldOfViewSimplifiedCaption(simplifiedPercentage);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void HideExample()
	{
		if (m_wRoot)
			m_wRoot.SetVisible(false);

		foreach (Widget panel : m_aPanels)
		{
			panel.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Single panel: the local player's nametag on its own.
	//! \param[in] squadmate shown as a squadmate's nametag, else as another player's
	protected void ShowSingle(bool squadmate = false, bool talking = false)
	{
		Widget panel = ShowPanel(SINGLE_PANEL);
		if (panel)
			ShowSampleNametag(panel.FindAnyWidget("Tag"), NAMETAG_PREVIEW_SCALE, 1, true, squadmate, talking);
	}

	//------------------------------------------------------------------------------------------------
	//! DayNight panel: the nametag by day and by night.
	//! \param[in] dimming NAMETAG_BRIGHTNESS_DIMMING value
	protected void ShowDayNight(bool dimming)
	{
		Widget panel = ShowPanel(DAY_NIGHT_PANEL);
		if (!panel)
			return;

		float nightOpacity = 1;
		if (dimming)
			nightOpacity = NIGHT_DIMMED_OPACITY;

		ShowSampleNametag(panel.FindAnyWidget("DayTag"), NAMETAG_COMPARE_SCALE, 1, true);
		ShowSampleNametag(panel.FindAnyWidget("NightTag"), NAMETAG_COMPARE_SCALE, nightOpacity, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Range panel: a teammate inside the range has a nametag, one beyond it doesn't.
	//! \param[in] range meters
	protected void ShowRange(int range)
	{
		Widget panel = ShowPanel(RANGE_PANEL);
		if (!panel)
			return;

		SetChildText(panel, "RangeLabel", string.Format("%1 m", range));
		ShowSampleNametag(panel.FindAnyWidget("Tag"), NAMETAG_DIAGRAM_SCALE, 1, true);
	}

	//------------------------------------------------------------------------------------------------
	//! LineOfSight panel: with line of sight checks the hill hides the teammate's nametag.
	//! \param[in] losEnabled
	protected void ShowLineOfSight(bool losEnabled)
	{
		Widget panel = ShowPanel(LINE_OF_SIGHT_PANEL);
		if (!panel)
			return;

		SetChildVisible(panel, "SightBlocked", losEnabled);
		SetChildVisible(panel, "HiddenLabel", losEnabled);
		SetChildVisible(panel, "SightClear", !losEnabled);

		Widget tag = panel.FindAnyWidget("Tag");
		if (losEnabled)
		{
			if (tag)
				tag.SetVisible(false);

			return;
		}

		ShowSampleNametag(tag, NAMETAG_DIAGRAM_SCALE, 1, true);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SIMPLIFIED RANGE
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Simplified panel: marks where along the range tags drop to the role icon only.
	//! \param[in] percentage NAMTEAG_RANGE_SIMPLIFIED value
	protected void ShowSimplifiedRange(int percentage)
	{
		Widget panel = ShowPanel(SIMPLIFIED_PANEL);
		if (!panel)
			return;

		int range = GetValue(CSI_GameSettings.NAMETAG_RANGE);
		float distance = range;
		if (GetValue(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE) && percentage > 0)
			distance = Math.Min(GetSimplifiedDistance(percentage, range), range);

		bool simplifies = distance < range;

		// Distances map onto the Axis widget as it's laid out
		float startX = 0;
		float endX = 1;
		Widget axis = panel.FindAnyWidget("Axis");
		if (axis)
		{
			startX = FrameSlot.GetAnchorMin(axis)[0];
			endX = FrameSlot.GetAnchorMax(axis)[0];
		}

		float splitX = Math.Lerp(startX, endX, distance / range);
		// The name zone, hidden at 100%
		Widget nearAxis = panel.FindAnyWidget("NearAxis");
		if (nearAxis)
		{
			FrameSlot.SetAnchorMax(nearAxis, splitX, FrameSlot.GetAnchorMax(nearAxis)[1]);
			nearAxis.SetVisible(!simplifies || percentage < 100);
		}

		SetChildText(panel, "RangeLabel", string.Format("%1 m", range));

		// Each mate stands mid-zone but at least MATE_SPACING past the figure before them;
		// the named mate is hidden only if that puts them past the split (100%)
		float fullZoneStart = startX + ZONE_MARGIN;
		float fullX = Math.Max(Math.Lerp(fullZoneStart, splitX, 0.5), startX + MATE_SPACING);
		bool showFull = fullX - FIGURE_HALF_WIDTH <= splitX;
		SetChildVisible(panel, "FullMate", showFull);
		SetChildVisible(panel, "FullTag", showFull);
		if (showFull)
		{
			MoveChildToX(panel, "FullMate", fullX);
			MoveChildToX(panel, "FullTag", fullX);
			ShowSampleNametag(panel.FindAnyWidget("FullTag"), NAMETAG_DIAGRAM_SCALE, 1, true);
		}

		// No split at 100%: every tag is icon only
		bool showSplit = simplifies && percentage < 100;
		SetChildVisible(panel, "TickSplit", showSplit);
		SetChildVisible(panel, "SplitLabel", showSplit);
		SetChildVisible(panel, "IconMate", simplifies);
		SetChildVisible(panel, "IconTag", simplifies);

		if (!simplifies)
			return;

		int roundedDistance = Math.Round(distance);
		MoveChildToX(panel, "TickSplit", splitX);
		MoveChildToX(panel, "SplitLabel", splitX);
		SetChildText(panel, "SplitLabel", string.Format("%1 m", roundedDistance));

		float figureBeforeX = startX;
		if (showFull)
			figureBeforeX = fullX;

		float iconOnlyX = Math.Max(Math.Lerp(splitX, endX, 0.5), figureBeforeX + MATE_SPACING);
		MoveChildToX(panel, "IconMate", iconOnlyX);
		MoveChildToX(panel, "IconTag", iconOnlyX);
		ShowSampleNametag(panel.FindAnyWidget("IconTag"), NAMETAG_DIAGRAM_SCALE, 1, false);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] percentage NAMTEAG_RANGE_SIMPLIFIED value
	//! \return
	protected string GetSimplifiedRangeCaption(int percentage)
	{
		if (!GetValue(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE))
			return "Role icons are hidden";

		if (percentage <= 0)
			return "Never simplified";

		if (percentage >= 100)
			return "Always icon";

		float range = GetValue(CSI_GameSettings.NAMETAG_RANGE);
		float distance = GetSimplifiedDistance(percentage, range);
		if (distance >= range)
			return "Never simplified within range";

		int roundedDistance = Math.Round(distance);
		return string.Format("Icon only beyond %1 m", roundedDistance);
	}

	//------------------------------------------------------------------------------------------------
	//! Distance where nametags drop to icon only.
	//! \param[in] percentage NAMTEAG_RANGE_SIMPLIFIED value
	//! \param[in] range meters
	//! \return meters; range or more means never within range
	protected float GetSimplifiedDistance(int percentage, float range)
	{
		float cutoff = range * (1 - percentage * 0.01);
		return Math.Sqrt(1 + cutoff * (range - 1));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 BODY POSITION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! BodyPosition panel: the tag on the zoomed-in figure at the tab's position and offset.
	protected void ShowBodyPosition()
	{
		Widget panel = ShowPanel(BODY_POSITION_PANEL);
		if (!panel)
			return;

		Widget tag = panel.FindAnyWidget("Tag");
		if (!tag)
			return;

		ShowSampleNametag(tag, NAMETAG_BODY_SCALE, 1, true);
		PlaceBodyPositionTag();
	}

	//------------------------------------------------------------------------------------------------
	//! Centers the tag at GetTagHeight on the figure, measured on screen from its Image.
	//! Retries until the panel is laid out.
	protected void PlaceBodyPositionTag()
	{
		GetGame().GetCallqueue().Remove(PlaceBodyPositionTag);

		if (!m_wRoot)
			return;

		Widget panel = m_wRoot.FindAnyWidget(BODY_POSITION_PANEL);
		if (!panel || !panel.IsVisible())
			return;

		Widget tag = panel.FindAnyWidget("Tag");
		Widget figure = panel.FindAnyWidget("Figure");
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!tag || !figure || !workspace)
			return;

		Widget image = figure.FindAnyWidget("Image");
		if (!image)
			return;

		float panelX, panelY, panelWidth, panelHeight;
		panel.GetScreenPos(panelX, panelY);
		panel.GetScreenSize(panelWidth, panelHeight);

		float imageX, imageY, imageWidth, imageHeight;
		image.GetScreenPos(imageX, imageY);
		image.GetScreenSize(imageWidth, imageHeight);

		float pixelsPerUnit = workspace.DPIScale(1);
		if (panelHeight <= 0 || imageHeight <= 0 || pixelsPerUnit <= 0)
		{
			GetGame().GetCallqueue().CallLater(PlaceBodyPositionTag, 10);
			return;
		}

		float bodyPixels = imageHeight * FIGURE_FEET;
		float tagScreenY = imageY + bodyPixels - GetTagHeight() / FIGURE_HEIGHT * bodyPixels;

		// Keep high HEAD offsets inside the panel
		float tagWidth, tagScreenHeight;
		tag.GetScreenSize(tagWidth, tagScreenHeight);
		if (tagScreenHeight <= 0)
		{
			GetGame().GetCallqueue().CallLater(PlaceBodyPositionTag, 10);
			return;
		}

		tagScreenY = Math.Max(tagScreenY, panelY + tagScreenHeight * 0.5 + BODY_TAG_TOP_MARGIN * pixelsPerUnit);

		// The tag is anchored to the panel's bottom
		FrameSlot.SetPosY(tag, (tagScreenY - (panelY + panelHeight)) / pixelsPerUnit);
	}

	//------------------------------------------------------------------------------------------------
	//! Tag height on a standing character: HEAD = head bone + 0.1-0.6 m, BODY = Neck1 bone - 0.5-0 m.
	//! \return meters above the feet
	protected float GetTagHeight()
	{
		int offset = GetValue(CSI_GameSettings.NAMETAG_POSITION_OFFSET);
		if (IsHeadPosition())
			return HEAD_BONE_HEIGHT + (offset + 1) * 0.1;

		return NECK_BONE_HEIGHT + (offset - 5) * 0.1;
	}

	//------------------------------------------------------------------------------------------------
	//! \return where the tag sits; BODY from the torso's middle, to the nearest 5 cm
	protected string GetTagPositionCaption()
	{
		if (IsHeadPosition())
		{
			int offset = GetValue(CSI_GameSettings.NAMETAG_POSITION_OFFSET);
			return string.Format("%1 cm above the head", (offset + 1) * 10);
		}

		int fromTorso = Math.Round((GetTagHeight() - TORSO_CENTER_HEIGHT) * 20) * 5;
		if (fromTorso == 0)
			return "At the middle of the torso";

		if (fromTorso > 0)
			return string.Format("%1 cm above the middle of the torso", fromTorso);

		return string.Format("%1 cm below the middle of the torso", -fromTorso);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsHeadPosition()
	{
		return SCR_Enum.GetEnumName(ENameTagPosition, GetValue(CSI_GameSettings.NAMETAG_POSITION)) == "HEAD";
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 FIELD OF VIEW
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! FieldOfView panel, seen from above: dotted edges at half the angle either side of the view line.
	//! \param[in] fieldOfView NAMETAG_FIELD_OF_VIEW value (degrees)
	//! \param[in] simplifiedPercentage NAMETAG_FIELD_OF_VIEW_SIMPLIFIED value, 0 for no shaded zones
	//! \param[in] showSquadmates show the full and icon-only squadmates
	protected void ShowFieldOfView(int fieldOfView, int simplifiedPercentage, bool showSquadmates)
	{
		Widget panel = ShowPanel(FIELD_OF_VIEW_PANEL);
		if (!panel)
			return;

		m_iFieldOfViewAngle = fieldOfView;
		m_iFieldOfViewSimplifiedPercentage = simplifiedPercentage;
		DrawFieldOfViewCanvas();

		float halfAngle = fieldOfView * 0.5;
		SetChildText(panel, "AngleLabel", string.Format("%1° field of view", fieldOfView));

		vector youPos;
		Widget you = panel.FindAnyWidget("You");
		if (you)
			youPos = FrameSlot.GetPos(you);

		float youX = youPos[0];
		float youY = youPos[1];

		CreateFieldOfViewEdgeDots(panel);
		foreach (int i, Widget dot : m_aFieldOfViewEdgeDots)
		{
			// First half of the dots is the left edge, second half the right
			float side = -1;
			int indexOnEdge = i;
			if (i >= FOV_EDGE_DOTS)
			{
				side = 1;
				indexOnEdge = i - FOV_EDGE_DOTS;
			}

			float distance = Math.Lerp(FOV_EDGE_START, FOV_EDGE_LENGTH, indexOnEdge / (FOV_EDGE_DOTS - 1.0));
			vector dotPos = GetPointAround(youX, youY, side * halfAngle, distance);
			FrameSlot.SetPos(dot, dotPos[0], dotPos[1]);
		}

		// Full-tag squadmate in the clear middle, icon-only one in a shaded zone
		float innerAngle = halfAngle * (1 - simplifiedPercentage * 0.01);
		vector fullPos = GetPointAround(youX, youY, -innerAngle * 0.5, FOV_FULL_MATE_DISTANCE);
		vector iconPos = GetPointAround(youX, youY, (innerAngle + halfAngle) * 0.5, FOV_ICON_MATE_DISTANCE);
		ShowFieldOfViewMate(panel, "FullMate", "FullTag", fullPos, true, showSquadmates);
		ShowFieldOfViewMate(panel, "IconMate", "IconTag", iconPos, false, showSquadmates && simplifiedPercentage > 0);
	}

	//------------------------------------------------------------------------------------------------
	//! FieldOfView: a squadmate's dot at a point, with their nametag centered above it.
	//! \param[in] panel FieldOfView panel
	//! \param[in] mateName dot widget
	//! \param[in] tagName nametag widget
	//! \param[in] position offset from the panel's bottom center
	//! \param[in] showName false for an icon-only tag
	//! \param[in] visible
	protected void ShowFieldOfViewMate(notnull Widget panel, string mateName, string tagName, vector position, bool showName, bool visible)
	{
		Widget mate = panel.FindAnyWidget(mateName);
		if (mate)
		{
			mate.SetVisible(visible);
			FrameSlot.SetPos(mate, position[0], position[1]);
		}

		Widget tag = panel.FindAnyWidget(tagName);
		if (!tag)
			return;

		tag.SetVisible(visible);
		if (!visible)
			return;

		// Centered over the dot (the tag's alignment is its bottom center)
		FrameSlot.SetPos(tag, position[0], position[1] - FOV_TAG_GAP);
		ShowSampleNametag(tag, NAMETAG_DIAGRAM_SCALE, 1, showName);
	}

	//------------------------------------------------------------------------------------------------
	//! Draws the view line and shaded zones on the canvas, sized from the hidden ViewLine.
	//! The line is drawn here, not by ViewLine, so it sits exactly between the zones.
	protected void DrawFieldOfViewCanvas()
	{
		GetGame().GetCallqueue().Remove(DrawFieldOfViewCanvas);

		if (!m_wRoot)
			return;

		Widget panel = m_wRoot.FindAnyWidget(FIELD_OF_VIEW_PANEL);
		if (!panel || !panel.IsVisible())
			return;

		CanvasWidget canvas = CanvasWidget.Cast(panel.FindAnyWidget("Canvas"));
		Widget viewLine = panel.FindAnyWidget("ViewLine");
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!canvas || !viewLine || !workspace)
			return;

		float width, height;
		canvas.GetScreenSize(width, height);

		// Not laid out yet: retry shortly; ViewLine shows the line meanwhile
		if (width <= 0 || height <= 0)
		{
			GetGame().GetCallqueue().CallLater(DrawFieldOfViewCanvas, 10);
			return;
		}

		viewLine.SetVisible(false);

		// ViewLine starts at "You"; its position is offset from the panel's bottom center
		vector linePos = FrameSlot.GetPos(viewLine);
		vector lineSize = FrameSlot.GetSize(viewLine);
		float centerX = width * 0.5 + workspace.DPIScale(linePos[0]);
		float centerY = height + workspace.DPIScale(linePos[1]);
		float radius = workspace.DPIScale(lineSize[1]);

		m_aFieldOfViewCanvasCommands.Clear();

		if (m_iFieldOfViewSimplifiedPercentage > 0)
		{
			float halfAngle = m_iFieldOfViewAngle * 0.5;
			float innerAngle = halfAngle * (1 - m_iFieldOfViewSimplifiedPercentage * 0.01);

			// See-through Reforger orange
			Color zoneColor = UIColors.CopyColor(UIColors.CONTRAST_COLOR);
			zoneColor.SetA(FOV_ZONE_OPACITY);

			// One wedge per side, from the inner angle out to the edge
			array<float> sides = {-1, 1};
			foreach (float side : sides)
			{
				PolygonDrawCommand zone = new PolygonDrawCommand();
				zone.m_iColor = zoneColor.PackToInt();
				zone.m_Vertices = {centerX, centerY};

				for (int step = 0; step <= FOV_ZONE_ARC_STEPS; step++)
				{
					float angle = Math.Lerp(innerAngle, halfAngle, step / (float)FOV_ZONE_ARC_STEPS) * side;
					vector point = GetPointAround(centerX, centerY, angle, radius);
					zone.m_Vertices.Insert(point[0]);
					zone.m_Vertices.Insert(point[1]);
				}

				m_aFieldOfViewCanvasCommands.Insert(zone);
			}
		}

		// View line last, so it's drawn over the zones
		LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = viewLine.GetColor().PackToInt();
		line.m_fWidth = workspace.DPIScale(lineSize[0]);
		line.m_Vertices = {centerX, centerY, centerX, centerY - radius};
		m_aFieldOfViewCanvasCommands.Insert(line);

		canvas.SetDrawCommands(m_aFieldOfViewCanvasCommands);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates the edge dots once, anchored at the panel's bottom center.
	//! \param[in] panel FieldOfView panel
	protected void CreateFieldOfViewEdgeDots(notnull Widget panel)
	{
		if (!m_aFieldOfViewEdgeDots.IsEmpty())
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		for (int i = 0; i < FOV_EDGE_DOTS * 2; i++)
		{
			// Reforger orange
			ImageWidget dot = ImageWidget.Cast(workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.BLEND | WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.INHERIT_CLIPPING, UIColors.CONTRAST_COLOR, 1, panel));
			if (!dot)
				continue;

			dot.LoadImageTexture(0, FOV_EDGE_DOT_TEXTURE);
			FrameSlot.SetAnchorMin(dot, 0.5, 1);
			FrameSlot.SetAnchorMax(dot, 0.5, 1);
			FrameSlot.SetAlignment(dot, 0.5, 0.5);
			FrameSlot.SetSize(dot, FOV_EDGE_DOT_SIZE, FOV_EDGE_DOT_SIZE);
			m_aFieldOfViewEdgeDots.Insert(dot);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] percentage NAMETAG_FIELD_OF_VIEW_SIMPLIFIED value
	//! \return
	protected string GetFieldOfViewSimplifiedCaption(int percentage)
	{
		if (!GetValue(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE))
			return "Role icons are hidden";

		if (percentage <= 0)
			return "Never simplified";

		return string.Format("Shaded: icon only, the outer %1 percent of the field of view", percentage);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] centerX
	//! \param[in] centerY
	//! \param[in] angle degrees from straight up, positive to the right
	//! \param[in] distance px from the center
	//! \return the point (x, y)
	protected vector GetPointAround(float centerX, float centerY, float angle, float distance)
	{
		float radians = angle * Math.DEG2RAD;
		return Vector(centerX + Math.Sin(radians) * distance, centerY - Math.Cos(radians) * distance, 0);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SAMPLE NAMETAG
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Shows the local player's nametag with the tab's nametag settings.
	//! \param[in] nametag CSI_Nametag.layout instance
	//! \param[in] previewScale size multiplier over the in-game size
	//! \param[in] opacity
	//! \param[in] showName false for an icon-only (simplified) tag
	//! \param[in] squadmate shown as a squadmate's nametag (without a fire team), else as another player's
	//! \param[in] talking colored as while talking, with the talking bars
	protected void ShowSampleNametag(Widget nametag, float previewScale, float opacity, bool showName, bool squadmate = false, bool talking = false)
	{
		if (!nametag)
			return;

		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();

		// In-game text and icon sizes, scaled for the example
		int scaleStep = Math.ClampInt(GetValue(CSI_GameSettings.NAMTEAG_SCALE) / 20, 0, 5);
		int textSize = (8 + scaleStep) * previewScale;
		int groupTextSize = (6 + scaleStep) * previewScale;
		int iconSize = (12 + scaleStep * 2) * previewScale;
		Color groupColor = CSI_ColorPresets.GetColor(CSI_EColorPreset.GRAY);
		Color textColor;
		if (talking)
			textColor = CSI_ColorPresets.GetColor(GetNonClashingPreset(CSI_GameSettings.NAMETAG_VON_COLOR));
		else if (squadmate)
			textColor = CSI_ColorPresets.GetColor(GetNonClashingPreset(CSI_GameSettings.STANDARD_SQUAD_COLOR));
		else
			textColor = CSI_ColorPresets.GetColor(GetNonClashingPreset(CSI_GameSettings.NAMETAG_COLOR));

		TextWidget nameWidget = TextWidget.Cast(nametag.FindAnyWidget("PlayerName"));
		if (nameWidget)
		{
			nameWidget.SetVisible(showName);
			nameWidget.SetText(CSI_SettingInfo.GetPreviewName(m_wSettingsRoot, localPlayerID, CSI_SettingInfo.GetProfileName()));
			nameWidget.SetExactFontSize(textSize);
			nameWidget.SetColor(textColor);
			nameWidget.SetOpacity(opacity);
		}

		TextWidget groupWidget = TextWidget.Cast(nametag.FindAnyWidget("PlayerGroupName"));
		if (groupWidget)
		{
			bool showGroup = GetValue(CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE) && (!squadmate || GetValue(CSI_GameSettings.SQUADMATE_GROUP_IN_NAMETAG_VISIBLE));
			groupWidget.SetVisible(showName && showGroup);
			groupWidget.SetText("Alpha 1-1");
			groupWidget.SetExactFontSize(groupTextSize);
			groupWidget.SetColor(groupColor);
			groupWidget.SetOpacity(opacity);
		}

		bool showIcon = GetValue(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE);
		int iconPosition = GetValue(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION);
		CSI_EIcon role = CSI_SettingInfo.GetPlayerIcon(localPlayerID);

		// Icon-only tags always show their icon
		if (!showName)
			showIcon = true;

		array<int> positions = {CSI_ENametagIconPosition.LEFT, CSI_ENametagIconPosition.CENTER, CSI_ENametagIconPosition.RIGHT};
		foreach (int position : positions)
		{
			ImageWidget iconWidget = ImageWidget.Cast(nametag.FindAnyWidget("RoleIcon" + SCR_Enum.GetEnumName(CSI_ENametagIconPosition, position)));
			if (!iconWidget)
				continue;

			bool visible = showIcon && position == iconPosition;
			iconWidget.SetVisible(visible);

			// The talking bars take the role icon's place, as in game
			CSI_TalkingBarsComponent talkingBars;
			if (visible && talking)
				talkingBars = CSI_TalkingBarsComponent.GetOrCreate(iconWidget);
			else
				talkingBars = CSI_TalkingBarsComponent.Find(iconWidget);

			if (talkingBars)
				talkingBars.SetActive(visible && talking);

			if (!visible)
				continue;

			if (talking)
			{
				if (talkingBars)
					iconWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, "EMPTY");
				else
					iconWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "VON");
			}
			else if (role == CSI_EIcon.MAN)
			{
				iconWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
			}
			else
			{
				string iconName = CSI_SettingInfo.GetIconName(m_wSettingsRoot, role, true);
				if (role == CSI_EIcon.DRIVER)
					iconName = iconName + "_ALT";

				iconWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, iconName);
			}

			iconWidget.SetSize(iconSize, iconSize);
			iconWidget.SetColor(textColor);
			iconWidget.SetOpacity(opacity);
		}

		nametag.SetVisible(true);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 HELPERS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Shows the panel, stretched to fill the layout.
	//! \param[in] panelName one of PANELS
	//! \return the panel, null if the layout doesn't have it
	protected Widget ShowPanel(string panelName)
	{
		if (!m_wRoot)
			return null;

		Widget panel = m_wRoot.FindAnyWidget(panelName);
		if (!panel)
			return null;

		FrameSlot.SetAnchorMin(panel, 0, 0);
		FrameSlot.SetAnchorMax(panel, 1, 1);
		FrameSlot.SetOffsets(panel, 0, 0, 0, 0);

		m_wRoot.SetVisible(true);
		panel.SetVisible(true);
		return panel;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetChildText(notnull Widget panel, string childName, string text)
	{
		TextWidget textWidget = TextWidget.Cast(panel.FindAnyWidget(childName));
		if (textWidget)
			textWidget.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetChildVisible(notnull Widget panel, string childName, bool visible)
	{
		Widget child = panel.FindAnyWidget(childName);
		if (child)
			child.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a point-anchored child to another horizontal anchor, keeping its offsets.
	//! \param[in] panel
	//! \param[in] childName
	//! \param[in] anchorX fraction of the panel's width
	protected void MoveChildToX(notnull Widget panel, string childName, float anchorX)
	{
		Widget child = panel.FindAnyWidget(childName);
		if (child)
			FrameSlot.SetAnchor(child, anchorX, FrameSlot.GetAnchorMin(child)[1]);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting
	//! \return the value selected in the settings tab
	protected int GetValue(string setting)
	{
		return CSI_SettingInfo.GetValue(m_wSettingsRoot, setting);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting a CSI_GameSettings.NON_TEAM_COLOR_SETTINGS name
	//! \return the color nametags get with the tab's values: Gray if it matches a fire team color (the VON color:
	//! CSI_ColorPresets.GetVONPreset)
	protected int GetNonClashingPreset(string setting)
	{
		int teamSet = GetValue(CSI_GameSettings.COLOR_TEAM_SET);
		if (setting == CSI_GameSettings.NAMETAG_VON_COLOR)
			return CSI_ColorPresets.GetVONPreset(GetValue(setting), teamSet, GetValue(CSI_GameSettings.STANDARD_SQUAD_COLOR), GetValue(CSI_GameSettings.NAMETAG_COLOR));

		return CSI_ColorPresets.GetNonClashingPreset(GetValue(setting), teamSet);
	}
}
