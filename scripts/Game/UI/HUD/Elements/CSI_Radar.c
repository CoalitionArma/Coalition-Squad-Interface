class CSI_Radar : SCR_ScriptedWidgetComponent
{
	protected CSI_SettingsManager m_SettingsManager;
	protected CSI_PlayerControllerManager m_ClientComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	protected static int ICON_WIDTH_AND_HEIGHT = 16;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_ClientComponent = CSI_PlayerControllerManager.GetInstance();
		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		m_SettingsManager = CSI_SettingsManager.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{		
		SCR_ChimeraCharacter localPlayerCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());
		
		if (!localPlayerCharacter) 
			return;

		// Freelook Direction
        float yaw = CSI_ChararcterHelper.GetLocalAimingYaw();
		vector localPlayerCharacterOrigin = localPlayerCharacter.GetOrigin();
		
		array<int> groupArray;
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (playersGroup && m_SettingsManager.GetCSISettingBool(CSI_SettingsManager.RADAR_VISIBLE))
			groupArray = CSI_UIHelper.GetSortedGroupArray(playersGroup.GetPlayerIDs());
		int groupCount = groupArray.Count();

		if (groupCount > 0)
		{
			foreach (int i, int playerId : groupArray)
			{
				float x, y, opacity, rotation, disT, dis, searchRadius;
				SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));

				if (!playerCharacter)
					continue;
				
				vector playerCharacterOrigin = playerCharacter.GetOrigin();

				// Get Distance
				dis = vector.Distance(localPlayerCharacterOrigin, playerCharacterOrigin);
				
				if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(localPlayerCharacter))
				{
					searchRadius = 8;
					disT = dis * 6.215;
				} else {
					searchRadius = 24;
					disT = dis * 2.0;
				};
				
				if (dis > searchRadius) 
					continue;

				if (playerId != SCR_PlayerController.GetLocalPlayerId())
				{
					// Get Direction
					float dir = vector.Direction(playerCharacterOrigin, localPlayerCharacterOrigin).ToYaw();
		
					// Get Relative Direction
					float relDir = Math.Mod(((dir - yaw) + 360), 360);
					relDir = Math.Mod(relDir - (dir * 2), 360);
					relDir = relDir * Math.DEG2RAD;
		
					x = (Math.Sin(relDir) * disT);
					y = (Math.Cos(relDir) * disT);
				};

				opacity = Math.Map(dis, (0.8*searchRadius), searchRadius, 0.6, 0);
				rotation = -Math.Mod((CSI_ChararcterHelper.GetCharacterYaw(playerCharacter) - yaw), 360);

				UpdatePlayerRadarWidget(i, playerId, opacity, x, y, rotation);
			};
		} else 
			groupCount = 0;

		for (int e = groupCount; e <= 24; e++)
			UpdatePlayerRadarWidget(e, -1, 0, 0, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerRadarWidget(int widgetNumber, int playerId, float opacity, float x, float y, float rotation)
	{
		Widget radarIcon = m_wRoot.FindAnyWidget(string.Format("RadarIcon%1", widgetNumber));

		if (radarIcon) 
		{
			float iconOpacity = radarIcon.GetOpacity();
			if (iconOpacity == 0 && iconOpacity == opacity)
				return;
			
			CSI_Icon icon = CSI_Icon.Cast(radarIcon.FindHandler(CSI_Icon));
			
			icon.IconUpdate(playerId);
			
			float widthAndHeight = ICON_WIDTH_AND_HEIGHT * (m_SettingsManager.GetCSISettingInt(CSI_SettingsManager.RADAR_ICON_SIZE) * 0.01);

			FrameSlot.SetPos(radarIcon, (x - widthAndHeight/2), (y - widthAndHeight/2));
			FrameSlot.SetSize(radarIcon, widthAndHeight, widthAndHeight);

			radarIcon.SetOpacity(opacity);
			icon.SetRotation(rotation);
		};
	}
}