class CSI_Radar : SCR_ScriptedWidgetComponent
{
	protected CSI_ClientManager m_ClientComponent;
	protected SCR_ChimeraCharacter m_ChimeraCharacter;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	protected static int ICON_WIDTH_AND_HEIGHT = 16;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_ClientComponent = CSI_ClientManager.GetInstance();
		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{		
		m_ChimeraCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());
		
		if (!m_ChimeraCharacter) 
			return;

		// Freelook Direction
        float yaw = CSI_ChararcterHelper.GetLocalAimingYaw();
		vector localPlayerCharacterOrigin = m_ChimeraCharacter.GetOrigin();
		
		array<int> groupArray = {};
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		if (playersGroup)
			groupArray = playersGroup.GetPlayerIDs();

		foreach (int i, int playerId : groupArray)
		{
			float x, y, opacity, rotation, disT, dis, searchRadius;
			SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));

			if (!playerCharacter)
				continue;
			
			vector playerCharacterOrigin = playerCharacter.GetOrigin();

			// Get Distance
			dis = vector.Distance(localPlayerCharacterOrigin, playerCharacterOrigin);
			
			if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter))
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

		for (int e = groupArray.Count(), e <= 24; e++)
			UpdatePlayerRadarWidget(e, -1, 0, 0, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerRadarWidget(int widgetNumber, int playerId, float opacity, float x, float y, float rotation)
	{
		Widget radarIcon = m_wRoot.FindAnyWidget(string.Format("RadarIcon%1", widgetNumber));

		if (radarIcon) 
		{
			CSI_Icon icon = CSI_Icon.Cast(radarIcon.FindHandler(CSI_Icon));

			icon.IconUpdate(playerId);

			FrameSlot.SetPos(radarIcon, (x - ICON_WIDTH_AND_HEIGHT/2), (y - ICON_WIDTH_AND_HEIGHT/2));
			FrameSlot.SetSize(radarIcon, ICON_WIDTH_AND_HEIGHT, ICON_WIDTH_AND_HEIGHT);

			radarIcon.SetOpacity(opacity);
			icon.SetRotation(rotation);
		};
	}
}