//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Added to each setting row in the CSI settings tab: tells CSI_SettingInfo which setting the
//! 				player is hovering, focusing or changing.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfoHandler : ScriptedWidgetEventHandler
{
	protected string m_sSetting;
	protected CSI_SettingInfo m_SettingInfo;	//!< not ref: CSI_SettingInfo owns this handler
	protected Widget m_wSetting;
	protected SCR_ChangeableComponentBase m_Changeable;

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_SettingInfo)
			m_SettingInfo.ShowSetting(m_sSetting);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_SettingInfo)
			m_SettingInfo.ShowSetting(m_sSetting);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Subscribed to the row's SCR_ChangeableComponentBase.m_OnChanged, so the example follows the new value.
	void OnValueChanged()
	{
		if (m_SettingInfo)
			m_SettingInfo.ShowSetting(m_sSetting);
	}

	//------------------------------------------------------------------------------------------------
	//! Hooks the setting's row: hover and focus through this handler, value changes through its m_OnChanged.
	//! \param[in] settingWidget
	void Attach(notnull Widget settingWidget)
	{
		m_wSetting = settingWidget;
		m_wSetting.AddHandler(this);

		m_Changeable = SCR_ChangeableComponentBase.Cast(settingWidget.FindHandler(SCR_ChangeableComponentBase));
		if (m_Changeable)
			m_Changeable.m_OnChanged.Insert(OnValueChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Undoes Attach.
	void Detach()
	{
		if (m_Changeable)
			m_Changeable.m_OnChanged.Remove(OnValueChanged);

		if (m_wSetting)
			m_wSetting.RemoveHandler(this);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] setting CSI_GameSettings name
	//! \param[in] settingInfo
	void CSI_SettingInfoHandler(string setting, CSI_SettingInfo settingInfo)
	{
		m_sSetting = setting;
		m_SettingInfo = settingInfo;
	}
}

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	"Setting Info" panel (CSI_SettingInfo.layout) at the top right of the CSI settings tab, for everyone:
//! 				the hovered setting's name (from its variable name, vanilla SCR_StringHelper), its CSI_GameSettings
//! 				attribute desc, and an example. The examples are the layouts in SettingInfoExamples/, each with its
//! 				own component (CSI_SettingInfoHudExamples, CSI_SettingInfoIconExamples, CSI_SettingInfoNametagExamples),
//! 				found and driven from here like CSI_HUD does its elements. Also holds the static helpers they share
//! 				(the tab's values, the local player's name, icon and group, drawing a role icon).
//! 				CSI_SettingsSubMenu finds it and hands it the tab root.
//!
//------------------------------------------------------------------------------------------------
class CSI_SettingInfo : SCR_ScriptedWidgetComponent
{
	protected TextWidget m_wName;
	protected TextWidget m_wCaption;
	protected TextWidget m_wDescription;

	protected ref array<ref CSI_SettingInfoHandler> m_aHandlers = {};

	// Example components, owned by their widgets
	protected CSI_SettingInfoHudExamples m_HudExamples;
	protected CSI_SettingInfoIconExamples m_IconExamples;
	protected CSI_SettingInfoNametagExamples m_NametagExamples;

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
		// Workbench's layout editor runs this too: keep the examples visible there so they can be edited
		if (!m_wName || !m_wCaption || !m_wDescription || SCR_Global.IsEditMode())
			return;

		FindExamples();
		ShowNothing();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		foreach (CSI_SettingInfoHandler handler : m_aHandlers)
			handler.Detach();

		m_aHandlers.Clear();

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	//! Hooks every setting row of the tab, so hovering, focusing or changing one shows it here.
	//! \param[in] settingsRoot CSI settings tab root
	void SetSettingsRoot(Widget settingsRoot)
	{
		if (!settingsRoot || !m_wName)
			return;

		// Again here, once the whole tab exists, in case the example components attached after this one
		FindExamples();
		ShowNothing();

		if (m_HudExamples)
			m_HudExamples.SetSettingsRoot(settingsRoot);

		if (m_IconExamples)
			m_IconExamples.SetSettingsRoot(settingsRoot);

		if (m_NametagExamples)
			m_NametagExamples.SetSettingsRoot(settingsRoot);

		foreach (string setting : CSI_GameSettings.GetSettingsArray())
		{
			// The settings column comes before the override column, so this finds the setting's own row
			Widget settingWidget = settingsRoot.FindAnyWidget(setting);
			if (!settingWidget)
				continue;

			CSI_SettingInfoHandler handler = new CSI_SettingInfoHandler(setting, this);
			handler.Attach(settingWidget);
			m_aHandlers.Insert(handler);
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
		if (!m_wName)
			return;

		m_wName.SetText(SCR_StringHelper.FormatValueNameToUserFriendly(setting));
		m_wDescription.SetText(GetAttributeDescription(CSI_GameSettings, setting));

		HideExamples();

		// Each example layout shows the settings it has an example for
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
	//! Finds the example components on the example layouts (HudExamples, IconExamples, NametagExamples).
	protected void FindExamples()
	{
		Widget hudExamples = m_wRoot.FindAnyWidget("HudExamples");
		if (hudExamples)
			m_HudExamples = CSI_SettingInfoHudExamples.Cast(hudExamples.FindHandler(CSI_SettingInfoHudExamples));

		Widget iconExamples = m_wRoot.FindAnyWidget("IconExamples");
		if (iconExamples)
			m_IconExamples = CSI_SettingInfoIconExamples.Cast(iconExamples.FindHandler(CSI_SettingInfoIconExamples));

		Widget nametagExamples = m_wRoot.FindAnyWidget("NametagExamples");
		if (nametagExamples)
			m_NametagExamples = CSI_SettingInfoNametagExamples.Cast(nametagExamples.FindHandler(CSI_SettingInfoNametagExamples));
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
	//! Text shown before the player has hovered a setting. The example layouts are visible in their layout files
	//! (so they can be edited), so this hides them.
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
	//! \param[in] setting CSI_GameSettings name, also the name of its widget in the tab
	//! \param[out] value the value shown in the tab
	//! \return false if the setting has no slider, combo box or spin box
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

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows a value in the setting's widget (the counterpart of GetWidgetValue), e.g. a server-forced value.
	//! \param[in] root CSI settings tab root
	//! \param[in] setting CSI_GameSettings name, also the name of its widget in the tab
	//! \param[in] value
	//! \return false if the setting has no slider, combo box or spin box
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

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] root CSI settings tab root
	//! \param[in] setting
	//! \return the value selected in the tab, 0 if the setting has no widget
	static int GetValue(Widget root, string setting)
	{
		int value;
		if (!GetWidgetValue(root, setting, value))
			return 0;

		return value;
	}

	//------------------------------------------------------------------------------------------------
	//! Reads a member's [Attribute] desc, so settings are described where they're declared (see CSI_GameSettings).
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
	//! Draws a role icon like CSI_Icon.SetIconWidget, using the tab's icon theme, icon type and arrow theme.
	//! \param[in] root CSI settings tab root
	//! \param[in] icon CSI_Icon.layout instance
	//! \param[in] size px; 0 keeps the size the layout gives it
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
			iconFrame.SetColor(CSI_UIHelper.ConvertColorTeamToColor(colorTeam));

		if (size > 0)
			FrameSlot.SetSize(icon, size, size);

		icon.SetOpacity(opacity);
		icon.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Same imageset names as CSI_UIHelper.GetIconString, but with the tab's icon theme rather than the saved one.
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
	//! Members of the local player's group, in HUD order. Outside a group: just the local player, if any.
	//! \param[out] memberIDs
	//! \param[in] maxCount
	static void GetPreviewGroupMembers(notnull array<int> memberIDs, int maxCount)
	{
		memberIDs.Clear();

		CSI_HUDSystem hudSystem = CSI_HUDSystem.GetInstance();
		if (hudSystem)
		{
			foreach (int playerID : hudSystem.GetLocalGroupPlayerIds())
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
	//! Player name like CSI_UIHelper.GetPlayersName, but with the tab's rank setting.
	//! \param[in] root CSI settings tab root
	//! \param[in] playerID 0 outside a session or for placeholders
	//! \param[in] fallbackName used without a player or name
	//! \return
	static string GetPreviewName(Widget root, int playerID, string fallbackName)
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
	//! \return the player's short rank, or "Pvt." when it isn't known (main menu, placeholders)
	static string GetRankShortName(int playerID)
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
	//! \return the player's current display icon, MAN without CSI data
	static CSI_EIcon GetPlayerIcon(int playerID)
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
	static CSI_EColorTeam GetPlayerColorTeam(int playerID, CSI_EColorTeam fallback)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		if (!playerData)
			return fallback;

		return playerData.GetColorTeam();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return null outside a session or without CSI data
	static CSI_PlayerData GetPlayerData(int playerID)
	{
		if (playerID <= 0)
			return null;

		CSI_PlayerDataManager playerDataManager = CSI_PlayerDataManager.GetInstance();
		if (!playerDataManager)
			return null;

		return playerDataManager.GetPlayerData(playerID);
	}
}
