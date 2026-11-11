//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Server world system: keeps a disconnected player's CSI data for DATA_RETENTION_TIME in case they reconnect,
//! 				then removes it on every machine through CSI_RplBroadcastManager.
//! 
//------------------------------------------------------------------------------------------------
class CSI_PlayerDisconnectSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref map<int, float> m_mPlayerTimeMap = new map<int, float>;	//!< player ID to ms spent disconnected

	protected CSI_RplBroadcastManager m_RplBroadcastManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;

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

		// Accumulate frame time like vanilla world systems; System.GetTickCount() is an int that overflows after ~24.8 days of uptime
		m_fTimeSinceLastCheck += args.GetTimeSliceSeconds();

		if (m_fTimeSinceLastCheck >= DATA_CHECK_INTERVAL)
		{
			DataRetentionCheck(m_fTimeSinceLastCheck * 1000);
			m_fTimeSinceLastCheck = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the elapsed time to every disconnected player and removes those past DATA_RETENTION_TIME.
	//! Players who are connected again lose their timer.
	//! \param[in] currentTimeDifference ms since the last check
	protected void DataRetentionCheck(float currentTimeDifference)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		// RemovePlayerData removes from the map we're iterating, so collect the IDs first and remove after the loop
		array<int> playersToRemove = {};

		foreach (int playerID, CSI_PlayerData playerData : m_PlayerDataManager.GetPlayerDataMap())
		{
			if (!playerManager.IsPlayerConnected(playerID))
			{
				float currentPlayerDisconnectedTime = m_mPlayerTimeMap.Get(playerID);
				currentPlayerDisconnectedTime = currentPlayerDisconnectedTime + currentTimeDifference;

				m_mPlayerTimeMap.Set(playerID, currentPlayerDisconnectedTime);

				if (currentPlayerDisconnectedTime >= (DATA_RETENTION_TIME * 1000))
					playersToRemove.Insert(playerID);
			} else if (m_mPlayerTimeMap.Get(playerID))
				m_mPlayerTimeMap.Remove(playerID);
		}

		foreach (int playerID : playersToRemove)
		{
			m_RplBroadcastManager.RemovePlayerData(playerID);
			m_mPlayerTimeMap.Remove(playerID);
		}
	}
};