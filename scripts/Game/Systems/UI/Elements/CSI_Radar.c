class CSI_Radar : SCR_ScriptedWidgetComponent
{
	protected CSI_ClientManager m_ClientComponent;
	protected SCR_ChimeraCharacter m_ChimeraCharacter;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

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

		//squadRadarSelfIconVisible = m_ClientComponent.ReturnLocalCSISettings()[12];
		//string m_sSquadRadarIconSize = m_ClientComponent.ReturnLocalCSISettings()[11];
		
		vector localPlayerCharacterOrigin = m_ChimeraCharacter.GetOrigin();
		
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		int positionToStartClearing;

		if(playersGroup)
		{
			foreach (int i, int playerId : playersGroup.GetPlayerIDs())
			{
				SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));
				Widget radarIcon = m_wRoot.FindAnyWidget(string.Format("RadarIcon%1", i));
	
				if (!radarIcon || !playerCharacter)
					continue;
				
				vector playerCharacterOrigin = playerCharacter.GetOrigin();
	
				// Get Distance
				float dis = vector.Distance(localPlayerCharacterOrigin, playerCharacterOrigin);
				
				float disT = dis * 2.0;
				int searchRadius = 24;
				
				if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter))
				{
					searchRadius = 8;
					disT = dis * 6.215;
				};
				
				if (dis > searchRadius) 
					continue;
				
				float x = 0;
				float y = 0 - 8;
				float rotation;
				
	            CSI_Icon icon = CSI_Icon.Cast(radarIcon.FindHandler(CSI_Icon));
	            icon.IconUpdate(playerId);
	
				if (playerId != SCR_PlayerController.GetLocalPlayerId())
				{
					// Get Direction
					float dir = vector.Direction(playerCharacterOrigin, localPlayerCharacterOrigin).ToYaw();
		
					// Get Relative Direction
					float relDir = Math.Mod(((dir - yaw) + 360), 360);
					relDir = Math.Mod(relDir - (dir * 2), 360);
					relDir = relDir * Math.DEG2RAD;
		
					x = (Math.Sin(relDir) * disT);
					y = (Math.Cos(relDir) * disT) - 8;
				};
	
				float widthAndHeight = 16; // * (m_sSquadRadarIconSize.ToInt() * 0.01);
				
				FrameSlot.SetPos(radarIcon, (x - widthAndHeight/2), y);
				FrameSlot.SetSize(radarIcon, widthAndHeight, widthAndHeight);
	
				radarIcon.SetOpacity(Math.Map(dis, (0.8*searchRadius), searchRadius, 0.6, 0));
				icon.SetRotation(-Math.Mod((CSI_ChararcterHelper.GetCharacterYaw(playerCharacter) - yaw), 360));
	
	            positionToStartClearing++;
			};
		};

		for (int e = positionToStartClearing; e <= 24; e++)
		{
			Widget RemoveRadarWidget = m_wRoot.FindAnyWidget(string.Format("RadarIcon%1", e));
			
			if (!RemoveRadarWidget) 
				continue;
			
            CSI_Icon RemoveRadarIcon = CSI_Icon.Cast(RemoveRadarWidget.FindHandler(CSI_Icon));
            RemoveRadarIcon.IconUpdate(0);
		};
	}
}