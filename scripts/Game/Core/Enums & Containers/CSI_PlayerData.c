class CSI_PlayerData
{
	protected CSI_EColorTeam m_iColorTeam = CSI_EColorTeam.NONE;
	protected CSI_EOverrideIcon m_iOverrideIcon = CSI_EOverrideIcon.AUTO;
	protected CSI_EIcon m_iDisplayIcon = CSI_EIcon.MAN;
	protected SCR_ECharacterRank m_iRank = SCR_ECharacterRank.PRIVATE;
	protected bool m_bIsTeamLeader;
	protected bool m_bIsSquadLeader;
	
	// Invoker for data updates
	protected ref ScriptInvoker m_OnDataUpdate;
	
	protected int m_iPlayerValue;
	
	//------------------------------------------------------------------------------------------------
	/**
	 * Replaces or sets the internal CSI_PlayerData record for the player.
	 * If newData is non-null, the player's data is updated with the provided instance.
	 *
	 * @param playerID: ID of the player whose data should be updated.
	 * @param newData: Pointer/reference to the new CSI_PlayerData to apply.
	 */
	void DataUpdate(int playerID, CSI_PlayerData newData = null)
	{	
		if(newData)	
		{
			SetColorTeam(newData.GetColorTeam());
			SetOverrideIcon(newData.GetOverrideIcon());
			SetDisplayIcon(newData.GetDisplayIcon());
			SetRank(newData.GetRank());
			SetIsTeamLeader(newData.GetIsTeamLeader());
			SetIsSquadLeader(newData.GetIsSquadLeader());
		};
		
		m_iPlayerValue = UpdatePlayerValue(playerID);
		
		if (m_OnDataUpdate)
			m_OnDataUpdate.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	/**
	 * Updates the value associated with a player identified by their ID
	 * @param playerID: The unique identifier of the player
	 * @return Updated value for the specified player
	 */
	protected int UpdatePlayerValue(int playerID)
	{
		int value = 0;

		// Sort player by their color so we can group color teams together (a lil bit racist).
		switch (m_iColorTeam) 
		{
			case CSI_EColorTeam.RED    : value = -3; break;
			case CSI_EColorTeam.BLUE   : value = -5; break;
			case CSI_EColorTeam.YELLOW : value = -7; break;
			case CSI_EColorTeam.GREEN  : value = -9; break;
			default : {value = 2;  break;};
		};

		switch (true) 
		{
			case (m_bIsSquadLeader) : value = -1; break;
			case (m_bIsTeamLeader && m_iColorTeam == CSI_EColorTeam.NONE) : value--; break;
			case (m_bIsTeamLeader && m_iColorTeam != CSI_EColorTeam.NONE) : value++; break;
		};

		return value;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDataUpdate()
	{
		if (!m_OnDataUpdate)
			m_OnDataUpdate = new ScriptInvoker();

		return m_OnDataUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	// SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetColorTeam(CSI_EColorTeam colorTeam)
	{
		m_iColorTeam = colorTeam;
	}

	//------------------------------------------------------------------------------------------------
	void SetOverrideIcon(CSI_EOverrideIcon overrideIcon)
	{
		m_iOverrideIcon = overrideIcon;
	}

	//------------------------------------------------------------------------------------------------
	void SetDisplayIcon(CSI_EIcon displayIcon)
	{
		m_iDisplayIcon = displayIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRank(SCR_ECharacterRank rank)
	{
		m_iRank = rank;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsTeamLeader(bool isTL)
	{
		m_bIsTeamLeader = isTL;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsSquadLeader(bool isSL)
	{
		m_bIsSquadLeader = isSL;
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerValue()
	{
		return m_iPlayerValue;
	}
	
	//------------------------------------------------------------------------------------------------
	CSI_EColorTeam GetColorTeam()
	{
		return m_iColorTeam;
	}
	
	//------------------------------------------------------------------------------------------------
	CSI_EOverrideIcon GetOverrideIcon()
	{
		return m_iOverrideIcon;
	}

	//------------------------------------------------------------------------------------------------
	CSI_EIcon GetDisplayIcon()
	{
		return m_iDisplayIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetRank()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTeamLeader()
	{
		return m_bIsTeamLeader;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsSquadLeader()
	{
		return m_bIsSquadLeader;
	}
	
	//------------------------------------------------------------------------------------------------
	// REPLICATION STUFF
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	bool RplSave(ScriptBitWriter writer)
	{
		writer.Write(m_iColorTeam, 6);
		writer.Write(m_iOverrideIcon, 6);
		writer.Write(m_iDisplayIcon, 6);
		writer.Write(m_iRank, 6);
		writer.Write(m_bIsTeamLeader, 1);
		writer.Write(m_bIsSquadLeader, 1);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RplLoad(ScriptBitReader reader)
	{
		reader.Read(m_iColorTeam, 6);
		reader.Read(m_iOverrideIcon, 6);
		reader.Read(m_iDisplayIcon, 6);
		reader.Read(m_iRank, 6);
		reader.Read(m_bIsTeamLeader, 1);
		reader.Read(m_bIsSquadLeader, 1);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(CSI_PlayerData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeBytes(instance.m_iColorTeam, 4);
		snapshot.SerializeBytes(instance.m_iOverrideIcon, 4);
		snapshot.SerializeBytes(instance.m_iDisplayIcon, 4);
		snapshot.SerializeBytes(instance.m_iRank, 4);
		snapshot.SerializeBytes(instance.m_bIsTeamLeader, 4);
		snapshot.SerializeBytes(instance.m_bIsSquadLeader, 4);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CSI_PlayerData instance)
	{
		snapshot.SerializeBytes(instance.m_iColorTeam, 4);
		snapshot.SerializeBytes(instance.m_iOverrideIcon, 4);
		snapshot.SerializeBytes(instance.m_iDisplayIcon, 4);
		snapshot.SerializeBytes(instance.m_iRank, 4);
		snapshot.SerializeBytes(instance.m_bIsTeamLeader, 4);
		snapshot.SerializeBytes(instance.m_bIsSquadLeader, 4);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeBool(packet);
		snapshot.EncodeBool(packet);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeBool(packet);
		snapshot.DecodeBool(packet);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(CSI_PlayerData instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.Compare(instance.m_iColorTeam, 4)
			&& snapshot.Compare(instance.m_iOverrideIcon, 4)
			&& snapshot.Compare(instance.m_iDisplayIcon, 4)
			&& snapshot.Compare(instance.m_iRank, 4)
			&& snapshot.Compare(instance.m_bIsTeamLeader, 4)
			&& snapshot.Compare(instance.m_bIsSquadLeader, 4);
	}
}