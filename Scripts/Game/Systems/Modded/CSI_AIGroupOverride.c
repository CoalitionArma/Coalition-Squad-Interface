modded class SCR_AIGroup : ChimeraAIGroup
{
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Default slot color team, accepted values are: \n\n None \n Red \n Blue \n Yellow \n Green \n\nArray index should line up with the index of prefab in 'Unit Prefab Slots'", category: "Group Members")]
	private ref array<string> m_aUnitPrefabColorTeams;
	
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Default slot override icon, accepted values are: \n\n None \n Team Lead \n Medic \n Marksman \n Machine Gunner \n Anti-Tank \n Grenadier \n Demolitionist \n Engineer \n Man \n\nArray index should line up with the index of prefab in 'Unit Prefab Slots'", category: "Group Members")]
	private ref array<string> m_aUnitPrefabOverrideIcons;
	
	protected override bool SpawnGroupMember(bool snapToTerrain, int index, ResourceName res, bool editMode, bool isLast)
	{	
		if (!GetGame().GetAIWorld().CanLimitedAIBeAdded())
		{
			if (isLast) 
			{
				Event_OnInit.Invoke(this);
			};
			
			//Event_OnLastGroupMemberSpawned.Invoke(this);
			
			return true;
		}
		BaseWorld world = GetWorld();
		AIFormationDefinition formationDefinition;
		AIFormationComponent formationComponent = AIFormationComponent.Cast(this.FindComponent(AIFormationComponent));
		if (formationComponent)
			formationDefinition = formationComponent.GetFormation();
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		GetWorldTransform(spawnParams.Transform);
		vector pos = spawnParams.Transform[3];
		
		if (formationDefinition)
			pos = CoordToParent(formationDefinition.GetOffsetPosition(index));
		else
			pos = CoordToParent(Vector(index, 0, 0));
		
		if (snapToTerrain)
		{
			float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
			pos[1] = surfaceY;
		}
		
		
		//Snap to the nearest navmesh point
		AIPathfindingComponent pathFindindingComponent = AIPathfindingComponent.Cast(this.FindComponent(AIPathfindingComponent));
		
		if (!editMode)
		{
			NavmeshWorldComponent navmesh = pathFindindingComponent.GetNavmeshComponent();
			if (navmesh)
			{	 
				if (navmesh.IsTileRequested(pos))
				{
					return false;	
				}
				if (!navmesh.IsTileLoaded(pos))
				{
					navmesh.LoadTileIn(pos);
					return false;
				}
			}
		}
		
		if (pathFindindingComponent && pathFindindingComponent.GetClosestPositionOnNavmesh(pos, "10 10 10", pos))
		{
			float groundHeight = world.GetSurfaceY(pos[0], pos[2]);
			if (pos[1] < groundHeight)
				pos[1] = groundHeight;
			vector outWaterSurfacePoint;
			EWaterSurfaceType waterSurfaceType;
			vector transformWS[4];
			vector obbExtents;
			if (ChimeraWorldUtils.TryGetWaterSurface(GetWorld(), pos, outWaterSurfacePoint, waterSurfaceType, transformWS, obbExtents))
			{
				pos = outWaterSurfacePoint;
			}
		}
		
		spawnParams.Transform[3] = pos;
		
		IEntity member = GetGame().SpawnEntityPrefab(res, true, world, spawnParams);
		
		if(!SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning() && Replication.IsServer() && (!m_aUnitPrefabColorTeams.IsEmpty() || !m_aUnitPrefabOverrideIcons.IsEmpty()))
			GetGame().GetCallqueue().CallLater(WaitUntilWeSetDefaults, 1000, true, index, member);
		
		if (!member)
			return true;
		
		// Move in to vehicle 
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(member.FindComponent(SCR_EditableEntityComponent));
		
		if (editMode)
			m_aSceneGroupUnitInstances.Insert(member);
		
		// Even same null-check is above, in some situations, member can get deleted and it would result in VME
		if (!member)
			return true;
		
		AddAIEntityToGroup(member);
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(member.FindComponent(FactionAffiliationComponent));
		
		if (factionAffiliation)
			factionAffiliation.SetAffiliatedFactionByKey(m_faction);
	
		if (isLast) 
		{
			Event_OnInit.Invoke(this);
		};
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void WaitUntilWeSetDefaults(int index, IEntity entity)
	{			
		if(!entity)
			GetGame().GetCallqueue().Remove(WaitUntilWeSetDefaults);
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(SCR_ChimeraCharacter.Cast(entity));
		
		if (playerID == 0)
			return;
			
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (!groupsManagerComponent)
			return;
		
		SCR_AIGroup playersGroup = groupsManagerComponent.GetPlayerGroup(playerID);
		
		if (!playersGroup)
			return;
		
		int groupID = playersGroup.GetGroupID();
		CSI_AuthorityComponent authorityComponent = CSI_AuthorityComponent.GetInstance();
		
		if (groupID == -1 || !authorityComponent)
			return;
		
		GetGame().GetCallqueue().Remove(WaitUntilWeSetDefaults);

		string colorTeam;
		string overrideIcon;
			
		if((m_aUnitPrefabColorTeams.Count() - 1) >= index)
			colorTeam = m_aUnitPrefabColorTeams.Get(index);
			
		if((m_aUnitPrefabOverrideIcons.Count() - 1) >= index)
			overrideIcon = m_aUnitPrefabOverrideIcons.Get(index);
			
		if (!colorTeam.IsEmpty())
		{
			switch (colorTeam)
			{
				case "Red"    : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "CT", "R"); break;}; // CT = ColorTeam
				case "Blue"   : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "CT", "B"); break;}; // CT = ColorTeam
				case "Yellow" : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "CT", "Y"); break;}; // CT = ColorTeam
				case "Green"  : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "CT", "G"); break;}; // CT = ColorTeam
			};
		};
			
		if (!overrideIcon.IsEmpty())
		{
			switch (overrideIcon)
			{
				case "Team Lead"      : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "FTL"); break;}; // OI = OverrideIcon
				case "Medic"          : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "MED"); break;}; // OI = OverrideIcon
				case "Marksman"       : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "MRK"); break;}; // OI = OverrideIcon
				case "Machine Gunner" : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "MG");  break;}; // OI = OverrideIcon
				case "Anti-Tank"      : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "AT");  break;}; // OI = OverrideIcon
				case "Grenadier"      : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "GRN"); break;}; // OI = OverrideIcon
				case "Demolitionist"  : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "EXP"); break;}; // OI = OverrideIcon
				case "Engineer"       : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "ENG"); break;}; // OI = OverrideIcon
				case "Man"            : {authorityComponent.UpdateAuthorityPlayerMapValue(groupID, playerID, "OI", "MAN"); break;}; // OI = OverrideIcon
			};
		};
	}
};