class CSI_PlayerData
{
	protected int m_iPlayerID;

	protected CSI_EColorTeam m_iColorTeam;
	protected CSI_EIcon m_iOverrideIcon;
	protected CSI_EIcon m_iDisplayIcon;
	protected CSI_EIcon m_iSpecialtyIcon;
	protected string m_sRank; //For the love of god make this a enum
	
	// Invoker for data updates
	protected ref ScriptInvoker m_OnDataUpdate;

	protected int m_iGroupID;
	
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
	void SetOverrideIcon(CSI_EIcon overrideIcon)
	{
		if (CSI_DataHelper.IsSpecialtyIcon(overrideIcon))
			m_iOverrideIcon = overrideIcon;
		else
			m_iOverrideIcon = CSI_EIcon.MAN;
	}

	//------------------------------------------------------------------------------------------------
	void SetDisplayIcon(CSI_EIcon displayIcon)
	{
		m_iDisplayIcon = displayIcon;
	}

	//------------------------------------------------------------------------------------------------
	void SetSpecialtyIcon(CSI_EIcon specialtyIcon)
	{
		if (CSI_DataHelper.IsSpecialtyIcon(specialtyIcon))
			m_iSpecialtyIcon = specialtyIcon;
		else
			m_iSpecialtyIcon = CSI_EIcon.MAN;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRank(string rank)
	{
		m_sRank = rank;
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
	CSI_EIcon GetOverrideIcon()
	{
		return m_iOverrideIcon;
	}

	//------------------------------------------------------------------------------------------------
	CSI_EIcon GetDisplayIcon()
	{
		return m_iDisplayIcon;
	}

	//------------------------------------------------------------------------------------------------
	CSI_EIcon GetSpecialtyIcon()
	{
		return m_iSpecialtyIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRank()
	{
		return m_sRank;
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
		writer.WriteInt(m_iSpecialtyIcon);
		writer.WriteString(m_sPlayerRank);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_iPlayerID);
		reader.ReadInt(m_iColorTeam);
		reader.ReadInt(m_iOverrideIcon);
		reader.ReadInt(m_iDisplayIcon);
		reader.ReadInt(m_iSpecialtyIcon);
		reader.ReadString(m_sPlayerRank);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(CRF_SlotDataContainer instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iPlayerID);
		snapshot.SerializeInt(instance.m_iColorTeam);
		snapshot.SerializeInt(instance.m_iOverrideIcon);
		snapshot.SerializeInt(instance.m_iDisplayIcon);
		snapshot.SerializeInt(instance.m_iSpecialtyIcon);
		snapshot.SerializeString(instance.m_sPlayerRank);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CRF_SlotDataContainer instance)
	{
		snapshot.SerializeInt(instance.m_iPlayerID);
		snapshot.SerializeInt(instance.m_iColorTeam);
		snapshot.SerializeInt(instance.m_iOverrideIcon);
		snapshot.SerializeInt(instance.m_iDisplayIcon);
		snapshot.SerializeInt(instance.m_iSpecialtyIcon);
		snapshot.SerializeString(instance.m_sPlayerRank);
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
		snapshot.EncodeString(packet);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeString(packet);
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
			&& lhs.CompareStringSnapshots(rhs)
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(CRF_SlotDataContainer instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iPlayerID)
			&& snapshot.CompareInt(instance.m_iColorTeam)
			&& snapshot.CompareInt(instance.m_iOverrideIcon)
			&& snapshot.CompareInt(instance.m_iDisplayIcon)
			&& snapshot.CompareInt(instance.m_iSpecialtyIcon)
			&& snapshot.CompareString(instance.m_sPlayerRank)
	}
}