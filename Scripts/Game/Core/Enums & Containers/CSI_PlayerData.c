//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	One player's CSI state: color team, override and display icons, rank and leader flags.
//! 
//------------------------------------------------------------------------------------------------
class CSI_PlayerData
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected int m_iPlayerID;
	protected CSI_EColorTeam m_iColorTeam = CSI_EColorTeam.NONE;
	protected CSI_EOverrideIcon m_iOverrideIcon = CSI_EOverrideIcon.AUTO;	//!< set by the squad leader
	protected CSI_EIcon m_iDisplayIcon = CSI_EIcon.MAN;						//!< self-reported by the owning client
	protected SCR_ECharacterRank m_iRank = SCR_ECharacterRank.PRIVATE;
	protected bool m_bIsTeamLeader;
	protected bool m_bIsSquadLeader;

	protected ref ScriptInvokerVoid m_OnDataUpdate;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 SETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	void SetPlayerID(PlayerId playerID)
	{
		m_iPlayerID = playerID;
	}

	//------------------------------------------------------------------------------------------------
	void SetColorTeam(CSI_EColorTeam colorTeam)
	{
		if (m_iColorTeam == colorTeam)
			return;

		m_iColorTeam = colorTeam;

		if (m_OnDataUpdate)
			m_OnDataUpdate.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! Doesn't invoke GetOnDataUpdate().
	//! \param[in] overrideIcon
	void SetOverrideIcon(CSI_EOverrideIcon overrideIcon)
	{
		m_iOverrideIcon = overrideIcon;
	}

	//------------------------------------------------------------------------------------------------
	void SetDisplayIcon(CSI_EIcon displayIcon)
	{
		if (m_iDisplayIcon == displayIcon)
			return;

		m_iDisplayIcon = displayIcon;

		if (m_OnDataUpdate)
			m_OnDataUpdate.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void SetRank(SCR_ECharacterRank rank)
	{
		if (m_iRank == rank)
			return;

		m_iRank = rank;

		if (m_OnDataUpdate)
			m_OnDataUpdate.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! Doesn't invoke GetOnDataUpdate().
	//! \param[in] isTL
	void SetIsTeamLeader(bool isTL)
	{
		m_bIsTeamLeader = isTL;
	}

	//------------------------------------------------------------------------------------------------
	//! Doesn't invoke GetOnDataUpdate().
	//! \param[in] isSL
	void SetIsSquadLeader(bool isSL)
	{
		m_bIsSquadLeader = isSL;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 GETTER METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		return m_iPlayerID;
	}

	//------------------------------------------------------------------------------------------------
	CSI_EColorTeam GetColorTeam()
	{
		return m_iColorTeam;
	}

	//------------------------------------------------------------------------------------------------
	//! \return AUTO if there's no override
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
	//! \return fired when the color team, display icon or rank changes
	ScriptInvokerVoid GetOnDataUpdate()
	{
		if (!m_OnDataUpdate)
			m_OnDataUpdate = new ScriptInvokerVoid();

		return m_OnDataUpdate;
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 REPLICATION METHODS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! JIP save; field order must match Load.
	//! \param[in] writer
	void Save(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iPlayerID);
		writer.WriteInt(m_iColorTeam);
		writer.WriteInt(m_iOverrideIcon);
		writer.WriteInt(m_iDisplayIcon);
		writer.WriteInt(m_iRank);
		writer.WriteBool(m_bIsTeamLeader);
		writer.WriteBool(m_bIsSquadLeader);
	}

	//------------------------------------------------------------------------------------------------
	//! JIP load; field order must match Save.
	//! \param[in] reader
	void Load(ScriptBitReader reader)
	{
		reader.ReadInt(m_iPlayerID);
		reader.ReadInt(m_iColorTeam);
		reader.ReadInt(m_iOverrideIcon);
		reader.ReadInt(m_iDisplayIcon);
		reader.ReadInt(m_iRank);
		reader.ReadBool(m_bIsTeamLeader);
		reader.ReadBool(m_bIsSquadLeader);
	}
}