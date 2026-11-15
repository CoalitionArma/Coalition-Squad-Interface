//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	The colours a colour setting can pick (CSI_EColorPreset), and the settings tab's colour box rows
//! 				for CSI_GameSettings.COLOR_SETTINGS. Read a setting's colour with GetSettingColor.
//!
//------------------------------------------------------------------------------------------------
class CSI_ColorPresets
{
	// Solid white image, tinted per box
	protected static const ResourceName SOLID_IMAGESET = "{ACEFA05B746C8798}ui/imagesets/debugUI.imageset";
	protected static const string SOLID_IMAGE = "Pixel_White_100";

	protected static const float BOX_INSET = 4;	//!< px between a box's colour and its tile

	//------------------------------------------------------------------------------------------------
	//! \param[in] preset CSI_EColorPreset value
	//! \return a new colour, white for an unknown value
	static Color GetColor(int preset)
	{
		switch (preset)
		{
			case CSI_EColorPreset.AMBER :	return new Color(0.947, 0.651, 0.136, 1);
			case CSI_EColorPreset.ORANGE :	return new Color(0.761, 0.392, 0.078, 1);	// the UI orange
			case CSI_EColorPreset.RED :		return new Color(0.855, 0.027, 0.028, 1);
			case CSI_EColorPreset.GREEN :	return new Color(0.014, 0.445, 0.08, 1);
			case CSI_EColorPreset.BLUE :	return new Color(0.002, 0.198, 0.847, 1);
			case CSI_EColorPreset.CYAN :	return new Color(0.014, 0.546, 0.896, 1);
			case CSI_EColorPreset.PINK :	return new Color(0.871, 0.039, 0.705, 1);
			case CSI_EColorPreset.PURPLE :	return new Color(0.45, 0.08, 0.86, 1);
		}

		return new Color(1, 1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] preset CSI_EColorPreset value
	//! \return name shown to players, empty for an unknown value
	static string GetName(int preset)
	{
		switch (preset)
		{
			case CSI_EColorPreset.WHITE :	return "White";
			case CSI_EColorPreset.AMBER :	return "Amber";
			case CSI_EColorPreset.ORANGE :	return "Orange";
			case CSI_EColorPreset.RED :		return "Red";
			case CSI_EColorPreset.GREEN :	return "Green";
			case CSI_EColorPreset.BLUE :	return "Blue";
			case CSI_EColorPreset.CYAN :	return "Cyan";
			case CSI_EColorPreset.PINK :	return "Pink";
			case CSI_EColorPreset.PURPLE :	return "Purple";
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \return number of colours (values 0 to Count() - 1)
	static int Count()
	{
		array<int> presets = {};
		return SCR_Enum.GetEnumValues(CSI_EColorPreset, presets);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting a CSI_GameSettings.COLOR_SETTINGS name
	//! \return the setting's colour, white without a settings manager
	static Color GetSettingColor(string setting)
	{
		CSI_SettingsManager settingsManager = CSI_SettingsManager.GetInstance();
		if (!settingsManager)
			return GetColor(CSI_EColorPreset.WHITE);

		return GetColor(settingsManager.GetSettingInt(setting));
	}

	//------------------------------------------------------------------------------------------------
	//! Fills the toolbox with one box per colour, so its selected index is the colour.
	//! Call before its value is loaded.
	//! \param[in] toolbox with MapColorSelectorEntry.layout elements
	//! \return false if its elements aren't image buttons
	static bool FillToolbox(notnull SCR_ToolboxComponent toolbox)
	{
		toolbox.ClearAll();
		for (int i, count = Count(); i < count; i++)
		{
			// AddItem rebuilds every element, so style them afterwards
			toolbox.AddItem(GetName(i));
		}

		array<SCR_ButtonBaseComponent> boxes = {};
		toolbox.GetItems(boxes);
		bool allImages = true;
		foreach (int preset, SCR_ButtonBaseComponent box : boxes)
		{
			if (!StyleBox(box, GetColor(preset)))
				allImages = false;
		}

		return allImages;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] box one toolbox element
	//! \param[in] color
	//! \return false if it isn't an image button
	protected static bool StyleBox(notnull SCR_ButtonBaseComponent box, notnull Color color)
	{
		// Without this the toolbox neither shows nor reports a selection
		box.SetToggleable(true);

		// Small boxes side by side, not stretched across the row
		Widget boxRoot = box.GetRootWidget();
		if (boxRoot)
		{
			LayoutSlot.SetSizeMode(boxRoot, LayoutSizeMode.Auto);
			LayoutSlot.SetHorizontalAlign(boxRoot, LayoutHorizontalAlign.Left);
			LayoutSlot.SetVerticalAlign(boxRoot, LayoutVerticalAlign.Center);
		}

		SCR_ButtonImageComponent imageBox = SCR_ButtonImageComponent.Cast(box);
		if (!imageBox)
			return false;

		imageBox.SetImage(SOLID_IMAGESET, SOLID_IMAGE);
		ImageWidget image = imageBox.GetImageWidget();
		if (!image)
			return false;

		image.SetColor(color);

		// Inset so the tile shows round it: dark, or orange when selected
		Widget imageFrame = image.GetParent();
		if (imageFrame)
			AlignableSlot.SetPadding(imageFrame, BOX_INSET, BOX_INSET, BOX_INSET, BOX_INSET);

		return true;
	}
}
