class CSI_PlayerDataManagerClass : SCR_BaseGameModeComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-21
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Every player's CSI_PlayerData, kept in sync by CSI_RplBroadcastManager and sent to joining clients.
//! 				The server registers a player's data when they connect, clears their group-specific data when
//! 				they join a different group, and keeps a disconnected player's data for DATA_RETENTION_TIME in
//! 				case they reconnect.
//!
//------------------------------------------------------------------------------------------------
class CSI_PlayerDataManager : SCR_BaseGameModeComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref map<PlayerId, ref CSI_PlayerData> m_mPlayerDataMap = new map<PlayerId, ref CSI_PlayerData>;	//!< keyed by player ID
	protected ref map<PlayerId, int> m_mLastGroupIDs = new map<PlayerId, int>;	//!< server only: player ID to the last group they joined
	protected ref map<PlayerId, WorldTimestamp> m_mDataExpiries = new map<PlayerId, WorldTimestamp>;	//!< server only: disconnected player ID to when their data expires
	protected bool m_bDataExpiryCheckQueued;	//!< server only: CheckDataExpiry is in the callqueue

	protected CSI_RplBroadcastManager m_RplBroadcastManager;

	static const int DATA_RETENTION_TIME = 390;	//!< seconds (6.5 minutes) a disconnected player's data is kept
	static const int DATA_CHECK_INTERVAL = 60;	//!< seconds between expiry checks, while a player's data is waiting

