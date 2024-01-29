class COA_Compass : SCR_InfoDisplay
{
	protected TextWidget m_wBearing = null;
	protected ImageWidget m_wCompass = null;
	protected SCR_ChimeraCharacter m_ChimeraCharacter = null;
	protected COA_GroupDisplayManagerComponent m_GroupDisplayManagerComponent = null;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent = null;
	protected SCR_AIGroup m_PlayersGroup = null;
	
	protected float m_fYaw;
	protected vector m_vOwnerOrigin;
	protected bool m_bCompassVisible = true;
	protected ref array<SCR_ChimeraCharacter> m_aAllPlayersWithinRange;

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

		//Refresh base widgets if we can't get them
		if (!m_wBearing || !m_wCompass) {
			m_wBearing = TextWidget.Cast(m_wRoot.FindAnyWidget("Bearing"));
			m_wCompass = ImageWidget.Cast(m_wRoot.FindAnyWidget("Compass"));
			return;
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
		
		m_ChimeraCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		
		if (!m_ChimeraCharacter) return;

		// Sets m_wBearings text and the m_wCompass direction
		SetBearingAndCompass();

		if (!m_GroupsManagerComponent) {
			m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
			return;
		};
		
		if (!m_GroupDisplayManagerComponent) {
			m_GroupDisplayManagerComponent = COA_GroupDisplayManagerComponent.GetInstance();
			return;
		};
		
		// Get players current group.
		m_PlayersGroup = m_GroupsManagerComponent.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (!m_PlayersGroup) {
			ClearSquadRadar(0);
			return;
		};
		
		SquadRadarSearch();
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
		m_fYaw = playerControllerComponent.GetAimingDirectionWorld().VectorToAngles()[0];
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(m_ChimeraCharacter.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				vector transform[4];
				GetGame().GetWorld().GetCurrentCamera(transform);

				m_fYaw = -Math3D.MatrixToAngles(transform)[0];
			};
		}
		
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
		
		array<string> groupArray = m_GroupDisplayManagerComponent.GetLocalGroupArray();
		
		m_aAllPlayersWithinRange = {};
		
		if (!groupArray || groupArray.Count() <= 1) {
			ClearSquadRadar(0);
			return;
		};
		
		ImageWidget radarlocalPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget("LocalPlayer"));
		SetSquadRadarImage(radarlocalPlayer, m_ChimeraCharacter, 1);
		
		m_vOwnerOrigin = m_ChimeraCharacter.GetOrigin();
		GetGame().GetWorld().QueryEntitiesBySphere(m_vOwnerOrigin, 18, ProcessEntity, null, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);
		
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
		
			// Get Distance
			float dis = vector.Distance(m_vOwnerOrigin, playerCharacterOrigin);
			float disT = dis * 4.345;
			
			// Get Direction
			vector dirV = vector.Direction(playerCharacterOrigin, m_vOwnerOrigin);
			float dir = dirV.ToYaw();

			// Get Relative Direction
			float relDir = Math.Mod(((dir - m_fYaw) + 360), 360);
			relDir = Math.Mod(relDir - (dir * 2), 360);
			relDir = relDir * Math.DEG2RAD;
			
			float x = (Math.Sin(relDir) * disT) - 9.75;
			float y = (Math.Cos(relDir) * disT) - 9.75;
		
			FrameSlot.SetPos(radarPlayer, x, y);
			
			SetSquadRadarImage(radarPlayer, playerCharacter, Math.Map(dis, 0, 18, 6, 0));
			posToStartClearing = i;
		};
		ClearSquadRadar(posToStartClearing + 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSquadRadarImage(ImageWidget radarPlayer, SCR_ChimeraCharacter playerCharacter, float opacity)
	{
		int processEntityID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerCharacter);
		
		string colorTeam = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), processEntityID, "ColorTeam");
		string icon      = m_GroupDisplayManagerComponent.ReturnLocalPlayerMapValue(m_PlayersGroup.GetGroupID(), processEntityID, "DisplayIcon");
		
		radarPlayer.SetOpacity(opacity);
		radarPlayer.LoadImageTexture(0, icon);
		radarPlayer.SetColorInt(colorTeam.ToInt());
		
		if (playerCharacter != m_ChimeraCharacter)
			radarPlayer.SetRotation(-Math.Mod((GetPlayersYaw(playerCharacter) - m_fYaw), 360));
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetPlayersYaw(SCR_ChimeraCharacter playerCharacter)
	{
		AimingComponent playerControllerComponent = playerCharacter.GetHeadAimingComponent();
		if (!playerControllerComponent) return 0;
		float yaw = playerControllerComponent.GetAimingDirectionWorld().VectorToAngles()[0];
		
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
		if (positionToStartClearing == 0) {
			ImageWidget radarlocalPlayer = ImageWidget.Cast(m_wRoot.FindAnyWidget("LocalPlayer"));
			radarlocalPlayer.SetOpacity(0);
		};
		
		for (int e = positionToStartClearing; e <= 19; e++)
		{
			ImageWidget removeRadarPlayerWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(string.Format("RadarPlayer%1", e)));
			if (!removeRadarPlayerWidget) continue;
			removeRadarPlayerWidget.SetOpacity(0);
		};
	}
}
