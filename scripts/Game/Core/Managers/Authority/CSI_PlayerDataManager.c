class CSI_PlayerDataManagerClass : ScriptComponentClass {};

class CSI_PlayerDataManager : ScriptComponent
{	
	// Primary Data Map
	protected ref map<int, ref CSI_PlayerData> m_mPlayerDataMap = new map<int, ref CSI_PlayerData>;

	// Replication Maps
	protected ref map<int, int> m_PlayersColorTeams 		= new map<int, int>();
	protected ref map<int, int> m_PlayersOverrideIcon 		= new map<int, int>();
	protected ref map<int, int> m_PlayersDisplayIcon 		= new map<int, int>();
	protected ref map<int, int> m_PlayersRank 				= new map<int, int>();
	protected ref map<int, bool> m_PlayersIsTeamLeader 		= new map<int, bool>();
	protected ref map<int, bool> m_PlayersIsSquadLeader 	= new map<int, bool>();
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the player's squad-leader status, display icon and character rank in the players data, this is updated only on the client and cannot be set by other players.
	 * @param playerID: ID of the player to update.
	 * @param isSL: True when the player should be flagged as squad leader; false otherwise.
	 * @param icon: CSI_EIcon value to apply as the player's current icon.
	 * @param rank: SCR_ECharacterRank value to assign as the player's rank.
	 */
	void UpdatePlayerData(int playerID, bool isSL, CSI_EIcon icon, SCR_ECharacterRank rank)
	{
		if (playerID <= 0)
			return;

		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (!playerData)
			playerData = CreatePlayerData(playerID);
		
		playerData.SetDisplayIcon(icon);
		playerData.SetRank(rank);
		playerData.SetIsSquadLeader(isSL);
		
		DataUpdate(playerData);
	}

	//------------------------------------------------------------------------------------------------
	/**
	 * Updates player data to clear any group-specific values
	 * @param playerID: ID of player to clear
	 */
	void ClearGroupSpecificData(int playerID)
	{
		if (playerID <= 0)
			return;

		CSI_PlayerData playerData = GetPlayerData(playerID);

		if (playerData)
		{
			playerData.SetColorTeam(CSI_EColorTeam.NONE);
			playerData.SetOverrideIcon(CSI_EOverrideIcon.AUTO);
			playerData.SetIsTeamLeader(false);
		}

		DataUpdate(playerData);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the specified player's CSI_EColorTeam value in the players data
	 * @param playerID: ID of the player whose color team will be updated.
	 * @param colorTeam: New color team value to assign.
	 */
	void UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		if (playerID <= 0)
			return;

		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetColorTeam(colorTeam);
		
		DataUpdate(playerData);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Update the temporary override icon for a player.
	 *
	 * Sets the specified player's CSI_EOverrideIcon value in the players data
	 * @param playerID: ID of the player whose override icon is being changed.
	 * @param overrideIcon: CSI_EOverrideIcon value indicating which override icon to set (use the enum's "AUTO" value to remove the override).
	 */
	void UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		if (playerID <= 0)
			return;

		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetOverrideIcon(overrideIcon);
		
		DataUpdate(playerData);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the specified player's team leader value in the players data
	 * @param playerID: ID of the player whose leader status is being changed.
	 * @param isTL: True to mark the player as team leader; false to revoke leader status.
	 */
	void UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		if (playerID <= 0)
			return;

		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetIsTeamLeader(isTL);
		
		DataUpdate(playerData);
	}

	//------------------------------------------------------------------------------------------------
	/**
	 * Retrieves player data instance for a given player ID
	 * @param playerID: The ID of player to get data for
	 * @return CSI_PlayerData instance
	 */
	CSI_PlayerData GetPlayerData(int playerID)
	{
		return m_mPlayerDataMap.Get(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Creates a new player data instance for the given player ID.
	 * @param playerID: The ID of player to create data for
	 * @return CSI_PlayerData instance
	 */
	protected CSI_PlayerData CreatePlayerData(int playerID)
	{
		CSI_PlayerData playerData = new CSI_PlayerData;
		playerData.SetPlayerID(playerID);
		m_mPlayerDataMap.Set(playerID, playerData);
		return playerData;
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates player data arrays that then update the information on all clients.
	 */
	protected void DataUpdate(CSI_PlayerData playerData)
	{
		RpcDo_PlayerDataUpdate(playerData);
		Rpc(RpcDo_PlayerDataUpdate, playerData);
	}

	//------------------------------------------------------------------------------------------------
	/**
	 * Updates and synchronizes the manager's authoritative player data with the current local data.
	 */
	 [RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlayerDataUpdate(CSI_PlayerData playerData)
	{
		int playerID = playerData.GetPlayerID();
		CSI_PlayerData oldPlayerData = m_mPlayerDataMap.Get(playerID);

		if(!oldPlayerData)
			m_mPlayerDataMap.Set(playerID, playerData);
		else
			oldPlayerData.DataUpdate(playerID, playerData);
	}

	//------------------------------------------------------------------------------------------------
	// Remove player data from list global list replicated
	void UnRegisterPlayerData(int playerID)
	{
		RpcDo_UnRegisterPlayerData(playerID);
		Rpc(RpcDo_UnRegisterPlayerData, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_UnRegisterPlayerData(int playerID)
	{
		if (!m_mPlayerDataMap.Contains(playerID))
			return;

		m_mPlayerDataMap.Remove(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		// Save maps
		CSI_ReplicationHelper.WriteMapIntInt(writer, m_PlayersColorTeams);
		CSI_ReplicationHelper.WriteMapIntInt(writer, m_PlayersOverrideIcon);
		CSI_ReplicationHelper.WriteMapIntInt(writer, m_PlayersDisplayIcon);
		CSI_ReplicationHelper.WriteMapIntInt(writer, m_PlayersRank);
		
		CSI_ReplicationHelper.WriteMapIntBool(writer, m_PlayersIsTeamLeader);
		CSI_ReplicationHelper.WriteMapIntBool(writer, m_PlayersIsSquadLeader);

		// Save playerData
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
		// Load maps
		CSI_ReplicationHelper.ReadMapIntInt(reader, m_PlayersColorTeams);
		CSI_ReplicationHelper.ReadMapIntInt(reader, m_PlayersOverrideIcon);
		CSI_ReplicationHelper.ReadMapIntInt(reader, m_PlayersDisplayIcon);
		CSI_ReplicationHelper.ReadMapIntInt(reader, m_PlayersRank);
		
		CSI_ReplicationHelper.ReadMapIntBool(reader, m_PlayersIsTeamLeader);
		CSI_ReplicationHelper.ReadMapIntBool(reader, m_PlayersIsSquadLeader);

		// Load playerData
		int playablesCount;
		reader.ReadInt(playablesCount);
		for (int i = 0; i < playablesCount; i++)
		{
			CSI_PlayerData playerData = new CSI_PlayerData();
			playerData.Load(reader);
			RpcDo_PlayerDataUpdate(playerData);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns the instance of the PlayerDataManager
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
