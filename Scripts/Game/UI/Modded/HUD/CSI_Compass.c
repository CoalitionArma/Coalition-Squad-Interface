class CSI_Compass : SCR_InfoDisplay
{
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	protected SCR_ChimeraCharacter m_ChimeraCharacter;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected CSI_ClientComponent m_ClientComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected SCR_AIGroup m_PlayersGroup;
	
	protected float m_fYaw;
	protected float m_fStoredYaw;
	protected float m_iSearchRadius;
	protected vector m_vOwnerOrigin;
	protected string m_sCompassVisible;
	protected string m_sSquadRadarVisible;
	protected int m_iSquadRadarIconSize;
	protected string m_sCompassTexture;
	protected ref array<SCR_ChimeraCharacter> m_aAllPlayersWithinRange;
	
	protected int m_iCheckSettingsFrame = 65;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
				
		if (!m_AuthorityComponent) {
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			return;
		};
		
		if (!m_ClientComponent) {
			m_ClientComponent = CSI_ClientComponent.GetInstance();
			return;
		};
		
		if (m_iCheckSettingsFrame >= 65) {
			m_iCheckSettingsFrame = 0;
			
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassTexture", m_sCompassTexture);
			if (m_sCompassTexture == "") m_sCompassTexture = "{D19C93F5109F3E1D}UI\Textures\HUD\Modded\Compasses\compass_shadow360.edds";
			
			GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarIconSize", m_iSquadRadarIconSize);
			if (m_iSquadRadarIconSize == 0) m_iSquadRadarIconSize = 100;
			
			string compassVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[0];
			switch (true)
			{
				case(compassVisibleSO == "true" || compassVisibleSO == "false") : { m_sCompassVisible = compassVisibleSO; break; };
				default : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("compassVisible", m_sCompassVisible); break; };
			};
			
			string squadRadarVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[1];
			switch (true)
			{
				case(squadRadarVisibleSO == "true" || squadRadarVisibleSO == "false") : { m_sSquadRadarVisible = squadRadarVisibleSO; break; };
				default : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarVisible", m_sSquadRadarVisible); break; };
			};
		} else { m_iCheckSettingsFrame++; };

		//Refresh base widgets if we can't get them
		if (!m_wBearing || !m_wCompass) {
			m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
			return;
		};
		
		if (m_sCompassVisible == "false") {
			m_wCompass.SetOpacity(0);
			m_wBearing.SetOpacity(0);
			ClearSquadRadar(-1);
			return;
		};
		
		m_ChimeraCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		
		if (!m_ChimeraCharacter) return;

		// Sets m_wBearings text and the m_wCompass direction
		SetBearingAndCompass();
		
		if (m_sSquadRadarVisible == "false") {
			ClearSquadRadar(-1);
			return;
		};

		if (!m_GroupsManagerComponent) {
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			return;
		};
				
		// Get players current group.
		m_PlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (!m_PlayersGroup) {
			ClearSquadRadar(-1);
			return;
		};
		
		SquadRadarSearch();
	}

	//------------------------------------------------------------------------------------------------

	// Compass Functions

	//------------------------------------------------------------------------------------------------

	protected void SetBearingAndCompass()
	{	
		m_wCompass.SetOpacity(1);
		m_wBearing.SetOpacity(1);
		
		m_wCompass.LoadImageTexture(0, m_sCompassTexture);
		
		AimingComponent playerControllerComponent = m_ChimeraCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) return;
		
		// Freelook Direction
		float altYaw = playerControllerComponent.GetAimingDirection().ToYaw();

		if (IsPlayerInVehicle(m_ChimeraCharacter) || altYaw > 0)
		{
			vector transform[4];
			GetGame().GetWorld().GetCurrentCamera(transform);

			m_fYaw = -Math3D.MatrixToAngles(transform)[0];
			if (IsPlayerInVehicle(m_ChimeraCharacter)) m_fStoredYaw = m_fYaw;
		} else {
			m_fYaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();
			m_fStoredYaw = m_fYaw;
		};
		
		int yawInt = -m_fYaw;
		if (yawInt < 0) { yawInt = 360 - Math.AbsFloat(yawInt); };
		
		m_wCompass.SetRotation(m_fYaw);
		// Set m_wBearing so if yaw outputs 6 it'll read 006
		string bearingAdd = "";
		// I love this.
		if (yawInt >= 0 & yawInt < 10) { bearingAdd = "00"; };
		if (yawInt >= 10 & yawInt < 100) { bearingAdd = "0"; };
		string bearingStr = bearingAdd + (yawInt.ToString());

		m_wBearing.SetText(bearingStr);
	}
	
	//------------------------------------------------------------------------------------------------

	// Squad Radar Functions

	//------------------------------------------------------------------------------------------------
	
	protected void SquadRadarSearch() {
		
		array<string> groupArray = m_ClientComponent.GetLocalGroupArray();
		
		m_aAllPlayersWithinRange = {};
		
		if (!groupArray || groupArray.Count() <= 1) {
			ClearSquadRadar(-1);
			return;
		};
		
		float widthAndHeight = 12 * (m_iSquadRadarIconSize * 0.01);
		
		ImageWidget radarlocalPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget("LocalPlayer"));
		
		string squadRadarSelfIconVisible = "";
		int opacity = 1;
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("squadRadarSelfIconVisible", squadRadarSelfIconVisible);
		
		if (squadRadarSelfIconVisible == "false") opacity = 0;
		
		SetSquadRadarImage(radarlocalPlayer, widthAndHeight, opacity, m_fStoredYaw, m_ChimeraCharacter);
		
		m_vOwnerOrigin = m_ChimeraCharacter.GetOrigin();
		
		m_iSearchRadius = 24;
		
		if (IsPlayerInVehicle(m_ChimeraCharacter)) m_iSearchRadius = 8;
		
		GetGame().GetWorld().QueryEntitiesBySphere(m_vOwnerOrigin, m_iSearchRadius, ProcessEntity, null, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);
		
		UpdateSquadRadarPositions();
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessEntity(IEntity processEntity)
	{
		SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(processEntity);
		if (!playerCharacter || playerCharacter == m_ChimeraCharacter) return true;
		
		int processEntityID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerCharacter);
		if(!processEntityID || !m_PlayersGroup.IsPlayerInGroup(processEntityID)) return true;
		
		m_aAllPlayersWithinRange.Insert(playerCharacter);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSquadRadarPositions()
	{
		int posToStartClearing = 0;
		foreach (int i, SCR_ChimeraCharacter playerCharacter : m_aAllPlayersWithinRange)
		{
			ImageWidget radarPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", i)));
			vector playerCharacterOrigin = playerCharacter.GetOrigin();
			
			int offset = 75;
			
			float widthAndHeight = 12 * (m_iSquadRadarIconSize * 0.01);
		
			// Get Distance
			float dis = vector.Distance(m_vOwnerOrigin, playerCharacterOrigin);
			float disT = dis * 2.0;
			
			if (IsPlayerInVehicle(playerCharacter)) offset = 74;
			if (IsPlayerInVehicle(m_ChimeraCharacter)) disT = dis * 6.475;
			
			// Get Direction
			vector dirV = vector.Direction(playerCharacterOrigin, m_vOwnerOrigin);
			float dir = dirV.ToYaw();

			// Get Relative Direction
			float relDir = Math.Mod(((dir - m_fYaw) + 360), 360);
			relDir = Math.Mod(relDir - (dir * 2), 360);
			relDir = relDir * Math.DEG2RAD;
			
			float x = (Math.Sin(relDir) * disT) - widthAndHeight/2;
			float y = (Math.Cos(relDir) * disT) - offset;
		
			FrameSlot.SetPos(radarPlayer, x, y);
			
			SetSquadRadarImage(radarPlayer, widthAndHeight, Math.Map(dis, 0, m_iSearchRadius, 4, 0), m_fYaw, playerCharacter);
			posToStartClearing = i + 1;
		};
		ClearSquadRadar(posToStartClearing);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSquadRadarImage(ImageWidget radarPlayer, float widthAndHeight, float opacity, float yaw, SCR_ChimeraCharacter playerCharacter)
	{
		int processEntityID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerCharacter);
		int groupID = m_PlayersGroup.GetGroupID();
		
		string colorTeam = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, processEntityID, "ColorTeam");
		string value     = m_AuthorityComponent.ReturnLocalPlayerMapValue(-1, processEntityID, "PlayerValue");
		string icon      = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, processEntityID, "DisplayIcon");
		
		int valueInt = value.ToInt();
		
		if (valueInt == 1) valueInt = -10;
		if (valueInt >= 2) valueInt = -11;
		
		radarPlayer.SetOpacity(opacity);
		radarPlayer.LoadImageTexture(0, icon);
		radarPlayer.SetZOrder(valueInt);
		radarPlayer.SetColorInt(colorTeam.ToInt());
		FrameSlot.SetSize(radarPlayer, widthAndHeight, widthAndHeight);
		
		radarPlayer.SetRotation(-Math.Mod((GetPlayersYaw(playerCharacter) - yaw), 360));
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetPlayersYaw(SCR_ChimeraCharacter playerCharacter)
	{
		AimingComponent playerControllerComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) return 0;
		
		float yaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				yaw = -playerCharacter.GetYawPitchRoll()[0];
			};
		}
		return yaw;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsPlayerInVehicle(SCR_ChimeraCharacter playerCharacter)
	{
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess) {
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment) return true;
		};
		return false;
	};
	
	//------------------------------------------------------------------------------------------------
	protected void ClearSquadRadar(int positionToStartClearing)
	{
		if (positionToStartClearing == -1) {
			ImageWidget radarlocalPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget("LocalPlayer"));
			radarlocalPlayer.SetOpacity(0);
		};
		
		for (int e = positionToStartClearing; e <= 24; e++)
		{
			ImageWidget removeRadarPlayerWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", e)));
			if (!removeRadarPlayerWidget) continue;
			removeRadarPlayerWidget.SetOpacity(0);
		};
	}
}
