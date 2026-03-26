class CSI_Radar : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDSystem m_HUDSystem;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	
	protected int m_iStoredGroupCount = -1;
	protected ref array<Widget> m_aRadarIcons = {};
	protected ref array<CSI_Icon> m_aRadarIconsClasses = {};

	protected static int ICON_WIDTH_AND_HEIGHT = 16;
	
	protected bool m_bRadarVisible;
	protected int m_iRadarIconSize;

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
		
		CSI_SettingsManager.GetInstance().GetOnSettingsUpdate().Insert(OnSettingsUpdate);
		OnSettingsUpdate();
		
		m_aRadarIcons = CSI_UIHelper.GetAllIcons(m_wRoot, "RadarIcon", 24);
		
		foreach (Widget radarIcon : m_aRadarIcons)
			m_aRadarIconsClasses.Insert(CSI_Icon.Cast(radarIcon.FindHandler(CSI_Icon)));
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

		if (groupCount > 1 && m_bRadarVisible)
		{
			foreach (int i, int playerID : m_HUDSystem.GetLocalGroupPlayerIds())
			{
				float x, y, opacity, rotation, disT, dis, searchRadius;
				SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));

				if (!playerCharacter)
				{
					UpdatePlayerRadarWidget(i, -1, 1, 0, 0, 0, 0);
					continue;
				};
				
				vector playerCharacterOrigin = playerCharacter.GetOrigin();

				// Get Distance
				dis = vector.Distance(localOrigin, playerCharacterOrigin);
				
				if (m_HUDSystem.GetIsLocalPlayerInVehicle())
					searchRadius = 3.5;
				else
					searchRadius = 24;
				
				if (dis > searchRadius)
				{
					UpdatePlayerRadarWidget(i, -1, 1, 0, 0, 0, 0);
					continue;
				};

				if (playerID != SCR_PlayerController.GetLocalPlayerId())
				{
					// Get Direction
					float dir = vector.Direction(playerCharacterOrigin, localOrigin).ToYaw();
		
					// Get Relative Direction
					float relDir = Math.Mod(((dir - localYaw) + 360), 360);
					relDir = Math.Mod(relDir - (dir * 2), 360);
					relDir = relDir * Math.DEG2RAD;
					
					disT = (dis * (132 / (searchRadius * 2.725)));
					x = (Math.Sin(relDir) * disT);
					y = (Math.Cos(relDir) * disT);
				};

				opacity = Math.Map(dis, (0.8 * searchRadius), searchRadius, 0.6, 0);
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
		CSI_Icon iconClass = m_aRadarIconsClasses[widgetNumber];
		
		if (radarIcon && iconClass)
		{
			float iconOpacity = radarIcon.GetOpacity();
			if (iconOpacity == 0 && iconOpacity == opacity)
				return;
			
			iconClass.IconUpdate(playerID);
			
			widthAndHeight = widthAndHeight * (m_iRadarIconSize * 0.01);

			FrameSlot.SetPos(radarIcon, (x - widthAndHeight/2), (y - widthAndHeight/2));
			FrameSlot.SetSize(radarIcon, widthAndHeight, widthAndHeight);

			iconClass.SetRotation(rotation);
			radarIcon.SetOpacity(opacity);
		};
	}
	
//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTINGS UPDATE
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	protected void OnSettingsUpdate()
	{
		CSI_SettingsManager settingsManager = CSI_SettingsManager.GetInstance();
		
		m_bRadarVisible = settingsManager.GetSettingBool(CSI_GameSettings.RADAR_VISIBLE);
		m_iRadarIconSize = settingsManager.GetSettingBool(CSI_GameSettings.RADAR_ICON_SIZE);
	}
}