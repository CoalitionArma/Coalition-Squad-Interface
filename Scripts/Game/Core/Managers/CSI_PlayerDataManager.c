class CSI_PlayerDataManagerClass : ScriptComponentClass {};

//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-11
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Game mode component holding every player's CSI_PlayerData. The server and each client keep their own
//! 				copy: CSI_RplBroadcastManager keeps them in sync, RplSave/RplLoad cover join-in-progress.
//! 				Public API: CRF calls GetInstance and GetPlayerData.
//! 
//------------------------------------------------------------------------------------------------
class CSI_PlayerDataManager : ScriptComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ref map<int, ref CSI_PlayerData> m_mPlayerDataMap = new map<int, ref CSI_PlayerData>;	//!< keyed by player ID

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
	//! \return null until the player's client has registered (needs a character and a group)
	CSI_PlayerData GetPlayerData(int playerID)
	{
		return m_mPlayerDataMap.Get(playerID);
	}

	//------------------------------------------------------------------------------------------------
	map<int, ref CSI_PlayerData> GetPlayerDataMap()
	{
		return m_mPlayerDataMap;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 REGISTER/DE-REGISTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! Creates the player's data if it doesn't exist yet.
	//! \param[in] playerID
	void RegisterPlayerData(int playerID)
	{
		if (!GetPlayerData(playerID))
			CreatePlayerData(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes the player's data, e.g. after they've been disconnected for a while (CSI_PlayerDisconnectSystem).
	//! \param[in] playerID
	void RemovePlayerData(int playerID)
	{
		if (GetPlayerData(playerID))
			m_mPlayerDataMap.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return the new data
	protected CSI_PlayerData CreatePlayerData(int playerID)
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
	void UpdatePlayerDisplayIcon(int playerID, CSI_EIcon icon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetDisplayIcon(icon);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetColorTeam(colorTeam);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] overrideIcon AUTO removes the override
	void UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetOverrideIcon(overrideIcon);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetRank(rank);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetIsTeamLeader(isTL);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
			playerData.SetIsSquadLeader(isSL);
	}

	//------------------------------------------------------------------------------------------------
	//! Server only: resets color team, override icon and team leader flag for everyone through the broadcast manager.
	//! \param[in] playerID
	void ClearGroupSpecificData(int playerID)
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
	//! Sends the whole map to a joining client.
	//! \param[in] writer
	//! \return
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

			int playerID = playerData.GetPlayerID();

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
