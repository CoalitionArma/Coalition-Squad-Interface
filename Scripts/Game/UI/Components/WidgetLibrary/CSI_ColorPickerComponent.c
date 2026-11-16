//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Color picker (CSI_ColorPicker.layout): a multiline toolbox with one color box per CSI_EColorPreset,
//! 				so its selected index is the color. Boxes fill their tile, the selected one gets an orange ring,
//! 				and blocked colors are faded and crossed out (SetColorBlocked).
//!
//------------------------------------------------------------------------------------------------
class CSI_ColorPickerComponent : SCR_ToolboxMultilineComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	// Solid white image, tinted per box
	protected static const ResourceName SOLID_IMAGESET = "{ACEFA05B746C8798}ui/imagesets/debugUI.imageset";
	protected static const string SOLID_IMAGE = "Pixel_White_100";

	// X over colors that can't be picked (SetColorBlocked)
	protected static const ResourceName BLOCKED_MARK_IMAGESET = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	protected static const string BLOCKED_MARK_IMAGE = "cancel";
	protected static const float BLOCKED_COLOR_OPACITY = 0.3;

	// Inside each box (MapColorSelectorEntry.layout)
	protected const string BOX_CONTENT_WIDGET_NAME = "Overlay";		//!< tile, outline and color, padded in by default
	protected const string BOX_TILE_WIDGET_NAME = "Background";		//!< orange while selected, clear otherwise
	protected const string BOX_OUTLINE_WIDGET_NAME = "Border";		//!< orange outline while hovered
	protected const string BLOCKED_MARK_WIDGET_NAME = "BlockedMark";

	protected static const float BOX_TILE_OUTSET = 2;	//!< px of the selected tile shown round the color

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// One box per color, before super creates them
		m_aElementNames = {};
		for (int i, count = CSI_ColorPresets.Count(); i < count; i++)
		{
			m_aElementNames.Insert(CSI_ColorPresets.GetName(i));
		}

		super.HandlerAttached(w);

		// Focus shows as the row's LineHighlight, as on the settings tab's other rows
		if (m_wBorder)
			m_wBorder.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Styles every box each time the boxes are built.
	override protected void CreateWidgets()
	{
		super.CreateWidgets();

		foreach (int preset, SCR_ButtonBaseComponent box : m_aSelectionElements)
		{
			if (!StyleBox(box, CSI_ColorPresets.GetColor(preset)))
			{
				Print("CSI_ColorPickerComponent: m_ElementLayout's boxes aren't image buttons, they won't show their colors", LogLevel.WARNING);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] box
	//! \param[in] color
	//! \return false if it isn't an image button
	protected bool StyleBox(notnull SCR_ButtonBaseComponent box, notnull Color color)
	{
		// Without this the toolbox neither shows nor reports a selection
		box.SetToggleable(true);

		// Transparent tile, so only the selected box shows one
		box.SetBackgroundColors(UIColors.TRANSPARENT, UIColors.TRANSPARENT);
		box.ColorizeBackground(false);

		Widget boxRoot = box.GetRootWidget();
		if (boxRoot)
			LayOutBox(boxRoot);

		SCR_ButtonImageComponent imageBox = SCR_ButtonImageComponent.Cast(box);
		if (!imageBox)
			return false;

		imageBox.SetImage(SOLID_IMAGESET, SOLID_IMAGE);
		ImageWidget image = imageBox.GetImageWidget();
		if (!image)
			return false;

		image.SetColor(color);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Square box of m_fElementHeight, centered in its grid cell, with the color filling it.
	//! \param[in] boxRoot
	protected void LayOutBox(notnull Widget boxRoot)
	{
		// Only the picker takes focus, so a gamepad moves between rows and steps through the boxes with left and right
		boxRoot.SetFlags(WidgetFlags.NOFOCUS);

		UniformGridSlot.SetHorizontalAlign(boxRoot, LayoutHorizontalAlign.Center);
		UniformGridSlot.SetVerticalAlign(boxRoot, LayoutVerticalAlign.Center);

		SizeLayoutWidget boxSize = SizeLayoutWidget.Cast(boxRoot.GetChildren());
		if (boxSize)
		{
			boxSize.EnableWidthOverride(true);
			boxSize.SetWidthOverride(m_fElementHeight);
			boxSize.EnableHeightOverride(true);
			boxSize.SetHeightOverride(m_fElementHeight);
		}

		// The color fills the whole box, the hover outline sits on its edge
		Widget content = boxRoot.FindAnyWidget(BOX_CONTENT_WIDGET_NAME);
		if (content)
			AlignableSlot.SetPadding(content, 0, 0, 0, 0);

		Widget outline = boxRoot.FindAnyWidget(BOX_OUTLINE_WIDGET_NAME);
		if (outline)
			AlignableSlot.SetPadding(outline, 0, 0, 0, 0);

		// The selected tile sticks out round the color as an orange ring
		Widget tile = boxRoot.FindAnyWidget(BOX_TILE_WIDGET_NAME);
		if (tile)
			AlignableSlot.SetPadding(tile, -BOX_TILE_OUTSET, -BOX_TILE_OUTSET, -BOX_TILE_OUTSET, -BOX_TILE_OUTSET);
	}

	//------------------------------------------------------------------------------------------------
	//! X over the box, shown while its color can't be picked. Created on first use.
	//! \param[in] boxRoot
	//! \return null if the box has no BOX_CONTENT_WIDGET_NAME
	protected Widget CreateBlockedMark(notnull Widget boxRoot)
	{
		Widget content = boxRoot.FindAnyWidget(BOX_CONTENT_WIDGET_NAME);
		if (!content)
			return null;

		ImageWidget blockedMark = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNORE_CURSOR, Color.White, 0, content));
		if (!blockedMark)
			return null;

		blockedMark.SetName(BLOCKED_MARK_WIDGET_NAME);
		blockedMark.LoadImageFromSet(0, BLOCKED_MARK_IMAGESET, BLOCKED_MARK_IMAGE);
		AlignableSlot.SetHorizontalAlign(blockedMark, LayoutHorizontalAlign.Stretch);
		AlignableSlot.SetVerticalAlign(blockedMark, LayoutVerticalAlign.Stretch);
		return blockedMark;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 BLOCKED COLORS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Fades and crosses out a color and disables its box, which left and right then skip.
	//! \param[in] preset CSI_EColorPreset value
	//! \param[in] blocked
	void SetColorBlocked(int preset, bool blocked)
	{
		SCR_ButtonBaseComponent box = GetItem(preset);
		if (!box)
			return;

		box.SetEnabled(!blocked, false);

		Widget boxRoot = box.GetRootWidget();
		if (boxRoot)
		{
			Widget blockedMark = boxRoot.FindAnyWidget(BLOCKED_MARK_WIDGET_NAME);
			if (!blockedMark && blocked)
				blockedMark = CreateBlockedMark(boxRoot);

			if (blockedMark)
				blockedMark.SetVisible(blocked);
		}

		SCR_ButtonImageComponent imageBox = SCR_ButtonImageComponent.Cast(box);
		if (!imageBox || !imageBox.GetImageWidget())
			return;

		if (blocked)
			imageBox.GetImageWidget().SetOpacity(BLOCKED_COLOR_OPACITY);
		else
			imageBox.GetImageWidget().SetOpacity(1);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] name widget name
	//! \param[in] parent
	//! \param[in] searchAllChildren
	//! \return null if not found
	static CSI_ColorPickerComponent GetColorPicker(string name, Widget parent, bool searchAllChildren = true)
	{
		return CSI_ColorPickerComponent.Cast(SCR_WLibComponentBase.GetComponent(CSI_ColorPickerComponent, name, parent, searchAllChildren));
	}
}
