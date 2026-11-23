//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-22
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: shows the player's CSI role or state icon in the RoleIcon widget picked by
//! 				NAMETAG_ROLE_ICON_POSITION, sized by NAMTEAG_SCALE and colored like the name; talking shows
//! 				animated bars (CSI_TalkingBarsComponent) in its place.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTIconBase
{
	static const int ICON_KEY_NONE = -3;	//!< no icon loaded; GetIconKey never returns it

	//------------------------------------------------------------------------------------------------
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;

		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;

		//-----------------------------------------------------------------------
		// Hide Unused Role Icon Widgets
		// AI always show their icon (their tags are icon only); players' follow ROLE_IN_NAMETAG_VISIBLE
		bool showIcon = data.m_eType == ENameTagEntityType.AI || CSI_NametagSettings.s_bRoleIcons;
		if (m_sWidgetName != CSI_NametagSettings.s_sRoleIconWidgetName || !showIcon)
		{
			data.SetVisibility(iWidget, false, 0, false);
			iWidget.SetVisible(false);	// UpdateElement skips hidden icons
			SetTalkingBars(iWidget, false);

			// Loaded again when shown: the bars stopped, and the icon may be the empty one they cover
			data.m_aElementIconKeys[index] = ICON_KEY_NONE;
			return;
		};

		//-----------------------------------------------------------------------
		// Set Static Scale Of The Icon
		iWidget.SetSize(CSI_NametagSettings.s_iIconSize, CSI_NametagSettings.s_iIconSize);

		//-----------------------------------------------------------------------
		// Actually Make The Widget Visible
		data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);

		// Only the RoleIcon widget in use shows; SetDefaults hid the others
		if (m_sWidgetName != CSI_NametagSettings.s_sRoleIconWidgetName)
			return;

		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget || !iWidget.IsVisible())
			return;

		// The image is only loaded when what the icon shows changes
		int iconKey = GetIconKey(data);
		bool loadIcon = iconKey != data.m_aElementIconKeys[index];
		data.m_aElementIconKeys[index] = iconKey;

		if (data.m_PlayerData)
		{
			//-----------------------------------------------------------------------
			// Color Updates For VON/Color Teams
			SetElementColor(data, index, data.m_TagColor);
			
			//-----------------------------------------------------------------------
			// State icons first, then the CSI display icon (MAN uses the pointer)
			if (loadIcon)
			{
				
				// Talking: CSI_TalkingBars over an empty icon
				bool talking = data.m_ePriorityEntityState == ENameTagEntityState.VON;
				SetTalkingBars(iWidget, talking);

				switch (true)
				{
					case (data.m_ePriorityEntityState == ENameTagEntityState.UNCONSCIOUS)	: iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "unconscious"); break;
					case (data.m_ePriorityEntityState == ENameTagEntityState.DEAD) 		: iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "death"); break;
					case (talking) : break;
					default : {
						bool setAlt = false;
						CSI_EIcon displayIcon = data.m_PlayerData.GetDisplayIcon();

						if (displayIcon == CSI_EIcon.DRIVER)
							setAlt = true;

						if (displayIcon == CSI_EIcon.MAN)
							iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "pointer-small");
						else
							iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, CSI_UIHelper.GetIconString(displayIcon, true, setAlt));
					}
				};
			}
		} else {
			// No CSI data: AI icon for AI, else empty
			SetElementColor(data, index, CSI_NametagSettings.s_NametagColor);

			if (loadIcon)
			{
				SetTalkingBars(iWidget, false);

				if (data.m_eType == ENameTagEntityType.AI)
					iWidget.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "ai");
				else
					iWidget.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, "EMPTY");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Shows or hides the talking bars (CSI_TalkingBarsComponent) beside the icon, which shows nothing while they
	//! play. Without its Overlay the icon shows vanilla's talking icon instead.
	//! \param[in] icon
	//! \param[in] talking
	protected void SetTalkingBars(notnull ImageWidget icon, bool talking)
	{
		CSI_TalkingBarsComponent talkingBars;
		if (talking)
			talkingBars = CSI_TalkingBarsComponent.GetOrCreate(icon);
		else
			talkingBars = CSI_TalkingBarsComponent.Find(icon);

		if (talkingBars)
			talkingBars.SetActive(talking);

		if (!talking)
			return;

		if (talkingBars)
			icon.LoadImageFromSet(0, CSI_UIHelper.CSI_ICONS, "EMPTY");
		else
			icon.LoadImageFromSet(0, CSI_UIHelper.VANILLA_NAMETAG_ICONS, "VON");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	//! \return what the icon shows: state, icon theme and display icon with CSI data, else -2 for AI and -1
	protected int GetIconKey(SCR_NameTagData data)
	{
		if (!data.m_PlayerData)
		{
			if (data.m_eType == ENameTagEntityType.AI)
				return -2;

			return -1;
		}

		return (data.m_ePriorityEntityState << 16) | (CSI_NametagSettings.s_iIconTheme << 8) | data.m_PlayerData.GetDisplayIcon();
	}
}