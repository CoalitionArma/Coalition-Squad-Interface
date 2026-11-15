class CSI_PlayerDataManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-14
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Every player's CSI_PlayerData, kept in sync by CSI_RplBroadcastManager and sent to joining clients.
//! 
//------------------------------------------------------------------------------------------------
class CSI_PlayerDataManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref map<PlayerId, ref CSI_PlayerData> m_mPlayerDataMap = new map<PlayerId, ref CSI_PlayerData>;	//!< keyed by player ID

	protected CSI_RplBroadcastManager m_RplBroadcastManager;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 MANAGER INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return null until the client registers it (needs a character and a group)
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

		if (playerData)
		{
			m_RplBroadcastManager.UpdatePlayerColorTeam(playerID, CSI_EColorTeam.NONE);
			m_RplBroadcastManager.UpdatePlayerOverrideIcon(playerID, CSI_EOverrideIcon.AUTO);
			m_RplBroadcastManager.UpdatePlayerTeamLeader(playerID, false);
		}
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
