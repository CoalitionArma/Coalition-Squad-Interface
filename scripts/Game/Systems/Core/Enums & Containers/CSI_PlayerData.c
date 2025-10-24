class CSI_PlayerData
{
	protected int m_iPlayerID;

	protected CSI_EColorTeam m_iColorTeam;
	
	protected CSI_EOverrideIcon m_iOverrideIcon;
	protected CSI_EIcon m_iDisplayIcon;

	protected SCR_ECharacterRank m_iRank;
	
	protected bool m_bIsTeamLeader;
	
	// Invoker for data updates
	protected ref ScriptInvoker m_OnDataUpdate;
	
	//------------------------------------------------------------------------------------------------
	// SCRIPT INVOKERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void InvokeDataUpdate()
	{
		if (m_OnDataUpdate)
			m_OnDataUpdate.Invoke();
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
	
	void ResetPlayerData()
	{
		m_iColorTeam = CSI_EColorTeam.NONE;
		m_iOverrideIcon = CSI_EOverrideIcon.AUTO;
		m_bIsTeamLeader = false;
	}

	//------------------------------------------------------------------------------------------------
	void SetPlayerId(int playerId)
	{
		m_iPlayerID = playerId;
	}
	
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
	void SetRank(string rank)
	{
		m_iRank = rank;
	}
	
	void SetIsTeamLeader(bool isTL)
	{
		m_bIsTeamLeader = isTL;
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	CSI_EColorTeam GetColorTeam()
	{
		return m_iColorTeam;
	}

	//------------------------------------------------------------------------------------------------
	CSI_EIcon GetDisplayIcon()
	{
		if (m_iOverrideIcon != CSI_EOverrideIcon.AUTO)
			return m_iOverrideIcon;
		else
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
	// REPLICATION STUFF
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iPlayerID);
		writer.WriteInt(m_iColorTeam);
		writer.WriteInt(m_iOverrideIcon);
		writer.WriteInt(m_iDisplayIcon);
		writer.WriteInt(m_iRank);
		writer.WriteBool(m_bIsTeamLeader);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_iPlayerID);
		reader.ReadInt(m_iColorTeam);
		reader.ReadInt(m_iOverrideIcon);
		reader.ReadInt(m_iDisplayIcon);
		reader.ReadInt(m_iRank);
		reader.ReadBool(m_bIsTeamLeader);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(CSI_PlayerData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iPlayerID);
		snapshot.SerializeInt(instance.m_iColorTeam);
		snapshot.SerializeInt(instance.m_iOverrideIcon);
		snapshot.SerializeInt(instance.m_iDisplayIcon);
		snapshot.SerializeInt(instance.m_iRank);
		snapshot.SerializeBool(instance.m_bIsTeamLeader);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CSI_PlayerData instance)
	{
		snapshot.SerializeInt(instance.m_iPlayerID);
		snapshot.SerializeInt(instance.m_iColorTeam);
		snapshot.SerializeInt(instance.m_iOverrideIcon);
		snapshot.SerializeInt(instance.m_iDisplayIcon);
		snapshot.SerializeInt(instance.m_iRank);
		snapshot.SerializeBool(instance.m_bIsTeamLeader);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeBool(packet);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
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
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(CSI_PlayerData instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iPlayerID)
			&& snapshot.CompareInt(instance.m_iColorTeam)
			&& snapshot.CompareInt(instance.m_iOverrideIcon)
			&& snapshot.CompareInt(instance.m_iDisplayIcon)
			&& snapshot.CompareInt(instance.m_iRank)
			&& snapshot.CompareBool(instance.m_bIsTeamLeader);
	}
}