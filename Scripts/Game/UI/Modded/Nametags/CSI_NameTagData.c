modded class SCR_NameTagData
{
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected string m_sGroupName;
	protected int m_iColorTeamInt;
	protected string m_sNametagsPos;
	protected string m_sNametagsVisible;	
	protected int m_iNametagsRange;	
	
	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		super.InitDefaults();
		
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsPosition", m_sNametagsPos);
		
		if (m_sNametagsPos == "HEAD") {
			m_eAttachedTo = ENameTagPosition.HEAD;
			m_eAttachedToLast = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
			m_eAttachedToLast = ENameTagPosition.BODY;
		};
	};
	
	//------------------------------------------------------------------------------------------------
	string GetGroupName()
	{
		// TODO: Better AI handling
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		
		if (group) 
		{
			string groupName = group.GetCustomName();
			
			if (groupName == "") 
			{
				string company, platoon, squad, character, format;
				group.GetCallsigns(company, platoon, squad, character, format);
				string originalName, newName;
				
				originalName = string.Format(format, company, platoon, squad, character);
				
				groupName = originalName;
			};
			
			m_sGroupName = groupName;
		} else {m_sGroupName = "";};
		
		return m_sGroupName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerColorTeam()
	{
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		if (!m_AuthorityComponent) return 0;
		
		string colorTeam = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iPlayerID, "ColorTeam");
		
		if (colorTeam == "" || !(m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)) return ARGB(255, 225, 225, 225);
		
		m_iColorTeamInt = colorTeam.ToInt();
		return m_iColorTeamInt;
	}
	
	void UpdateAttatchedTo() 
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsPosition", m_sNametagsPos);
		
		if (m_sNametagsPos == "HEAD") {
			m_eAttachedTo = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
		};
	}
	
	// Overriding this whole thing to force these fucking tags to the body
	//------------------------------------------------------------------------------------------------
	override void SetTagPosition(ENameTagPosition pos, bool gradualChange = false)
	{
		if (m_sNametagsPos == "HEAD") {
			m_eAttachedTo = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
		};
		
		gradualChange = false;
		m_fTimeSlicePosChange = 0;
		m_vTagWorldPosLast = m_vTagWorldPos;
		
		if (!gradualChange)
			m_eAttachedToLast = pos;
	}
}