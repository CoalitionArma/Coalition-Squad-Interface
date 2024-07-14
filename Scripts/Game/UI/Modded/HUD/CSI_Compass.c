class CSI_Compass : SCR_InfoDisplay
{
	protected TextWidget m_wBearing;
	protected ImageWidget m_wCompass;
	protected SCR_AIGroup m_PlayersGroup;
	protected PlayerManager m_PlayerManager;
	protected CSI_ClientComponent m_ClientComponent;
	protected SCR_ChimeraCharacter m_ChimeraCharacter;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;

	protected vector m_vOwnerOrigin;
	protected float m_fYaw, m_fStoredYaw, m_iSearchRadius;
	protected ref array<SCR_ChimeraCharacter> m_aAllPlayersWithinRange;

	protected string m_sCompassTexture, m_sSquadRadarIconSize;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		m_ChimeraCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		
		if (!m_ChimeraCharacter) 
			return;

		if (!m_AuthorityComponent || !m_ClientComponent || !m_GroupsManagerComponent || !m_wBearing || !m_wCompass || !m_PlayerManager) 
		{
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			m_ClientComponent = CSI_ClientComponent.GetInstance();
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
			m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			m_PlayerManager = GetGame().GetPlayerManager();
			return;
		};
		
		// Get players current group.
		m_PlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		string compassVisible = m_ClientComponent.ReturnLocalCSISettings()[0];
		string squadRadarVisible = m_ClientComponent.ReturnLocalCSISettings()[1];

		if (compassVisible == "false") 
		{
			if (m_wCompass.GetOpacity() > 0) 
			{
				m_wCompass.SetOpacity(0);
				m_wBearing.SetOpacity(0);
			};
		} else {
			if (m_wCompass.GetOpacity() < 1) 
			{
				m_wCompass.SetOpacity(1);
				m_wBearing.SetOpacity(1);
			
				m_sCompassTexture = m_ClientComponent.ReturnLocalCSISettings()[14];
				m_wCompass.LoadImageTexture(0, m_sCompassTexture);
			};
		};
		
		SetBearingAndCompass(compassVisible);

		if (squadRadarVisible == "false" || !m_PlayersGroup) 
			ClearSquadRadar(-1);
		else
			SquadRadarSearch();
	}

	//------------------------------------------------------------------------------------------------

	// Compass Functions

	//------------------------------------------------------------------------------------------------

	protected void SetBearingAndCompass(string compassVisible)
	{
		AimingComponent playerControllerComponent = m_ChimeraCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) 
			return;

		// Freelook Direction
		float altYaw = playerControllerComponent.GetAimingDirection().ToYaw();

		if (IsPlayerInVehicle(m_ChimeraCharacter) || altYaw > 0)
		{
			vector transform[4];
			GetGame().GetWorld().GetCurrentCamera(transform);

			m_fYaw = -Math3D.MatrixToAngles(transform)[0];
			
			if (IsPlayerInVehicle(m_ChimeraCharacter)) 
				m_fStoredYaw = m_fYaw;
		} else {
			m_fYaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();
			m_fStoredYaw = m_fYaw;
		};
		
		if (compassVisible == "false") 
			return;

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

	// Squad Radar Functions

	//------------------------------------------------------------------------------------------------

	protected void SquadRadarSearch() 
	{
		m_aAllPlayersWithinRange = {};

		array<string> groupArray = m_ClientComponent.GetLocalGroupArray();
		if (!groupArray || groupArray.Count() <= 1) 
		{
			ClearSquadRadar(-1);
			return;
		};

		string squadRadarSelfIconVisible = "";
		int opacity = 1;
		squadRadarSelfIconVisible = m_ClientComponent.ReturnLocalCSISettings()[12];

		if (squadRadarSelfIconVisible == "false") 
			opacity = 0;

		m_iSearchRadius = 24;
		if (IsPlayerInVehicle(m_ChimeraCharacter)) 
			m_iSearchRadius = 8;
		
		m_sSquadRadarIconSize = m_ClientComponent.ReturnLocalCSISettings()[11];
		
		ImageWidget radarlocalPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget("LocalPlayer"));
		SetSquadRadarImage(radarlocalPlayer, 0, 0, m_fStoredYaw, opacity, m_ChimeraCharacter);

		m_vOwnerOrigin = m_ChimeraCharacter.GetOrigin();
		GetGame().GetWorld().QueryEntitiesBySphere(m_vOwnerOrigin, m_iSearchRadius, ProcessEntity, null, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);

		UpdateSquadRadarPositions();
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessEntity(IEntity processEntity)
	{
		SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(processEntity);
		if (!playerCharacter || playerCharacter == m_ChimeraCharacter) 
			return true;

		int processEntityID = m_PlayerManager.GetPlayerIdFromControlledEntity(playerCharacter);
		if (!processEntityID || !m_PlayersGroup.IsPlayerInGroup(processEntityID)) 
			return true;

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

			// Get Distance
			float dis = vector.Distance(m_vOwnerOrigin, playerCharacterOrigin);
			float disT = dis * 2.0;

			if (IsPlayerInVehicle(m_ChimeraCharacter)) 
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

			SetSquadRadarImage(radarPlayer, x, y, m_fYaw, Math.Map(dis, 0, m_iSearchRadius, 4, 0), playerCharacter);
			posToStartClearing = i + 1;
		};
		ClearSquadRadar(posToStartClearing);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetSquadRadarImage(ImageWidget radarPlayer, float x, float y, float yaw, float opacity, SCR_ChimeraCharacter playerCharacter)
	{
		int processEntityID = m_PlayerManager.GetPlayerIdFromControlledEntity(playerCharacter);
		int groupID = m_PlayersGroup.GetGroupID();

		string iconString = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, processEntityID, "DI"); // DI = DisplayIcon
		string colorTeamString = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, processEntityID, "CT"); // CT = ColorTeam
		string storedSpecialtyIconString = m_AuthorityComponent.ReturnLocalPlayerMapValue(groupID, processEntityID, "SSI"); // SSI = StoredSpecialtyIcon

		if (iconString.IsEmpty() || storedSpecialtyIconString.IsEmpty()) 
			return;
		
		int initialWidthAndHeight = 16;
		
		if (iconString == "DRV" || iconString == "PAX")
			initialWidthAndHeight = 12;
		
		if (m_fStoredYaw == yaw && iconString == "GNR")
			y = y - 78.5;
		else
			y = y - 76;
		
		float widthAndHeight = initialWidthAndHeight * (m_sSquadRadarIconSize.ToInt() * 0.01);
		
		FrameSlot.SetPos(radarPlayer, (x - widthAndHeight/2), y);
		FrameSlot.SetSize(radarPlayer, widthAndHeight, widthAndHeight);

		radarPlayer.SetOpacity(opacity);
		radarPlayer.SetRotation(-Math.Mod((GetPlayersYaw(playerCharacter) - yaw), 360));
		radarPlayer.LoadImageTexture(0, m_ClientComponent.SwitchStringToIcon(iconString));
		radarPlayer.SetColorInt(m_ClientComponent.SwitchStringToColorTeam(colorTeamString));
		radarPlayer.SetZOrder(DeterminePlayerValue(storedSpecialtyIconString, colorTeamString));
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetPlayersYaw(SCR_ChimeraCharacter playerCharacter)
	{
		AimingComponent playerControllerComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) 
			return 0;

		float yaw = playerControllerComponent.GetAimingDirectionWorld().ToYaw();

		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
				yaw = -playerCharacter.GetYawPitchRoll()[0];
		}
		return yaw;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsPlayerInVehicle(SCR_ChimeraCharacter playerCharacter)
	{
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess) 
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment) 
				return true;
		};
		return false;
	};

	//------------------------------------------------------------------------------------------------
	protected void ClearSquadRadar(int positionToStartClearing)
	{
		if (positionToStartClearing == -1) 
		{
			ImageWidget radarlocalPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget("LocalPlayer"));
			radarlocalPlayer.SetOpacity(0);
		};

		for (int e = positionToStartClearing; e <= 24; e++)
		{
			ImageWidget removeRadarPlayerWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", e)));
			
			if (!removeRadarPlayerWidget) 
				continue;
			
			removeRadarPlayerWidget.SetOpacity(0);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	int DeterminePlayerValue(string icon, string colorTeam)
	{
		int value = 0;
		
		switch (colorTeam) {
			case "R" : {value = -3; break;};
			case "B" : {value = -5; break;};
			case "Y" : {value = -7; break;};
			case "G" : {value = -9; break;};
			default  : {value = 2;  break;};
		};
		
		switch (true) {
			case (icon == "SL")                        : {value = -1; break;};
			case (icon == "FTL" && colorTeam == "N/A") : {value--;    break;};
			case (icon == "FTL" && colorTeam != "N/A") : {value++;    break;};
		};

		if (value == 1) 
			value = -10;
		
		if (value >= 2) 
			value = -11;
		
		return value;
	}
}
