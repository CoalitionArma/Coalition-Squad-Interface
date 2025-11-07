class CSI_Radar : SCR_ScriptedWidgetComponent
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_HUDManager m_HUDManager;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	
	protected int m_iStoredGroupCount = -1;
	protected ref array<Widget> m_aRadarIcons;

	protected static int ICON_WIDTH_AND_HEIGHT = 16;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
		m_HUDManager = CSI_HUDManager.GetInstance();
		
		m_aRadarIcons = CSI_UIHelper.GetAllIcons(m_wRoot, "RadarIcon", 24);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates the radar display each frame.
	 */
	void Update()
	{			
		SCR_ChimeraCharacter localCharacter = m_HUDManager.GetLocalPlayerCharacter();
		if (!localCharacter)
			return;
		
		int groupCount = m_HUDManager.GetLocalGroupCount();
        float localYaw = m_HUDManager.GetLocalYaw();
		vector localOrigin = localCharacter.GetOrigin();

		if (groupCount > 1 && m_SettingsManager.GetSettingBool(CSI_GameSettings.RADAR_VISIBLE))
		{
			foreach (int i, int playerID : m_HUDManager.GetLocalGroupPlayerIds())
			{
				float x, y, opacity, rotation, disT, dis, searchRadius;
				SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));

				if (!playerCharacter)
					continue;
				
				vector playerCharacterOrigin = playerCharacter.GetOrigin();

				// Get Distance
				dis = vector.Distance(localOrigin, playerCharacterOrigin);
				
				if (m_HUDManager.GetIsLocalPlayerInVehicle())
				{
					searchRadius = 8;
					disT = dis * 6.215;
				} else {
					searchRadius = 24;
					disT = dis * 2.0;
				};
				
				if (dis > searchRadius) 
					continue;

				if (playerID != SCR_PlayerController.GetLocalPlayerId())
				{
					// Get Direction
					float dir = vector.Direction(playerCharacterOrigin, localOrigin).ToYaw();
		
					// Get Relative Direction
					float relDir = Math.Mod(((dir - localYaw) + 360), 360);
					relDir = Math.Mod(relDir - (dir * 2), 360);
					relDir = relDir * Math.DEG2RAD;
		
					x = (Math.Sin(relDir) * disT);
					y = (Math.Cos(relDir) * disT);
				};

				opacity = Math.Map(dis, (0.8*searchRadius), searchRadius, 0.6, 0);
				rotation = -Math.Mod((CSI_ChararcterHelper.GetCharacterYaw(playerCharacter) - localYaw), 360);

				UpdatePlayerRadarWidget(i, playerID, ICON_WIDTH_AND_HEIGHT, opacity, x, y, rotation);
			};
		} else
			groupCount = 0;

		if(m_iStoredGroupCount == groupCount)
			return;
		
		m_iStoredGroupCount = groupCount;
		for (int e = groupCount; e <= 24; e++)
			UpdatePlayerRadarWidget(e, -1, 1, 0, 0, 0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates radar widget properties for a specific player on the HUD.
	 * @param widgetNumber The index of the radar widget to update
	 * @param playerID ID of the player
	 * @param widthAndHeight Width and height dimensions for the widget
	 * @param opacity Transparency value for the widget
	 * @param x X coordinate position
	 * @param y Y coordinate position
	 * @param rotation Rotation angle in degrees
	 */
	protected void UpdatePlayerRadarWidget(int widgetNumber, int playerID, float widthAndHeight, float opacity, float x, float y, float rotation)
	{
		Widget radarIcon = m_aRadarIcons[widgetNumber];

		if (radarIcon) 
		{
			float iconOpacity = radarIcon.GetOpacity();
			if (iconOpacity == 0 && iconOpacity == opacity)
				return;
			
			CSI_Icon icon = CSI_Icon.Cast(radarIcon.FindHandler(CSI_Icon));
			
			icon.IconUpdate(playerID);
			
			widthAndHeight = widthAndHeight * (m_SettingsManager.GetSettingInt(CSI_GameSettings.RADAR_ICON_SIZE) * 0.01);

			FrameSlot.SetPos(radarIcon, (x - widthAndHeight/2), (y - widthAndHeight/2));
			FrameSlot.SetSize(radarIcon, widthAndHeight, widthAndHeight);

			icon.SetRotation(rotation);
			radarIcon.SetOpacity(opacity);
		};
	}
}