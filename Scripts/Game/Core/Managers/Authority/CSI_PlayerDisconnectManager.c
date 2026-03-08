class CSI_PlayerDisconnectManagerClass : ScriptComponentClass {};

class CSI_PlayerDisconnectManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref map<int, int> m_mPlayerTimeMap = new map<int, int>;
	
	protected CSI_RplBroadcastManager m_RplBroadcastManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	
	static int DATA_RETENTION_TIME = 900;
	static int DATA_CHECK_INTERVAL = 60;
	protected int m_iLastDataRetentionCheckTick;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SYSTEM INITILIZATION
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (RplSession.Mode() == RplMode.Client) 
			return;

		m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
		
		SetEventMask(owner, EntityEvent.FRAME);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ONFRAME METHODS
//=============================================================================================================================================================================================================================================================================================================================================================
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		int currentTime = System.GetTickCount();
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
				
				if (currentPlayerDisconnectedTime >= DATA_RETENTION_TIME)
				{
					m_RplBroadcastManager.RemovePlayerData(playerID);
					m_mPlayerTimeMap.Remove(playerID);
				};
			} else if (m_mPlayerTimeMap.Get(playerID))
				m_mPlayerTimeMap.Remove(playerID);
		}
	}
};