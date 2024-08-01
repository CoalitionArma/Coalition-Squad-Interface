modded class SCR_GroupsManagerComponent : SCR_BaseGameModeComponent
{	
	[RplProp()]
	protected ref array<array<RplId>> m_aGroupsIDsArray = new array<array<RplId>>;
	
	[RplProp()]
	protected ref array<array<string>> m_aGroupsPrefabColorsArray = new array<array<string>>;
	
	[RplProp()]
	protected ref array<array<string>> m_aGroupsPrefabOverridesArray = new array<array<string>>;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if(Replication.IsServer()) 
		{
			GetGame().GetCallqueue().CallLater(ReplicateGroupArrays, 850);
			GetGame().GetCallqueue().CallLater(ReplicateGroupArrays, 1850);
			GetGame().GetCallqueue().CallLater(ReplicateGroupArrays, 2850);
			GetGame().GetCallqueue().CallLater(ReplicateGroupArrays, 8850);
		};
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ReplicateGroupArrays()
	{
		m_aGroupsIDsArray.Insert(new array<RplId>);
		m_aGroupsPrefabColorsArray.Insert(new array<string>);
		m_aGroupsPrefabOverridesArray.Insert(new array<string>);
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsArray(array<RplId> value)
	{
		m_aGroupsIDsArray.Insert(value);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsPrefabColorsArray(array<string> value)
	{
		m_aGroupsPrefabColorsArray.Insert(value);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsPrefabOverridesArray(array<string> value)
	{
		m_aGroupsPrefabOverridesArray.Insert(value);
	}
	
	//------------------------------------------------------------------------------------------------
	array<array<RplId>> ReturnAllGroupIDsArray()
	{
		return m_aGroupsIDsArray;
	}
	
	array<array<string>> ReturnAllGroupPrefabColorsArray()
	{
		return m_aGroupsPrefabColorsArray;
	}
	
	array<array<string>> ReturnAllGroupPrefabOverridesArray()
	{
		return m_aGroupsPrefabOverridesArray;
	}
}

modded class SCR_AIGroup : ChimeraAIGroup
{
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Default slot color team, accepted values are: \n\n None \n Red \n Blue \n Yellow \n Green \n\nArray index should line up with the index of prefab in 'Unit Prefab Slots'", category: "Group Members")]
	ref array<string> m_aUnitPrefabColorTeams;
	
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Default slot override icon, accepted values are: \n\n None \n Team Lead \n Medic \n Marksman \n Machine Gunner \n Anti-Tank \n Grenadier \n Demolitionist \n Engineer \n Man \n\nArray index should line up with the index of prefab in 'Unit Prefab Slots'", category: "Group Members")]
	ref array<string> m_aUnitPrefabOverrideIcons;
	
	[RplProp()]
	ref array<RplId> m_aSpawnedPrefabIDs = {};
	
	protected override bool SpawnGroupMember(bool snapToTerrain, int index, ResourceName res, bool editMode, bool isLast)
	{	
		if (!GetGame().GetAIWorld().CanLimitedAIBeAdded())
		{
			if (isLast) 
			{
				Event_OnInit.Invoke(this);
				SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
				groupManager.UpdateGroupsArray(m_aSpawnedPrefabIDs);
				groupManager.UpdateGroupsPrefabColorsArray(m_aUnitPrefabColorTeams);
				groupManager.UpdateGroupsPrefabOverridesArray(m_aUnitPrefabOverrideIcons);
				Replication.BumpMe();
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
		m_aSpawnedPrefabIDs.InsertAt(Replication.FindId(member), index);
		
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
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
			groupManager.UpdateGroupsArray(m_aSpawnedPrefabIDs);
			groupManager.UpdateGroupsPrefabColorsArray(m_aUnitPrefabColorTeams);
			groupManager.UpdateGroupsPrefabOverridesArray(m_aUnitPrefabOverrideIcons);
			Replication.BumpMe();
		};
		return true;
	}
};