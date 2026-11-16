//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	On each CSI settings tab row: tells CSI_SettingInfo which setting is hovered, focused or changed,
//! 				and previews a hovered color box.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfoHandler
{
	protected string m_sSetting;
	protected CSI_SettingInfo m_SettingInfo;	//!< not ref: CSI_SettingInfo owns this handler
	protected Widget m_wSetting;
	protected SCR_ModularButtonComponent m_RowButton;
	protected SCR_ChangeableComponentBase m_Changeable;
	protected SCR_ToolboxComponent m_ColorRow;	//!< color rows only

	//------------------------------------------------------------------------------------------------
	protected void OnRowMouseEnter(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		ShowThisSetting();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRowFocus(SCR_ModularButtonComponent comp)
	{
		ShowThisSetting();
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the new value a frame later: a clicked toolbox reports the change before selecting it.
	void OnValueChanged()
	{
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		callQueue.Remove(ShowThisSetting);
		callQueue.CallLater(ShowThisSetting);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowThisSetting()
	{
		if (m_SettingInfo)
			m_SettingInfo.ShowSetting(m_sSetting);
	}

	//------------------------------------------------------------------------------------------------
	//! Previews the hovered box's color, unless the row is locked.
	//! \param[in] w box root widget
	protected void OnColorBoxEntered(Widget w)
	{
		if (!m_SettingInfo || !m_wSetting || !m_wSetting.IsEnabled())
			return;

		int preset = FindColorBox(w);
		if (preset >= 0)
			m_SettingInfo.PreviewSetting(m_sSetting, preset);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnColorBoxLeft(Widget w)
	{
		if (m_SettingInfo)
			m_SettingInfo.EndPreview(m_sSetting, FindColorBox(w));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] w box root widget
	//! \return the box's index, its CSI_EColorPreset; -1 if not a box of this row
	protected int FindColorBox(Widget w)
	{
		if (!m_ColorRow)
			return -1;

		array<SCR_ButtonBaseComponent> boxes = {};
		m_ColorRow.GetItems(boxes);
		foreach (int preset, SCR_ButtonBaseComponent box : boxes)
		{
			if (box.GetRootWidget() == w)
				return preset;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] subscribe false to unsubscribe
	protected void SubscribeColorBoxes(bool subscribe)
	{
		array<SCR_ButtonBaseComponent> boxes = {};
		m_ColorRow.GetItems(boxes);
		foreach (SCR_ButtonBaseComponent box : boxes)
		{
			SCR_ButtonImageComponent imageBox = SCR_ButtonImageComponent.Cast(box);
			if (!imageBox)
				continue;

			if (subscribe)
			{
				imageBox.m_OnMouseEnter.Insert(OnColorBoxEntered);
				imageBox.m_OnMouseLeave.Insert(OnColorBoxLeft);
			}
			else
			{
				imageBox.m_OnMouseEnter.Remove(OnColorBoxEntered);
				imageBox.m_OnMouseLeave.Remove(OnColorBoxLeft);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Hooks the row's hover, focus and value changes, and each color box's hover.
	//! \param[in] settingWidget
	void Attach(notnull Widget settingWidget)
	{
		m_wSetting = settingWidget;

		m_RowButton = SCR_ModularButtonComponent.Cast(settingWidget.FindHandler(SCR_ModularButtonComponent));
		if (m_RowButton)
		{
			m_RowButton.m_OnMouseEnter.Insert(OnRowMouseEnter);
			m_RowButton.m_OnFocus.Insert(OnRowFocus);
		}
		else
		{
			PrintFormat("CSI_SettingInfoHandler: row '%1' has no SCR_ModularButtonComponent, hovering or focusing it won't show it in Setting Info", m_sSetting, level: LogLevel.WARNING);
		}

		m_Changeable = SCR_ChangeableComponentBase.Cast(settingWidget.FindHandler(SCR_ChangeableComponentBase));
		if (m_Changeable)
			m_Changeable.m_OnChanged.Insert(OnValueChanged);

		if (CSI_GameSettings.COLOR_SETTINGS.Contains(m_sSetting))
			m_ColorRow = SCR_ToolboxComponent.Cast(m_Changeable);

		if (m_ColorRow)
			SubscribeColorBoxes(true);
	}

	//------------------------------------------------------------------------------------------------
	void Detach()
	{
		GetGame().GetCallqueue().Remove(ShowThisSetting);

		if (m_RowButton)
		{
			m_RowButton.m_OnMouseEnter.Remove(OnRowMouseEnter);
			m_RowButton.m_OnFocus.Remove(OnRowFocus);
		}

		if (m_Changeable)
			m_Changeable.m_OnChanged.Remove(OnValueChanged);

		if (m_ColorRow)
			SubscribeColorBoxes(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \return the row's label, empty if none
	string GetLabelText()
	{
		// GetLabel needs the label widget, which rows without a label don't have
		if (!m_Changeable || !m_Changeable.GetLabelWidget())
			return string.Empty;

		TextWidget label = m_Changeable.GetLabel();
		if (!label)
			return string.Empty;

		return label.GetText();
	}

	//------------------------------------------------------------------------------------------------
	void CSI_SettingInfoHandler(string setting, CSI_SettingInfo settingInfo)
	{
		m_sSetting = setting;
		m_SettingInfo = settingInfo;
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	"Setting Info" panel of the CSI settings tab: the hovered setting's name, description and example
//! 				(SettingInfoExamples/), plus static helpers the examples share.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfo : SCR_ScriptedWidgetComponent
{
	protected TextWidget m_wName;
	protected TextWidget m_wCaption;
	protected TextWidget m_wDescription;

	protected ref array<ref CSI_SettingInfoHandler> m_aHandlers = {};
	protected ref map<string, string> m_mSettingLabels = new map<string, string>();	//!< setting -> its row's label

	// Example components, owned by their widgets
	protected CSI_SettingInfoHudExamples m_HudExamples;
	protected CSI_SettingInfoIconExamples m_IconExamples;
	protected CSI_SettingInfoNametagExamples m_NametagExamples;

	protected static string s_sProfileName;	//!< cached by GetProfileName

	// Previewed value, e.g. a hovered color box (PreviewSetting)
	protected static string s_sPreviewSetting;
	protected static int s_iPreviewValue;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wName = TextWidget.Cast(w.FindAnyWidget("SettingInfoName"));
		m_wCaption = TextWidget.Cast(w.FindAnyWidget("SettingInfoCaption"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("SettingInfoDescription"));
		// Layout editor: keep the examples visible so they can be edited
		if (SCR_Global.IsEditMode())
			return;

		if (!m_wName || !m_wCaption || !m_wDescription)
		{
			Print("CSI_SettingInfo: 'SettingInfoName', 'SettingInfoCaption' or 'SettingInfoDescription' not found, Setting Info won't show settings", LogLevel.WARNING);
			return;
		}

		FindExamples();
		ShowNothing();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		foreach (CSI_SettingInfoHandler handler : m_aHandlers)
			handler.Detach();

		m_aHandlers.Clear();
		m_mSettingLabels.Clear();
		s_sPreviewSetting = string.Empty;

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	//! Hooks every setting row, so hovering, focusing or changing one shows it here.
	//! \param[in] settingsRoot CSI settings tab root
	void SetSettingsRoot(Widget settingsRoot)
	{
		if (!settingsRoot || !m_wName || !m_wCaption || !m_wDescription)
			return;

		// Again, in case the examples attached after this component, warning about missing ones
		FindExamples(true);
		ShowNothing();

		if (m_HudExamples)
			m_HudExamples.SetSettingsRoot(settingsRoot);

		if (m_IconExamples)
			m_IconExamples.SetSettingsRoot(settingsRoot);

		if (m_NametagExamples)
			m_NametagExamples.SetSettingsRoot(settingsRoot);

		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			// Finds the setting row, not its override row (the list comes first)
			Widget settingWidget = settingsRoot.FindAnyWidget(setting);
			if (!settingWidget)
				continue;

			CSI_SettingInfoHandler handler = new CSI_SettingInfoHandler(setting, this);
			handler.Attach(settingWidget);
			m_aHandlers.Insert(handler);

			string label = handler.GetLabelText();
			if (!label.IsEmpty())
				m_mSettingLabels.Set(setting, label);
		}
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Shows a setting's name, description, caption and example.
	//! \param[in] setting CSI_GameSettings name
	void ShowSetting(string setting)
	{
		if (!m_wName || !m_wCaption || !m_wDescription)
			return;

		// The row's label, or the readable variable name without one
		string name;
		if (!m_mSettingLabels.Find(setting, name))
			name = SCR_StringHelper.FormatValueNameToUserFriendly(setting);

		m_wName.SetText(name);
		m_wDescription.SetText(GetAttributeDescription(CSI_GameSettings, setting));

		HideExamples();

		string caption;
		if (!m_HudExamples || !m_HudExamples.ShowExample(setting, caption))
		{
			if (!m_IconExamples || !m_IconExamples.ShowExample(setting, caption))
			{
				if (m_NametagExamples)
					m_NametagExamples.ShowExample(setting, caption);
			}
		}

		m_wCaption.SetText(caption);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows a setting with a hovered value (e.g. a color box) until EndPreview.
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value shown instead of the tab's value
	void PreviewSetting(string setting, int value)
	{
		s_sPreviewSetting = setting;
		s_iPreviewValue = value;
		ShowSetting(setting);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value previewed value; a newer preview is kept
	void EndPreview(string setting, int value)
	{
		if (s_sPreviewSetting != setting || s_iPreviewValue != value)
			return;

		s_sPreviewSetting = string.Empty;
		ShowSetting(setting);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] warn log the examples that are missing
	protected void FindExamples(bool warn = false)
	{
		Widget hudExamples = m_wRoot.FindAnyWidget("HudExamples");
		if (hudExamples)
			m_HudExamples = CSI_SettingInfoHudExamples.Cast(hudExamples.FindHandler(CSI_SettingInfoHudExamples));

		if (!m_HudExamples && warn)
			Print("CSI_SettingInfo: 'HudExamples' with CSI_SettingInfoHudExamples not found, the HUD settings won't show examples", LogLevel.WARNING);

		Widget iconExamples = m_wRoot.FindAnyWidget("IconExamples");
		if (iconExamples)
			m_IconExamples = CSI_SettingInfoIconExamples.Cast(iconExamples.FindHandler(CSI_SettingInfoIconExamples));

		if (!m_IconExamples && warn)
			Print("CSI_SettingInfo: 'IconExamples' with CSI_SettingInfoIconExamples not found, the icon settings won't show examples", LogLevel.WARNING);

		Widget nametagExamples = m_wRoot.FindAnyWidget("NametagExamples");
		if (nametagExamples)
			m_NametagExamples = CSI_SettingInfoNametagExamples.Cast(nametagExamples.FindHandler(CSI_SettingInfoNametagExamples));

		if (!m_NametagExamples && warn)
			Print("CSI_SettingInfo: 'NametagExamples' with CSI_SettingInfoNametagExamples not found, the nametag settings won't show examples", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideExamples()
	{
		if (m_HudExamples)
			m_HudExamples.HideExample();

		if (m_IconExamples)
			m_IconExamples.HideExample();

		if (m_NametagExamples)
			m_NametagExamples.HideExample();
	}

	//------------------------------------------------------------------------------------------------
	//! Placeholder shown before a setting is hovered; also hides the examples.
	protected void ShowNothing()
	{
		HideExamples();
		m_wName.SetText("Setting Info");
		m_wCaption.SetText(string.Empty);
		m_wDescription.SetText("Hover over a setting to see what it changes.");
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC HELPERS (shared by the example components)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] root CSI settings tab root
	//! \param[in] setting CSI_GameSettings name
	//! \param[out] value shown value
	//! \return false if the setting has no value widget
	static bool GetWidgetValue(Widget root, string setting, out int value)
	{
		if (!root)
			return false;

		Widget widget = root.FindAnyWidget(setting);
		if (!widget)
			return false;

		SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
		if (sliderComp)
		{
			value = sliderComp.GetValue();
			return true;
		}

		SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(widget.FindHandler(SCR_ComboBoxComponent));
		if (comboComp)
		{
			value = comboComp.GetCurrentIndex();
			return true;
		}

		SCR_SpinBoxComponent spinComp = SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));
		if (spinComp)
		{
			value = spinComp.GetCurrentIndex();
			return true;
		}

		SCR_ToolboxComponent toolboxComp = SCR_ToolboxComponent.Cast(widget.FindHandler(SCR_ToolboxComponent));
		if (toolboxComp)
		{
			value = toolboxComp.GetCurrentIndex();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] root CSI settings tab root
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] value
	//! \return false if the setting has no value widget
	static bool SetWidgetValue(Widget root, string setting, int value)
	{
		if (!root)
			return false;

		Widget widget = root.FindAnyWidget(setting);
		if (!widget)
			return false;

		SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
		if (sliderComp)
		{
			sliderComp.SetValue(value);
			return true;
		}

		SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(widget.FindHandler(SCR_ComboBoxComponent));
		if (comboComp)
		{
			comboComp.SetCurrentItem(value, false, true);
			return true;
		}

		SCR_SpinBoxComponent spinComp = SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));
		if (spinComp)
		{
			spinComp.SetCurrentItem(value, false, true);
			return true;
		}

		SCR_ToolboxComponent toolboxComp = SCR_ToolboxComponent.Cast(widget.FindHandler(SCR_ToolboxComponent));
		if (toolboxComp)
		{
			toolboxComp.SetCurrentItem(value, false, true);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] root CSI settings tab root
	//! \param[in] setting
	//! \return the tab's or previewed value, 0 without a widget
	static int GetValue(Widget root, string setting)
	{
		if (!s_sPreviewSetting.IsEmpty() && setting == s_sPreviewSetting)
			return s_iPreviewValue;

		int value;
		if (!GetWidgetValue(root, setting, value))
			return 0;

		return value;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type class declaring the member
	//! \param[in] variableName
	//! \return empty if the member has no Attribute
	static string GetAttributeDescription(typename type, string variableName)
	{
		for (int i, count = type.GetVariableCount(); i < count; i++)
		{
			if (type.GetVariableName(i) != variableName)
				continue;

			array<Class> attributes = {};
			type.GetVariableAttributes(i, attributes);
			foreach (Class attributeClass : attributes)
			{
				Attribute attribute = Attribute.Cast(attributeClass);
				if (attribute)
					return attribute.m_Desc;
			}

			break;
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Draws a role icon with the tab's icon theme, icon type and arrow theme.
	//! \param[in] root CSI settings tab root
	//! \param[in] icon CSI_Icon.layout instance
	//! \param[in] size px, 0 keeps the layout's size
	//! \param[in] role
	//! \param[in] colorTeam
	//! \param[in] iconRotation degrees
	//! \param[in] arrowRotation degrees
	//! \param[in] opacity
	static void ShowIcon(Widget root, Widget icon, float size, CSI_EIcon role, CSI_EColorTeam colorTeam, float iconRotation, float arrowRotation, float opacity)
	{
		if (!icon)
			return;

		ImageWidget iconWidget = ImageWidget.Cast(icon.FindAnyWidget("Icon"));
		ImageWidget outlineWidget = ImageWidget.Cast(icon.FindAnyWidget("Outline"));
		ImageWidget arrowWidget = ImageWidget.Cast(icon.FindAnyWidget("Arrow"));
		ImageWidget iconLarge = ImageWidget.Cast(icon.FindAnyWidget("Icon_Large"));
		ImageWidget outlineLarge = ImageWidget.Cast(icon.FindAnyWidget("Outline_Large"));
		Widget iconFrame = icon.FindAnyWidget("IconFrame");
		if (!iconWidget || !outlineWidget || !arrowWidget || !iconLarge || !outlineLarge)
			return;

		// Vehicle seat icons are always simple and drawn large without an arrow
		bool isVehicleIcon = CSI_UIHelper.m_aVehicleIcons.Contains(role);
		bool simpleIcon = isVehicleIcon || GetValue(root, CSI_GameSettings.ICON_TYPE) != CSI_EIconType.REGULAR;
		string iconName = GetIconName(root, role, simpleIcon);

		iconWidget.SetVisible(!isVehicleIcon);
		outlineWidget.SetVisible(!isVehicleIcon && simpleIcon);
		arrowWidget.SetVisible(!isVehicleIcon);
		iconLarge.SetVisible(isVehicleIcon);
		outlineLarge.SetVisible(isVehicleIcon);

		if (isVehicleIcon)
		{
			iconLarge.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, iconName);
			iconLarge.SetRotation(iconRotation);
			outlineLarge.SetRotation(iconRotation);
		}
		else
		{
			iconWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, iconName);
			iconWidget.SetRotation(iconRotation);
			outlineWidget.SetRotation(iconRotation);

			arrowWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, "ARROW " + SCR_Enum.GetEnumName(CSI_EArrowTheme, GetValue(root, CSI_GameSettings.ARROW_THEME)));
			arrowWidget.SetRotation(arrowRotation);
		}

		if (iconFrame)
			iconFrame.SetColor(CSI_ColorPresets.GetColorTeamColor(colorTeam, GetValue(root, CSI_GameSettings.COLOR_TEAM_SET), GetValue(root, CSI_GameSettings.STANDARD_SQUAD_COLOR)));

		if (size > 0)
			FrameSlot.SetSize(icon, size, size);

		icon.SetOpacity(opacity);
		icon.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Imageset name of a role icon, in the tab's icon theme.
	//! \param[in] root CSI settings tab root
	//! \param[in] role
	//! \param[in] simpleIcon
	//! \return
	static string GetIconName(Widget root, CSI_EIcon role, bool simpleIcon)
	{
		string iconName = SCR_Enum.GetEnumName(CSI_EIcon, role);
		if (!CSI_UIHelper.m_aNonThemedRegularIcons.Contains(role) && !CSI_UIHelper.m_aVehicleIcons.Contains(role))
			iconName = SCR_Enum.GetEnumName(CSI_EIconTheme, GetValue(root, CSI_GameSettings.ICON_THEME)) + " " + iconName;

		if (simpleIcon)
			iconName = iconName + "_ICON";

		return iconName;
	}

	//------------------------------------------------------------------------------------------------
	//! Local player's group in HUD order, or just the local player outside a group.
	//! \param[out] memberIDs
	//! \param[in] maxCount
	static void GetPreviewGroupMembers(notnull array<int> memberIDs, int maxCount)
	{
		memberIDs.Clear();

		CSI_HUDSystem hudSystem = CSI_HUDSystem.GetInstance();
		if (hudSystem)
		{
			foreach (PlayerId playerID : hudSystem.GetLocalGroupPlayerIds())
			{
				if (memberIDs.Count() >= maxCount)
					break;

				memberIDs.Insert(playerID);
			}
		}

		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		if (memberIDs.IsEmpty() && localPlayerID > 0)
			memberIDs.Insert(localPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Cached: without a profile name, SCR_Global.GetProfileName logs on every call.
	//! \return
	static string GetProfileName()
	{
		if (s_sProfileName.IsEmpty())
			s_sProfileName = SCR_Global.GetProfileName();

		return s_sProfileName;
	}

	//------------------------------------------------------------------------------------------------
	//! Player name, with a rank if the tab's rank setting is on.
	//! \param[in] root CSI settings tab root
	//! \param[in] playerID 0 outside a session or for placeholders
	//! \param[in] fallbackName used without a player or name
	//! \return
	static string GetPreviewName(Widget root, PlayerId playerID, string fallbackName)
	{
		string name;
		if (playerID > 0)
		{
			SCR_PlayerNamesFilterCache namesCache = SCR_PlayerNamesFilterCache.GetInstance();
			if (namesCache)
				name = namesCache.GetPlayerDisplayName(playerID);

			if (name.IsEmpty() && GetGame().GetPlayerManager())
				name = GetGame().GetPlayerManager().GetPlayerName(playerID);
		}

		if (name.IsEmpty())
			name = fallbackName;

		if (!GetValue(root, CSI_GameSettings.RANK_VISIBLE))
			return name;

		return GetRankShortName(playerID) + " " + name;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return "Pvt." if unknown
	static string GetRankShortName(PlayerId playerID)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (!playerData)
			return "Pvt.";

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (!faction || !faction.GetRanks())
			return "Pvt.";

		string rank = faction.GetRanks().GetRankNameShort(playerData.GetRank());
		if (rank.IsEmpty())
			return "Pvt.";

		return rank;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return MAN without CSI data
	static CSI_EIcon GetPlayerIcon(PlayerId playerID)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (!playerData)
			return CSI_EIcon.MAN;

		return playerData.GetDisplayIcon();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] fallback used without CSI data
	//! \return
	static CSI_EColorTeam GetPlayerColorTeam(PlayerId playerID, CSI_EColorTeam fallback)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (!playerData)
			return fallback;

		return playerData.GetColorTeam();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return null outside a session or without CSI data
	static CSI_PlayerData GetPlayerData(PlayerId playerID)
	{
		if (playerID <= 0)
			return null;

		CSI_PlayerDataManager playerDataManager = CSI_PlayerDataManager.GetInstance();
		if (!playerDataManager)
			return null;

		return playerDataManager.GetPlayerData(playerID);
	}
}
