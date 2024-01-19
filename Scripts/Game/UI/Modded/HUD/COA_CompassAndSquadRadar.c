class COA_Compass : SCR_InfoDisplay
{
	protected TextWidget m_wBearing = null;
	protected ImageWidget m_wCompass = null;
	protected SCR_ChimeraCharacter m_ChimeraCharacter = null;
	protected COA_GroupDisplayManagerComponent m_GroupDisplayManagerComponent = null;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent = null;
	protected SCR_AIGroup m_PlayersGroup = null;
	
	private int m_iSquadRadarRefresh;
	private vector m_vOwnerOrigin;
	private bool m_bCompassVisible = true;
	private ref array<SCR_ChimeraCharacter> m_aAllPlayersWithinRange;

	//------------------------------------------------------------------------------------------------

	// Override/static functions

	//------------------------------------------------------------------------------------------------
	
	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetGame().GetInputManager().AddActionListener("ToggleCompass", EActionTrigger.DOWN, ToggleCompass);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		
		if (!m_GroupsManagerComponent)
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (!m_GroupDisplayManagerComponent)
			m_GroupDisplayManagerComponent = COA_GroupDisplayManagerComponent.GetInstance();
		
		// Get players current group.
		m_PlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

		//Refresh base widgets if we can't get them
		if (!m_wBearing || !m_wCompass) {
			m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
		};
		
		if (m_bCompassVisible)
		{
			m_wCompass.SetOpacity(1);
			m_wBearing.SetOpacity(1);
		}
		else
		{
			m_wCompass.SetOpacity(0);
			m_wBearing.SetOpacity(0);
			ClearSquadRadar(0);
			return;
		}

		// Can't run if these dont exist better exit out.
		if (!m_wBearing || !m_wCompass) return;
		
		m_ChimeraCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());

		// Sets m_wBearings text and the m_wCompass direction
		SetBearingAndCompass();
		
		// Only update the group displpay every 5 frames.
		if (m_iSquadRadarRefresh <= 5) {
			m_iSquadRadarRefresh++; 
			return;
		};
		
		SquadRadarSearch();
		m_iSquadRadarRefresh = 0;
	}

	//------------------------------------------------------------------------------------------------

	// Compass Functions

	//------------------------------------------------------------------------------------------------

	protected void ToggleCompass()
	{
		m_bCompassVisible = !m_bCompassVisible;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBearingAndCompass()
	{	
		AimingComponent playerControllerComponent = m_ChimeraCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) return;
		float yaw = playerControllerComponent.GetAimingDirectionWorld().VectorToAngles()[0];
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(m_ChimeraCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				vector transform[4];
				GetGame().GetWorld().GetCurrentCamera(transform);

				yaw = -Math3D.MatrixToAngles(transform)[0];
			};
		}
		
		int yawInt = -yaw;
		if (yawInt < 0) { yawInt = 360 - Math.AbsFloat(yawInt); };
		
		m_wCompass.SetRotation(yaw);

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
		
		if (!m_bCompassVisible)
		{
			m_wCompass.SetOpacity(0);
			m_wBearing.SetOpacity(0);
			ClearSquadRadar(0);
			return;
		}
		
		if (!m_GroupDisplayManagerComponent) return;
		
		array<string> groupArray = m_GroupDisplayManagerComponent.GetLocalGroupArray();
		
		m_aAllPlayersWithinRange = {};
		
		if (!groupArray || groupArray.Count() <= 1) {
			ClearSquadRadar(0);
			return;
		};
		
		SetSquadRadarImage(0, m_ChimeraCharacter);
		
		m_vOwnerOrigin = m_ChimeraCharacter.GetOrigin();
		GetGame().GetWorld().QueryEntitiesBySphere(m_vOwnerOrigin, 35, ProcessEntity, null, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);
		
		UpdateSquadRadarPositions();
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessEntity(IEntity processEntity)
	{
		SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(processEntity);
		if (!playerCharacter || playerCharacter == m_ChimeraCharacter) return true;
		
		int processEntityID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerCharacter);
		if(!processEntityID || !m_PlayersGroup.IsPlayerInGroup(processEntityID))return true;
		
		m_aAllPlayersWithinRange.Insert(playerCharacter);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSquadRadarPositions()
	{
		int posToStartClearing = 0;
		foreach (int i, SCR_ChimeraCharacter playerCharacter : m_aAllPlayersWithinRange)
		{
			i = i + 1;	
	
			vector processEntityOrigin = playerCharacter.GetOrigin();
		
			float distance = vector.Distance(m_vOwnerOrigin, processEntityOrigin);
		
			SetSquadRadarImage(i, playerCharacter);
			posToStartClearing = i;
		};
		ClearSquadRadar(posToStartClearing + 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSquadRadarImage(int radarPlayerWidgetInt, SCR_ChimeraCharacter playerCharacter)
	{
		ImageWidget radarPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", radarPlayerWidgetInt)));
		
		int processEntityID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerCharacter);
		
		string colorTeam = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), processEntityID, "ColorTeam");
		string icon      = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), processEntityID, "DisplayIcon");
		
		radarPlayer.SetOpacity(1);
		radarPlayer.LoadImageTexture(0, icon);
		
		switch (colorTeam)
		{
			case "Red"    : {radarPlayer.SetColorInt(ARGB(185, 200, 65, 65));   break;};
			case "Blue"   : {radarPlayer.SetColorInt(ARGB(185, 0, 92, 255));    break;};
			case "Yellow" : {radarPlayer.SetColorInt(ARGB(185, 230, 230, 0));   break;};
			case "Green"  : {radarPlayer.SetColorInt(ARGB(185, 0, 190, 85));    break;};
			case "None"   : {radarPlayer.SetColorInt(ARGB(185, 235, 235, 235)); break;};
		};
		
		if (radarPlayerWidgetInt != 0)
			radarPlayer.SetRotation(GetPlayersYaw(playerCharacter));
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetPlayersYaw(SCR_ChimeraCharacter playerCharacter)
	{
		AimingComponent playerControllerComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) return 0;
		float yaw = playerControllerComponent.GetAimingDirectionWorld().VectorToAngles()[0];
		yaw = -yaw;
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				yaw = playerCharacter.GetYawPitchRoll()[0];
			};
		}
		return yaw;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearSquadRadar(int positionToStartClearing)
	{
		for (int e = positionToStartClearing; e <= 19; e++)
		{
			ImageWidget removeRadarPlayerWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", e)));
			if (!removeRadarPlayerWidget) continue;
			removeRadarPlayerWidget.SetOpacity(0);
		};
	}
}
