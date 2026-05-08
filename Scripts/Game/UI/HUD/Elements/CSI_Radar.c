class CSI_Radar : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	
	protected int m_iStoredGroupCount = -1;
	protected ref array<Widget> m_aRadarIcons;
	protected ref array<CSI_Icon> m_aRadarIconHandlers;

	protected static int ICON_WIDTH_AND_HEIGHT = 16;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDSystem = CSI_HUDSystem.GetInstance();
		
		m_aRadarIcons = CSI_UIHelper.GetAllIcons(m_wRoot, "RadarIcon", 25);

		m_aRadarIconHandlers = {};
		foreach (Widget radarIcon : m_aRadarIcons)
		{
			if (radarIcon)
				m_aRadarIconHandlers.Insert(CSI_Icon.Cast(radarIcon.FindHandler(CSI_Icon)));
			else
				m_aRadarIconHandlers.Insert(null);
		};
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	//! Updates the radar display each frame.
	void Update()
	{			
		SCR_ChimeraCharacter localCharacter = m_HUDSystem.GetLocalPlayerCharacter();
		if (!localCharacter)
			return;
		
		int groupCount = m_HUDSystem.GetLocalGroupCount();
        float localYaw = m_HUDSystem.GetLocalYaw();
		vector localOrigin = localCharacter.GetOrigin();

		if (groupCount > 1 && m_SettingsManager.GetSettingBool(CSI_GameSettings.RADAR_VISIBLE))
		{
			// Hoist loop-invariant values
			int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
			float searchRadius;
			if (m_HUDSystem.GetIsLocalPlayerInVehicle())
				searchRadius = 3.5;
			else
				searchRadius = 24;
			float searchRadiusFadeStart = 0.8 * searchRadius;
			float radarScale = 132.0 / (searchRadius * 2.725);

			foreach (int i, int playerID : m_HUDSystem.GetLocalGroupPlayerIds())
			{
				float x, y, opacity, rotation, disT, dis;
				SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));

				if (!playerCharacter)
				{
					UpdatePlayerRadarWidget(i, -1, 1, 0, 0, 0, 0);
					continue;
				};
				
				vector playerCharacterOrigin = playerCharacter.GetOrigin();

				// Get Distance
				dis = vector.Distance(localOrigin, playerCharacterOrigin);
				
				if (dis > searchRadius)
				{
					UpdatePlayerRadarWidget(i, -1, 1, 0, 0, 0, 0);
					continue;
				};

				if (playerID != localPlayerId)
				{
					// Get Relative Direction (algebraic simplification of the original two-step Mod)
					float dir = vector.Direction(playerCharacterOrigin, localOrigin).ToYaw();
					float relDir = Math.Mod(360 - dir - localYaw, 360) * Math.DEG2RAD;
					
					disT = dis * radarScale;
					x = Math.Sin(relDir) * disT;
					y = Math.Cos(relDir) * disT;
				};

				opacity = Math.Map(dis, searchRadiusFadeStart, searchRadius, 0.6, 0);
				rotation = -Math.Mod((CSI_CharacterHelper.GetCharacterYaw(playerCharacter) - localYaw), 360);

				UpdatePlayerRadarWidget(i, playerID, ICON_WIDTH_AND_HEIGHT, opacity, x, y, rotation);
			};
		} else
			groupCount = 0;

		for (int e = groupCount; e <= 24; e++)
			UpdatePlayerRadarWidget(e, -1, 1, 0, 0, 0, 0);
	}
	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 ELEMENT SPECIFIC METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Updates radar widget properties for a specific player on the HUD.
	//! \param[in] widgetNumber The index of the radar widget to update
	//! \param[in] playerID ID of the player
	//! \param[in] widthAndHeight Width and height dimensions for the widget
	//! \param[in] opacity Transparency value for the widget
	//! \param[in] x X coordinate position
	//! \param[in] y Y coordinate position
	//! \param[in] rotation Rotation angle in degrees
	protected void UpdatePlayerRadarWidget(int widgetNumber, int playerID, float widthAndHeight, float opacity, float x, float y, float rotation)
	{
		Widget radarIcon = m_aRadarIcons[widgetNumber];

		if (radarIcon) 
		{
			float iconOpacity = radarIcon.GetOpacity();
			if (iconOpacity == 0 && iconOpacity == opacity)
				return;
			
			CSI_Icon icon = m_aRadarIconHandlers[widgetNumber];
			
			icon.IconUpdate(playerID);
			
			widthAndHeight = widthAndHeight * (m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01);

			FrameSlot.SetPos(radarIcon, (x - widthAndHeight/2), (y - widthAndHeight/2));
			FrameSlot.SetSize(radarIcon, widthAndHeight, widthAndHeight);

			icon.SetRotation(rotation);
			radarIcon.SetOpacity(opacity);
		};
	}
}