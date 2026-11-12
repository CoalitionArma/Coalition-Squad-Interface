//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-12
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Server world system: keeps a disconnected player's CSI data for DATA_RETENTION_TIME in case they reconnect,
//! 				then removes it on every machine through CSI_RplBroadcastManager. Every disconnect starts a new window,
//! 				timed with a WorldTimestamp like vanilla SCR_ReconnectComponent's reconnect window.
//! 
//------------------------------------------------------------------------------------------------
class CSI_PlayerDisconnectSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref map<int, WorldTimestamp> m_mDataExpiries = new map<int, WorldTimestamp>();	//!< disconnected player ID to when their CSI data is removed

	protected CSI_RplBroadcastManager m_RplBroadcastManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	protected SCR_BaseGameMode m_GameMode;	//!< listened to for disconnects

	static const int DATA_RETENTION_TIME = 390;	//!< seconds (6.5 minutes)
	static const int DATA_CHECK_INTERVAL = 60;	//!< seconds between checks
	protected float m_fTimeSinceLastCheck;		//!< seconds since the last retention check

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INIT METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Server)
			.AddPoint(WorldSystemPoint.FrameStart);
	}

	//------------------------------------------------------------------------------------------------
	//! The disconnect listener is added from OnUpdatePoint, which only runs between OnStarted and OnStopped.
	override protected void OnStopped()
	{
		if (m_GameMode)
			m_GameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);

		m_GameMode = null;
		super.OnStopped();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 UPDATE METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_RplBroadcastManager || !m_PlayerDataManager)
		{
			m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();
			m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
			return;
		}

		if (!m_GameMode)
		{
			m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (!m_GameMode)
				return;

			m_GameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
		}

		// Accumulate frame time like vanilla world systems; System.GetTickCount() is an int that overflows after ~24.8 days of uptime
		m_fTimeSinceLastCheck += args.GetTimeSliceSeconds();

		if (m_fTimeSinceLastCheck >= DATA_CHECK_INTERVAL)
		{
			DataRetentionCheck();
			m_fTimeSinceLastCheck = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Starts the player's retention window. A player who reconnected and left again gets a new one.
	//! \param[in] playerID
	//! \param[in] cause
	//! \param[in] timeout
	protected void OnPlayerDisconnected(int playerID, KickCauseCode cause, int timeout)
	{
		m_mDataExpiries.Set(playerID, GetGame().GetWorld().GetTimestamp().PlusSeconds(DATA_RETENTION_TIME));
	}

	//------------------------------------------------------------------------------------------------
	//! Removes the CSI data of players whose retention window has passed. Players who are connected again lose their window.
	protected void DataRetentionCheck()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		WorldTimestamp now = GetGame().GetWorld().GetTimestamp();

		// Safety net: data of a player who left before the disconnect listener was added still gets a window
		foreach (int playerID, CSI_PlayerData playerData : m_PlayerDataManager.GetPlayerDataMap())
		{
			if (!m_mDataExpiries.Contains(playerID) && !playerManager.IsPlayerConnected(playerID))
				m_mDataExpiries.Set(playerID, now.PlusSeconds(DATA_RETENTION_TIME));
		}

		// Collect first and remove after the loop, as the map can't change while it's iterated
		array<int> reconnectedPlayerIDs = {};
		array<int> expiredPlayerIDs = {};
		foreach (int windowPlayerID, WorldTimestamp expiry : m_mDataExpiries)
		{
			if (playerManager.IsPlayerConnected(windowPlayerID))
				reconnectedPlayerIDs.Insert(windowPlayerID);
			else if (expiry.Less(now))
				expiredPlayerIDs.Insert(windowPlayerID);
		}

		foreach (int reconnectedPlayerID : reconnectedPlayerIDs)
		{
			m_mDataExpiries.Remove(reconnectedPlayerID);
		}

		foreach (int expiredPlayerID : expiredPlayerIDs)
		{
			m_mDataExpiries.Remove(expiredPlayerID);

			if (m_PlayerDataManager.GetPlayerData(expiredPlayerID))
				m_RplBroadcastManager.RemovePlayerData(expiredPlayerID);
		}
	}
};