//=============================================================================================================================================================================================================================================================================================================================================================
//	 MANAGER INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnGroupPlayerAdded);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		SCR_AIGroup.GetOnPlayerAdded().Remove(OnGroupPlayerAdded);

		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(CheckDataExpiry);
	}

	//------------------------------------------------------------------------------------------------
	//! Registers the player's data on every machine; a reconnecting player keeps theirs.
	//! \param[in] playerId
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);

		// Back in time: their data stays
		if (m_mDataExpiries.Contains(playerId))
		{
			m_mDataExpiries.Remove(playerId);
			UpdateDataExpiryCheck();
		}

		if (m_RplBroadcastManager && !GetPlayerData(playerId))
			m_RplBroadcastManager.RegisterPlayerData(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: starts the retention window of the player's data, replacing any earlier one.
	//! \param[in] playerId
	//! \param[in] cause
	//! \param[in] timeout
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		if (!Replication.IsServer())
			return;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		m_mDataExpiries.Set(playerId, world.GetTimestamp().PlusSeconds(DATA_RETENTION_TIME));
		UpdateDataExpiryCheck();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return null for a player who hasn't connected, or whose data expired after a disconnect
	CSI_PlayerData GetPlayerData(PlayerId playerID)
	{
		return m_mPlayerDataMap.Get(playerID);
	}

	//------------------------------------------------------------------------------------------------
	map<PlayerId, ref CSI_PlayerData> GetPlayerDataMap()
	{
		return m_mPlayerDataMap;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 REGISTER/DE-REGISTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	void RegisterPlayerData(PlayerId playerID)
	{
		if (!GetPlayerData(playerID))
			CreatePlayerData(playerID);
	}

	//------------------------------------------------------------------------------------------------
	void RemovePlayerData(PlayerId playerID)
	{
		if (GetPlayerData(playerID))
			m_mPlayerDataMap.Remove(playerID);

		m_mLastGroupIDs.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	protected CSI_PlayerData CreatePlayerData(PlayerId playerID)
	{
		CSI_PlayerData playerData = new CSI_PlayerData;
		playerData.SetPlayerID(playerID);
		m_mPlayerDataMap.Set(playerID, playerData);
		return playerData;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerDisplayIcon(PlayerId playerID, CSI_EIcon icon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetDisplayIcon(icon);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerColorTeam(PlayerId playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetColorTeam(colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] overrideIcon AUTO removes the override
	void UpdatePlayerOverrideIcon(PlayerId playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetOverrideIcon(overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerRank(PlayerId playerID, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetRank(rank);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerTeamLeader(PlayerId playerID, bool isTL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetIsTeamLeader(isTL);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerSquadLeader(PlayerId playerID, bool isSL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetIsSquadLeader(isSL);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: resets the player's color team, override icon and team leader flag.
	//! \param[in] playerID
	void ClearGroupSpecificData(PlayerId playerID)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (!playerData || !m_RplBroadcastManager)
			return;

		// Only what is set is broadcast, which for most players is nothing
		if (playerData.GetColorTeam() != CSI_EColorTeam.NONE)
			m_RplBroadcastManager.UpdatePlayerColorTeam(playerID, CSI_EColorTeam.NONE);

		if (playerData.GetOverrideIcon() != CSI_EOverrideIcon.AUTO)
			m_RplBroadcastManager.UpdatePlayerOverrideIcon(playerID, CSI_EOverrideIcon.AUTO);

		if (playerData.GetIsTeamLeader())
			m_RplBroadcastManager.UpdatePlayerTeamLeader(playerID, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: clears the player's group-specific data when they join a different group than their last one.
	//! \param[in] group
	//! \param[in] playerID
	protected void OnGroupPlayerAdded(SCR_AIGroup group, PlayerId playerID)
	{
		if (!Replication.IsServer())
			return;

		// Rejoining the same group (e.g. after a reconnect) keeps the data
		int lastGroupID;
		if (m_mLastGroupIDs.Find(playerID, lastGroupID) && lastGroupID != group.GetGroupID())
			ClearGroupSpecificData(playerID);

		m_mLastGroupIDs.Set(playerID, group.GetGroupID());
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 DISCONNECTED PLAYERS (SERVER)
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Runs CheckDataExpiry every DATA_CHECK_INTERVAL while a disconnected player's data is waiting.
	protected void UpdateDataExpiryCheck()
	{
		ScriptCallQueue callqueue = GetGame().GetCallqueue();
		if (m_mDataExpiries.IsEmpty())
		{
			if (m_bDataExpiryCheckQueued)
				callqueue.Remove(CheckDataExpiry);

			m_bDataExpiryCheckQueued = false;
			return;
		}

		// Tracked here: a repeating call that runs late reads a negative GetRemainingTime while it runs
		if (m_bDataExpiryCheckQueued)
			return;

		callqueue.CallLater(CheckDataExpiry, DATA_CHECK_INTERVAL * 1000, true);
		m_bDataExpiryCheckQueued = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes expired players' CSI data on every machine, and the windows of players who are back.
	protected void CheckDataExpiry()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		if (!m_RplBroadcastManager)
			m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();

		if (!m_RplBroadcastManager)
			return;

		WorldTimestamp now = GetGame().GetWorld().GetTimestamp();

		// Collect first: the map can't change while it's iterated
		array<int> reconnectedPlayerIDs = {};
		array<int> expiredPlayerIDs = {};
		foreach (PlayerId playerID, WorldTimestamp expiry : m_mDataExpiries)
		{
			if (playerManager.IsPlayerConnected(playerID))
				reconnectedPlayerIDs.Insert(playerID);
			else if (expiry.Less(now))
				expiredPlayerIDs.Insert(playerID);
		}

		foreach (int reconnectedPlayerID : reconnectedPlayerIDs)
		{
			m_mDataExpiries.Remove(reconnectedPlayerID);
		}

		foreach (int expiredPlayerID : expiredPlayerIDs)
		{
			m_mDataExpiries.Remove(expiredPlayerID);

			if (GetPlayerData(expiredPlayerID))
				m_RplBroadcastManager.RemovePlayerData(expiredPlayerID);
		}

		UpdateDataExpiryCheck();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 REPLICATION METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		// Count first so RplLoad knows how many entries follow
		int playablesCount = m_mPlayerDataMap.Count();
		writer.WriteInt(playablesCount);
		foreach (int PlayerID, CSI_PlayerData playerData : m_mPlayerDataMap)
		{
			playerData.Save(writer);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RplLoad(ScriptBitReader reader)
	{
		int playablesCount;
		reader.ReadInt(playablesCount);
		for (int i = 0; i < playablesCount; i++)
		{
			CSI_PlayerData playerData = new CSI_PlayerData();
			playerData.Load(reader);

			PlayerId playerID = playerData.GetPlayerID();

			// Read every entry to keep the stream aligned, but only keep valid IDs
			if (playerID > 0)
				m_mPlayerDataMap.Set(playerID, playerData);
		}

		return true;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	protected static CSI_PlayerDataManager m_sInstance;
	static CSI_PlayerDataManager GetInstance()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	void CSI_PlayerDataManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
}
