//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Nametag examples (CSI_SettingInfoNametags.layout) for the Nametags settings and Rank Visible. The layout
//! 				has one panel per kind of example (Single, DayNight, Range, Simplified, LineOfSight, BodyPosition,
//! 				FieldOfView); this shows one, fills in the local player's nametag with the tab's settings, and moves
//! 				the widgets that depend on values (range split, tag height, field of view edges and shaded zones).
//! 				Missing panels are logged as warnings when it's created.
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
	protected static const float NAMETAG_DIAGRAM_SCALE = 1.4;	//!< Range, Simplified, LineOfSight and FieldOfView: next to the figures
	protected static const float NAMETAG_BODY_SCALE = 2.24;		//!< BodyPosition: over the zoomed-in figure
	protected static const float NIGHT_DIMMED_OPACITY = 0.35;	//!< DayNight: the night tag with brightness dimming on

	// Simplified: where the squadmates stand along the range, as fractions of the panel's width
	protected static const float ZONE_MARGIN = 0.06;			//!< left for the "You" figure at the start
	protected static const float MATE_SPACING = 0.08;			//!< closest a mate stands to the figure before them
	protected static const float FIGURE_HALF_WIDTH = 0.02;		//!< half a figure's width

	// BodyPosition: the figure (task-man.edds) and the bones nametags attach to, as standing heights in metres
	protected static const float FIGURE_HEIGHT = 1.8;			//!< the character in the figure image, head to feet
	protected static const float FIGURE_FEET = 0.986;			//!< the character's feet, as a fraction of the image height (head at the top)
	protected static const float TORSO_CENTRE_HEIGHT = 1.07;	//!< middle of the torso: task-man.edds measured, shoulders 1.44 m to hips 0.69 m
	protected static const float NECK_BONE_HEIGHT = 1.5;		//!< approximately, the Neck1 bone BODY tags attach to
	protected static const float HEAD_BONE_HEIGHT = 1.62;		//!< approximately, the head bone HEAD tags attach to
	protected static const float BODY_TAG_TOP_MARGIN = 6;		//!< px: closest the tag's top gets to the panel's top

	// FieldOfView, seen from above: px from "You" unless noted
	protected static const float FOV_EDGE_START = 30;			//!< to the first dot of each edge
	protected static const float FOV_EDGE_LENGTH = 215;			//!< to the last dot of each edge (as long as ViewLine)
	protected static const int FOV_EDGE_DOTS = 21;				//!< dots per edge
	protected static const float FOV_EDGE_DOT_SIZE = 5.6;		//!< px: dot width and height
	protected static const ResourceName FOV_EDGE_DOT_TEXTURE = "{73B3D8BBB785B5B9}UI/Textures/Common/circleFull.edds";	//!< vanilla filled circle
	protected static const int FOV_ZONE_ARC_STEPS = 16;			//!< straight pieces making up each shaded zone's curved edge
	protected static const float FOV_ZONE_OPACITY = 0.3;		//!< shaded zones' opacity
	protected static const float FOV_FULL_MATE_DISTANCE = 157;	//!< to the squadmate with a full nametag
	protected static const float FOV_ICON_MATE_DISTANCE = 83.5;	//!< to the icon-only squadmate (closer, so its tag stays below the other dot)
	protected static const float FOV_TAG_GAP = 9.8;				//!< from a squadmate's dot centre up to the bottom of their nametag

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
		// Pending retries of DrawFieldOfViewCanvas and PlaceBodyPositionTag
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(DrawFieldOfViewCanvas);
			GetGame().GetCallqueue().Remove(PlaceBodyPositionTag);
		}

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] settingsRoot CSI settings tab root
	void SetSettingsRoot(Widget settingsRoot)
	{
		m_wSettingsRoot = settingsRoot;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 EXAMPLES
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Shows the example for a setting, if this layout has one.
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
			int fieldOfView = Math.ClampInt(GetValue(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW), CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_MIN, CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_MAX);

			// The field of view on its own; the shaded zones and squadmates belong to the simplified percentage
			if (setting == CSI_GameSettings.NAMETAG_FIELD_OF_VIEW)
			{
				ShowFieldOfView(fieldOfView, 0, false);
				caption = string.Format("Up close: %1° either side of where you look, narrowing with distance", fieldOfView * 0.5);
				return true;
			}

			// Like in game, nothing is simplified without role icons
			int simplifiedPercentage = Math.ClampInt(GetValue(CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED), 0, CSI_GameSettings.NAMETAG_FIELD_OF_VIEW_SIMPLIFIED_MAX);
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
	protected void ShowSingle()
	{
		Widget panel = ShowPanel(SINGLE_PANEL);
		if (panel)
			ShowSampleNametag(panel.FindAnyWidget("Tag"), NAMETAG_PREVIEW_SCALE, 1, true);
	}

	//------------------------------------------------------------------------------------------------
	//! DayNight panel: the same nametag by day and by night, the night one faded with brightness dimming.
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
	//! \param[in] range metres
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
	//! Simplified panel: its Axis is the whole range; NearAxis and the split widgets move to where tags drop to
	//! the role icon only.
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
		// The name zone; none at 100%, where it would only be a sliver at "You"
		Widget nearAxis = panel.FindAnyWidget("NearAxis");
		if (nearAxis)
		{
			FrameSlot.SetAnchorMax(nearAxis, splitX, FrameSlot.GetAnchorMax(nearAxis)[1]);
			nearAxis.SetVisible(!simplifies || percentage < 100);
		}

		SetChildText(panel, "RangeLabel", string.Format("%1 m", range));

		// Each mate stands in the middle of their own zone, but at least MATE_SPACING after the figure before them, so a
		// mate in a narrow zone stands at its edge instead of disappearing. The named mate is only left out when that
		// would put them past the split, i.e. names never show (100%). The full zone starts after the "You" figure.
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

		// No split at 100%: every tag is icon only ("Always icon")
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
	//! Distance where nametags drop to icon only, using the same formula as SCR_NTElementBase.ShouldWeClearText
	//! (squared distance against the near zone, start 1 m).
	//! \param[in] percentage NAMTEAG_RANGE_SIMPLIFIED value
	//! \param[in] range metres
	//! \return metres; range or more means never within range
	protected float GetSimplifiedDistance(int percentage, float range)
	{
		float cutoff = range * (1 - percentage * 0.01);
		return Math.Sqrt(1 + cutoff * (range - 1));
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 BODY POSITION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! BodyPosition panel: a see-through Figure zoomed to the upper body; the tag moves to where
	//! CSI_SCR_NameTagData.UpdateTagPos puts it for the tab's position and offset (PlaceBodyPositionTag).
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
	//! BodyPosition: centres the tag (as SCR_NameTagRulesetBase centres it on its bone) at GetTagHeight on the figure as
	//! it's drawn. Measured on screen from the Figure's Image, so it follows however the figure layout is arranged:
	//! the character's head is at the image's top and their feet at FIGURE_FEET of its height, which stands for
	//! FIGURE_HEIGHT. Waits for the panel to be laid out, like DrawFieldOfViewCanvas.
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

		// Held inside the example: high HEAD offsets would otherwise push the tag above the panel's top
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
	//! Where the tab's NAMETAG_POSITION and NAMETAG_POSITION_OFFSET put the tag on a standing character, with the offsets
	//! of CSI_SCR_NameTagData.UpdateTagPos: HEAD = head bone + 0.1-0.6 m, BODY = Neck1 bone - 0.5-0 m.
	//! \return metres above the feet
	protected float GetTagHeight()
	{
		int offset = GetValue(CSI_GameSettings.NAMETAG_POSITION_OFFSET);
		if (IsHeadPosition())
			return HEAD_BONE_HEIGHT + (offset + 1) * 0.1;

		return NECK_BONE_HEIGHT + (offset - 5) * 0.1;
	}

	//------------------------------------------------------------------------------------------------
	//! \return where NAMETAG_POSITION and NAMETAG_POSITION_OFFSET put the tag. BODY is measured from the middle of the
	//! torso (TORSO_CENTRE_HEIGHT), to the nearest 5 cm.
	protected string GetTagPositionCaption()
	{
		if (IsHeadPosition())
		{
			int offset = GetValue(CSI_GameSettings.NAMETAG_POSITION_OFFSET);
			return string.Format("%1 cm above the head", (offset + 1) * 10);
		}

		int fromTorso = Math.Round((GetTagHeight() - TORSO_CENTRE_HEIGHT) * 20) * 5;
		if (fromTorso == 0)
			return "At the middle of the torso";

		if (fromTorso > 0)
			return string.Format("%1 cm above the middle of the torso", fromTorso);

		return string.Format("%1 cm below the middle of the torso", -fromTorso);
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the tab's NAMETAG_POSITION is HEAD
	protected bool IsHeadPosition()
	{
		return SCR_Enum.GetEnumName(ENameTagPosition, GetValue(CSI_GameSettings.NAMETAG_POSITION)) == "HEAD";
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 FIELD OF VIEW
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! FieldOfView panel, seen from above: dotted edges at half the angle either side of the view line, like
	//! SCR_NameTagRulesetFriendlies (m_iMaxAngle / 2 up close). For the simplified percentage, the simplified outer part
	//! is shaded with a squadmate in each part. Positions are offsets from the panel's bottom centre; "You" stands where
	//! the layout puts it.
	//! \param[in] fieldOfView NAMETAG_FIELD_OF_VIEW value (degrees)
	//! \param[in] simplifiedPercentage NAMETAG_FIELD_OF_VIEW_SIMPLIFIED value, 0 for no shaded zones
	//! \param[in] showSquadmates show the squadmates with full and icon-only nametags (simplified percentage example)
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

		// Edges as dotted lines: rotated image widgets didn't draw here, positioned dots do
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

		// Like the Simplified range example: a squadmate in the clear middle (left of the view line) with their full
		// nametag, and one in the middle of a shaded zone (right) with the role icon only, when there is one
		float innerAngle = halfAngle * (1 - simplifiedPercentage * 0.01);
		vector fullPos = GetPointAround(youX, youY, -innerAngle * 0.5, FOV_FULL_MATE_DISTANCE);
		vector iconPos = GetPointAround(youX, youY, (innerAngle + halfAngle) * 0.5, FOV_ICON_MATE_DISTANCE);
		ShowFieldOfViewMate(panel, "FullMate", "FullTag", fullPos, true, showSquadmates);
		ShowFieldOfViewMate(panel, "IconMate", "IconTag", iconPos, false, showSquadmates && simplifiedPercentage > 0);
	}

	//------------------------------------------------------------------------------------------------
	//! FieldOfView: a squadmate's dot at a point, with their nametag centred above it.
	//! \param[in] panel FieldOfView panel
	//! \param[in] mateName dot widget
	//! \param[in] tagName nametag widget
	//! \param[in] position offset from the panel's bottom centre
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

		// Centred over the dot (the tag's alignment is its bottom centre)
		FrameSlot.SetPos(tag, position[0], position[1] - FOV_TAG_GAP);
		ShowSampleNametag(tag, NAMETAG_DIAGRAM_SCALE, 1, showName);
	}

	//------------------------------------------------------------------------------------------------
	//! Draws the FieldOfView panel's view line and shaded simplified zones (the outer part of each side of the field of
	//! view) on its canvas, with polygons like vanilla SCR_SpiderNet. The line is drawn here instead of by the ViewLine
	//! widget so it sits exactly between the zones: the canvas and widgets land on screen pixels slightly differently.
	//! ViewLine stays in the layout (hidden at runtime) and gives the line its position, length, width and color.
	//! The canvas works in screen pixels, so layout units are converted with WorkspaceWidget.DPIScale.
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

		// Not laid out yet (the panel was just shown): try again shortly, like vanilla SCR_SpiderNet.DrawSpiderNet.
		// Until then the ViewLine widget still shows the line.
		if (width <= 0 || height <= 0)
		{
			GetGame().GetCallqueue().CallLater(DrawFieldOfViewCanvas, 10);
			return;
		}

		viewLine.SetVisible(false);

		// ViewLine starts at "You" and points straight ahead; its position is an offset from the panel's bottom centre
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

			// Vanilla's Reforger orange like the edges, see-through (a copy, as vanilla UIColors.CopyColor advises)
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
	//! Creates the FieldOfView edge dots once, anchored like "You" (panel bottom centre), the way vanilla
	//! SCR_BaseAnalogGauge.CreateRingMark creates its marks.
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
			// Vanilla's Reforger orange, as the line of sight example's sight line
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
	//! \param[in] distance px from the centre
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
	//! The local player's nametag, using the tab's nametag scale, role icon, rank and group settings.
	//! \param[in] nametag CSI_Nametag.layout instance
	//! \param[in] previewScale size multiplier over the in-game size
	//! \param[in] opacity
	//! \param[in] showName false for an icon-only (simplified) tag
	protected void ShowSampleNametag(Widget nametag, float previewScale, float opacity, bool showName)
	{
		if (!nametag)
			return;

		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();

		// Same sizes as SCR_NTElementBase.GetNametagTextScale / GetNametagImageScale, scaled for the example
		int scaleStep = Math.ClampInt(GetValue(CSI_GameSettings.NAMTEAG_SCALE) / 20, 0, 5);
		int textSize = (8 + scaleStep) * previewScale;
		int groupTextSize = (6 + scaleStep) * previewScale;
		int iconSize = (12 + scaleStep * 2) * previewScale;
		Color textColor = CSI_UIHelper.ConvertColorTeamToColor(CSI_EColorTeam.NONE);

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
			groupWidget.SetVisible(showName && GetValue(CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE));
			groupWidget.SetText("Alpha 1-1");
			groupWidget.SetExactFontSize(groupTextSize);
			groupWidget.SetColor(textColor);
			groupWidget.SetOpacity(opacity);
		}

		bool showIcon = GetValue(CSI_GameSettings.ROLE_IN_NAMETAG_VISIBLE);
		int iconPosition = GetValue(CSI_GameSettings.NAMETAG_ROLE_ICON_POSITION);
		CSI_EIcon role = CSI_SettingInfo.GetPlayerIcon(localPlayerID);

		// Icon-only tags always show their icon, like simplified tags in game
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
			if (!visible)
				continue;

			// Same images as CSI_SCR_NTIconBase: vanilla pointer for MAN, simple CSI icons otherwise
			if (role == CSI_EIcon.MAN)
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
	//! Shows the layout with only the given panel, filling it. The layout stacks the panels one below another so each
	//! can be edited in Workbench.
	//! \param[in] panelName one of PANELS
	//! \return the panel, null if the layout doesn't have it (warned about in HandlerAttached)
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
	//! \param[in] panel
	//! \param[in] childName
	//! \param[in] text
	protected void SetChildText(notnull Widget panel, string childName, string text)
	{
		TextWidget textWidget = TextWidget.Cast(panel.FindAnyWidget(childName));
		if (textWidget)
			textWidget.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] panel
	//! \param[in] childName
	//! \param[in] visible
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
}
