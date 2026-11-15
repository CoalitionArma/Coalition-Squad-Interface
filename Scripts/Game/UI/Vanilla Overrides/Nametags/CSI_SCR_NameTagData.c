//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: names with rank; adds player data, group name and field of view place for CSI's elements;
//! 				tag position and height from settings; opacity follows scene brightness.
//! 
//------------------------------------------------------------------------------------------------
modded class SCR_NameTagData : Managed
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	CSI_PlayerData m_PlayerData;	//!< null for AI and players without CSI data
	string m_sGroupName;			//!< set by GetGroupName
	float m_fFieldOfViewFraction;	//!< place in the tag's visible angle: 0 centre, 1 edge

	//------------------------------------------------------------------------------------------------
	override void UpdateEntityType()
	{
		super.UpdateEntityType();
		UpdatePlayerData();
	}

	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		// CSI: cache managers
		if (!m_SettingsManager || !m_PlayerDataManager)
		{
			m_SettingsManager = CSI_SettingsManager.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		};

		m_eEntityStateFlags = ENameTagEntityState.HIDDEN | ENameTagEntityState.DEFAULT;
	 	m_ePriorityEntityState = ENameTagEntityState.HIDDEN;

		UpdateAttatchedTo();	// CSI: position from NAMETAG_POSITION

		m_iZoneID = -1;
		m_iGroupID = -1;
		m_iPlayerID = -1;
		m_PlayerData = null;	// CSI
		m_fTimeSliceUpdate = 1.0;
		m_fTimeSliceVON = 0;
		m_fTimeSlicePosChange = 0;
		m_fTimeSliceCleanup = 0;
		m_fTimeSliceFade = 0;
		m_fTimeSliceVisibility = 0;
		m_fDistance = 0;
		m_fOpacityFade = 1;
		m_sName = string.Empty;
		m_sGroupName = string.Empty;	// CSI
		m_aNameParams = {};
	};

	//------------------------------------------------------------------------------------------------
	override void GetName(out string name, out notnull array<string> nameParams)
	{
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			// CSI: name with rank, refresh CSI data
			m_sName = CSI_UIHelper.GetPlayersName(m_iPlayerID);
			UpdatePlayerData();
		} else if (m_eType == ENameTagEntityType.AI)
		{
			SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(m_Entity.FindComponent(SCR_CharacterIdentityComponent));
			if (scrCharIdentity)
			{
				scrCharIdentity.GetFormattedFullName(m_sName, m_aNameParams);
			} else {
				CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(m_Entity.FindComponent(CharacterIdentityComponent));
				if (charIdentity && charIdentity.GetIdentity())
					m_sName = charIdentity.GetIdentity().GetName();
				else
					m_sName = "No character identity!";
			};
		};

		name = m_sName;
		nameParams.Copy(m_aNameParams);
	}

	//------------------------------------------------------------------------------------------------
	//! Set visibility of nametag widget
	//! \param[in] widget
	//! \param[in] visible
	//! \param[in] visibleOpacity
	//! \param[in] animate unused, UpdateOpacity handles fading
	override void SetVisibility(Widget widget, bool visible, float visibleOpacity, bool animate = true)
	{
		if (!widget)
			return;

		float targetVal;
		if (visible)
		{
			m_NameTagWidget.SetVisible(true);
			if (widget == m_NameTagWidget)
				m_fVisibleOpacity = visibleOpacity;

			m_Flags |= ENameTagFlags.VISIBLE;
			m_Flags &= ~ENameTagFlags.UPDATE_DISABLE;
			m_Flags &= ~ENameTagFlags.DISABLED;
			targetVal = visibleOpacity;
		}
		else
			targetVal = 0;

		// CSI: opacity set by UpdateOpacity
		if (targetVal >= 0.1)
			widget.SetVisible(true);

		UpdateOpacity(widget, targetVal);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets an element's opacity from scene brightness, or to targetOpacity with dimming off.
	//! \param[in] widget null to use data.m_aNametagElements[index]
	//! \param[in] targetOpacity 0 hides it
	//! \param[in] data
	//! \param[in] index
	void UpdateOpacity(Widget widget, float targetOpacity = 0.85, SCR_NameTagData data = null, int index = 0)
	{
		if (!widget)
		{
			widget = data.m_aNametagElements[index];
			if (!widget)
				return;
		};

		if (m_Entity && targetOpacity != 0)
		{
			BaseWorld world = GetGame().GetWorld();
			CameraManager cameraManager = GetGame().GetCameraManager();

			bool dimWithBrightness = !m_SettingsManager || m_SettingsManager.GetSettingBool(CSI_GameSettings.NAMETAG_BRIGHTNESS_DIMMING);

			if (world && cameraManager && dimWithBrightness)
			{
				CameraBase camera = cameraManager.CurrentCamera();
				if (camera)
				{
					int cameraIndex = camera.GetCameraIndex();

					float brightness = Math.Clamp(world.GetCameraSceneMiddleBrightness(cameraIndex), 0.1, 10);

					float hdrBrightnessFactor = world.GetCameraHDRBrightness(cameraIndex);
					if (hdrBrightnessFactor > 1) // Only let HDR raise brightness (NVGs)
						brightness = brightness * hdrBrightnessFactor;

					targetOpacity = Math.Clamp(brightness, 0.1, 1);
				 }
			}
		} else
			targetOpacity = 0;

		if (m_fTagFadeSpeed != 0)
			AnimateWidget.Opacity(widget, targetOpacity, m_fTagFadeSpeed);
		else
		{
			AnimateWidget.StopAnimation(widget, WidgetAnimationOpacity);
			widget.SetOpacity(targetOpacity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets m_sGroupName to the group's custom name or callsign; empty if none or the setting is off.
	void GetGroupName()
	{
		// TODO: Better AI handling
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group || !m_SettingsManager || !m_SettingsManager.GetSettingBool(CSI_GameSettings.GROUP_IN_NAMETAG_VISIBLE))
		{
			m_sGroupName = "";
			return;
		}

		m_sGroupName = group.GetCustomName();

		if (m_sGroupName.IsEmpty())
		{
			string company, platoon, squad, character, format;
			group.GetCallsigns(company, platoon, squad, character, format);
			company = WidgetManager.Translate(company);
			m_sGroupName = string.Format(format, company, platoon, squad, character);
		};
	}

	//------------------------------------------------------------------------------------------------
	//! Update tag position
	override void UpdateTagPos()
	{
		UpdateAttatchedTo();	// CSI: pick up NAMETAG_POSITION changes

		vector matPos[4];
		Animation anim = m_Entity.GetAnimation();
		anim.GetBoneMatrix(m_iSpineBone, matPos);
		m_vEntWorldPos = m_Entity.CoordToParent(matPos[3]);
		anim.GetBoneMatrix(m_iHeadBone, matPos);
		m_vEntHeadPos = m_Entity.CoordToParent(matPos[3]);

		// CSI: height offset from NAMETAG_POSITION_OFFSET (0.1 m steps)
		vector nametagOffsetVector = "0 0 0";
		int nametagOffset = 0;

		if (m_SettingsManager)
			nametagOffset = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_POSITION_OFFSET);

		if (m_eAttachedTo == ENameTagPosition.HEAD)
		{
			nametagOffsetVector[1] = ((nametagOffset + 1) * 0.1);
			m_vTagWorldPos = m_vEntHeadPos + nametagOffsetVector;
		}
		else if (m_eAttachedTo == ENameTagPosition.BODY)
		{
			nametagOffsetVector[1] = ((nametagOffset - 5) * 0.1);
			m_vTagWorldPos = m_vEntWorldPos + nametagOffsetVector;
		}

		if (m_eType != ENameTagEntityType.PLAYER && GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Entity) > 0)
		{
			m_Flags |= ENameTagFlags.ENT_TYPE_UPDATE;
			m_Flags |= ENameTagFlags.NAME_UPDATE;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerData()
	{
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();

		if (m_PlayerDataManager && m_iPlayerID > 0)
			m_PlayerData = m_PlayerDataManager.GetPlayerData(m_iPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets m_eAttachedTo from NAMETAG_POSITION (BODY without settings).
	void UpdateAttatchedTo()
	{
		ENameTagPosition nametagPos = ENameTagPosition.BODY;

		if (m_SettingsManager)
			nametagPos = m_SettingsManager.GetSettingInt(CSI_GameSettings.NAMETAG_POSITION);

		m_eAttachedTo = nametagPos;
		m_eAttachedToLast = nametagPos;
	}
}
