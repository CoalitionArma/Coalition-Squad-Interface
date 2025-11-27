class CSI_PlayerDisconnectManagerClass : ScriptComponentClass {};

class CSI_PlayerDisconnectManager : ScriptComponent
{
	protected CSI_RplBroadcastManager m_RplBroadcastManager;
	protected CSI_PlayerDataManager m_PlayerDataManager;
	
	static int MAX_DATA_RETENTION_TIME = 900;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();
		m_PlayerDataManager = CSI_PlayerDataManager.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		m_iUpdate++;
		m_HUDManager.UpdateLocalAimingYaw();
		
		float currentTime = System.GetTickCount();
		if (currentTime - m_fLastSummaryTime >= m_iSummaryInterval * 1000)
		{
			PrintTelemetrySummary();
			m_fLastSummaryTime = currentTime;
		}
		
		UpdateAllLocalPlayerValues();
		m_HUDManager.UpdateLocalHUDValues();
	}
	
};