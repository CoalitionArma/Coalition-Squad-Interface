class CSI_PlayerDisconnectSystem : GameSystem
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref map<int, int> m_mPlayerTimeMap = new map<int, int>;
	
	protected CSI_RplBroadcastSystem m_RplBroadcastSystem;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	
	static int DATA_RETENTION_TIME = 900;
	static int DATA_CHECK_INTERVAL = 60;
	protected int m_iLastDataRetentionCheckTick;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SYSTEM INITILIZATION
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false)
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInit()
	{
		super.OnInit();

		m_RplBroadcastSystem = CSI_RplBroadcastSystem.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ONFRAME METHODS
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{		
		float currentTime = System.GetTickCount();
		int currentTimeDifference = currentTime - m_iLastDataRetentionCheckTick;
		
		if (currentTimeDifference >= DATA_CHECK_INTERVAL * 1000)
		{
			DataRetentionCheck(currentTimeDifference);
			m_iLastDataRetentionCheckTick = currentTime;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DataRetentionCheck(int currentTimeDifference)
	{
		array<int> outPlayers = new array<int>;
		GetGame().GetPlayerManager().GetPlayers(outPlayers);
		
		foreach (int playerID, CSI_PlayerData playerData : m_PlayerDataManager.GetPlayerDataMap()) 
		{
			if (!outPlayers.Contains(playerID)) 
			{
				int currentPlayerDisconnectedTime = m_mPlayerTimeMap.Get(playerID);
				currentPlayerDisconnectedTime = currentPlayerDisconnectedTime + currentTimeDifference;
				
				m_mPlayerTimeMap.Set(playerID, currentPlayerDisconnectedTime);
				
				Print(currentTimeDifference);
				Print(currentPlayerDisconnectedTime >= (DATA_RETENTION_TIME * 1000));
				
				if (currentPlayerDisconnectedTime >= (DATA_RETENTION_TIME * 1000))
				{
					m_RplBroadcastSystem.RemovePlayerData(playerID);
					m_mPlayerTimeMap.Remove(playerID);
				};
			} else if (m_mPlayerTimeMap.Get(playerID))
				m_mPlayerTimeMap.Remove(playerID);
		}
	}
};