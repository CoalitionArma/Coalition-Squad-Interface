class CSI_Radar : SCR_ScriptedWidgetComponent
{
	protected CSI_ClientManager m_ClientComponent;
	protected SCR_ChimeraCharacter m_ChimeraCharacter;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_ClientComponent.GetInstance();
		m_GroupsManagerComponent.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{		
		m_ChimeraCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());
		
		if (!m_ChimeraCharacter) 
			return;

		// Freelook Direction
        float yaw = CSI_ChararcterHelper.GetLocalAimingYaw();

		//squadRadarSelfIconVisible = m_ClientComponent.ReturnLocalCSISettings()[12];
		string m_sSquadRadarIconSize = m_ClientComponent.ReturnLocalCSISettings()[11];
		
		vector localPlayerCharacterOrigin = m_ChimeraCharacter.GetOrigin();
		
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		int positionToStartClearing;

		foreach (int i, int playerId : playersGroup.GetPlayerIDs())
		{
			SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalMainEntity());
			
			if (!playerCharacter)
				continue;
			
			vector playerCharacterOrigin = playerCharacter.GetOrigin();

			// Get Distance
			float dis = vector.Distance(localPlayerCharacterOrigin, playerCharacterOrigin);
			
			int searchRadius = 24;
			
			if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter))
				searchRadius = 8;
			
			if (dis > searchRadius) 
				continue;
			
			ImageWidget radarPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", i)));

            CSI_Icon icon = CSI_Icon.Cast(radarPlayer.FindHandler(CSI_Icon));
            icon.IconUpdate(playerId);

			float disT = dis * 2.0;
			
			if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter)) 
				disT = dis * 6.215;

			// Get Direction
			vector dirV = vector.Direction(playerCharacterOrigin, localPlayerCharacterOrigin);
			float dir = dirV.ToYaw();

			// Get Relative Direction
			float relDir = Math.Mod(((dir - yaw) + 360), 360);
			relDir = Math.Mod(relDir - (dir * 2), 360);
			relDir = relDir * Math.DEG2RAD;

			float x = (Math.Sin(relDir) * disT);
			float y = (Math.Cos(relDir) * disT);

			float widthAndHeight = 16 * (m_sSquadRadarIconSize.ToInt() * 0.01);
			y = y - 76;
			
			FrameSlot.SetPos(radarPlayer, (x - widthAndHeight/2), y);
			FrameSlot.SetSize(radarPlayer, widthAndHeight, widthAndHeight);

			radarPlayer.SetOpacity(Math.Map(dis, (0.8*searchRadius), searchRadius, 0.6, 0));
			radarPlayer.SetRotation(-Math.Mod((CSI_ChararcterHelper.GetCharacterYaw(playerCharacter) - yaw), 360));

            positionToStartClearing++;
		};

		for (int e = positionToStartClearing; e <= 24; e++)
		{
			ImageWidget removeRadarPlayerWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", e)));
			
			if (!removeRadarPlayerWidget) 
				continue;
			
			removeRadarPlayerWidget.SetVisible(false);
		};
	}
}