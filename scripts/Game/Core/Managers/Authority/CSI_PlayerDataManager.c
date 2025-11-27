class CSI_PlayerDataManagerClass : ScriptComponentClass {};

class CSI_PlayerDataManager : ScriptComponent
{	
	// Primary Data Map
	protected ref map<int, ref CSI_PlayerData> m_mPlayerDataMap = new map<int, ref CSI_PlayerData>;

	protected CSI_RplBroadcastManager m_RplBroadcastManager;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_RplBroadcastManager = CSI_RplBroadcastManager.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Create the players data class.
	 * @param playerID: ID of the player to register.
	 */
	void RegisterPlayerData(int playerID)
	{
		if (!GetPlayerData(playerID))
			CreatePlayerData(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Removes a players data from the data map, primarily used to clean out disconnects
	 * @param playerID: ID of the player to remove.
	 */
	void RemovePlayerData(int playerID)
	{
		if (GetPlayerData(playerID))
			m_mPlayerDataMap.Remove(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Updates player data to clear any group-specific values
	 * @param playerID: ID of player to clear
	 */
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
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the player's display icon.
	 * @param playerID: ID of the player to update.
	 * @param icon: CSI_EIcon value to apply as the player's current icon.
	 */
	void UpdatePlayerDisplayIcon(int playerID, CSI_EIcon icon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetDisplayIcon(icon);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the specified player's CSI_EColorTeam value in the players data
	 * @param playerID: ID of the player whose color team will be updated.
	 * @param colorTeam: New color team value to assign.
	 */
	void UpdatePlayerColorTeam(int playerID, CSI_EColorTeam colorTeam)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetColorTeam(colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Update the temporary override icon for a player.
	 * Sets the specified player's CSI_EOverrideIcon value in the players data
	 * @param playerID: ID of the player whose override icon is being changed.
	 * @param overrideIcon: CSI_EOverrideIcon value indicating which override icon to set (use the enum's "AUTO" value to remove the override).
	 */
	void UpdatePlayerOverrideIcon(int playerID, CSI_EOverrideIcon overrideIcon)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetOverrideIcon(overrideIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the specified player's rank value in their data
	 * @param playerID: ID of the player whose color team will be updated.
	 * @param rank: New rank value to assign.
	 */
	void UpdatePlayerRank(int playerID, SCR_ECharacterRank rank)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetRank(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the specified player's team leader value in the players data
	 * @param playerID: ID of the player whose leader status is being changed.
	 * @param isTL: True to mark the player as team leader; false to revoke leader status.
	 */
	void UpdatePlayerTeamLeader(int playerID, bool isTL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetIsTeamLeader(isTL);
	}
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Sets the specified player's squad leader value in the players data
	 * @param playerID: ID of the player whose leader status is being changed.
	 * @param isSL: True to mark the player as squad leader; false to revoke leader status.
	 */
	void UpdatePlayerSquadLeader(int playerID, bool isSL)
	{
		CSI_PlayerData playerData = GetPlayerData(playerID);
		
		if (playerData)
			playerData.SetIsSquadLeader(isSL);
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
	override protected bool RplSave(ScriptBitWriter writer)
	{
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
		// Load playerData
		int playablesCount;
		reader.ReadInt(playablesCount);
		for (int i = 0; i < playablesCount; i++)
		{
			CSI_PlayerData playerData = new CSI_PlayerData();
			playerData.Load(reader);
			
			int playerID = playerData.GetPlayerID();
			
			if (playerID > 0)
				m_mPlayerDataMap.Set(playerID, playerData);
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
