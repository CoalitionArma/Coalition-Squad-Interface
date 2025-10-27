class CSI_Compass : SCR_ScriptedWidgetComponent
{
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	protected SCR_AIGroup m_PlayersGroup;
	protected PlayerManager m_PlayerManager;
	protected CSI_ClientManager m_ClientComponent;
	protected SCR_ChimeraCharacter m_ChimeraCharacter;
	protected CSI_AuthorityManager m_AuthorityComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	protected vector m_vOwnerOrigin;
	protected float m_fYaw, m_fStoredYaw;
	protected ref array<SCR_ChimeraCharacter> m_aAllPlayersWithinRange;

	protected string m_sCompassTexture, m_sSquadRadarIconSize;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_AuthorityComponent.GetInstance();
		m_ClientComponent.GetInstance();
		m_GroupsManagerComponent.GetInstance();
		m_PlayerManager = GetGame().GetPlayerManager();
		
		m_wCompass = ImageWidget.Cast(w.FindAnyWidget("Compass"));
		m_wBearing = TextWidget.Cast(w.FindAnyWidget("Bearing"));
	}
	
	//------------------------------------------------------------------------------------------------
	void Update()
	{		
		m_ChimeraCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		
		if (!m_ChimeraCharacter) 
			return;
		
		SetBearingAndCompass();

		SquadRadarSearch();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBearingAndCompass()
	{
		AimingComponent playerControllerComponent = m_ChimeraCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) 
			return;

		// Freelook Direction
		float altYaw = playerControllerComponent.GetAimingDirection().ToYaw();

		if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter) || altYaw > 0)
		{
			vector transform[4];
			GetGame().GetWorld().GetCurrentCamera(transform);

			m_fYaw = -Math3D.MatrixToAngles(transform)[0];
			
			if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter)) 
				m_fStoredYaw = m_fYaw;
		} else {
			m_fYaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();
			m_fStoredYaw = m_fYaw;
		};

		int yawInt = -m_fYaw;
		
		if (yawInt < 0) 
			yawInt = 360 - Math.AbsFloat(yawInt);

		m_wCompass.SetRotation(m_fYaw);
		// Set m_wBearing so if yaw outputs 6 it'll read 006
		string bearingAdd = "";
		
		// I love this.
		if (yawInt >= 0 & yawInt < 10) 
			bearingAdd = "00";
		
		if (yawInt >= 10 & yawInt < 100) 
			bearingAdd = "0";
		
		string bearingStr = bearingAdd + (yawInt.ToString());

		m_wBearing.SetText(bearingStr);
	}

	//------------------------------------------------------------------------------------------------
	protected void SquadRadarSearch() 
	{
		m_aAllPlayersWithinRange = {};

		string squadRadarSelfIconVisible = "";
		int opacity = 1;
		squadRadarSelfIconVisible = m_ClientComponent.ReturnLocalCSISettings()[12];

		if (squadRadarSelfIconVisible == "false") 
			opacity = 0;
		
		m_sSquadRadarIconSize = m_ClientComponent.ReturnLocalCSISettings()[11];
		
		ImageWidget radarlocalPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget("LocalPlayer"));
		
		
		
		m_vOwnerOrigin = m_ChimeraCharacter.GetOrigin();
		
		UpdateSquadRadarPositions();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSquadRadarPositions()
	{	
		SCR_AIGroup playersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		foreach (int i, int playerId : playersGroup.GetPlayerIDs())
		{
			SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(m_PlayerManager.GetPlayerControlledEntity(playerId));
			
			if (!playerCharacter)
				continue
			
			vector playerCharacterOrigin = playerCharacter.GetOrigin();

			// Get Distance
			float dis = vector.Distance(m_vOwnerOrigin, playerCharacterOrigin);
			float disT = dis * 2.0;
			
			int searchRadius = 24;
			
			if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter))
				searchRadius = 8;
			
			if (dis > searchRadius) 
				continue;
			
			ImageWidget radarPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", i)));

			if (CSI_ChararcterHelper.GetCharacterVehicleCompartment(m_ChimeraCharacter)) 
				disT = dis * 6.215;

			// Get Direction
			vector dirV = vector.Direction(playerCharacterOrigin, m_vOwnerOrigin);
			float dir = dirV.ToYaw();

			// Get Relative Direction
			float relDir = Math.Mod(((dir - m_fYaw) + 360), 360);
			relDir = Math.Mod(relDir - (dir * 2), 360);
			relDir = relDir * Math.DEG2RAD;

			float x = (Math.Sin(relDir) * disT);
			float y = (Math.Cos(relDir) * disT);

			float widthAndHeight = 16 * (m_sSquadRadarIconSize.ToInt() * 0.01);
			y = y - 76;
			
			FrameSlot.SetPos(radarPlayer, (x - widthAndHeight/2), y);
			FrameSlot.SetSize(radarPlayer, widthAndHeight, widthAndHeight);

			radarPlayer.SetOpacity(Math.Map(dis, (0.8*searchRadius), searchRadius, 0.6, 0));
			radarPlayer.SetRotation(-Math.Mod((CSI_ChararcterHelper.GetCharacterYaw(playerCharacter) - m_fYaw), 360));
		};
	}
}
