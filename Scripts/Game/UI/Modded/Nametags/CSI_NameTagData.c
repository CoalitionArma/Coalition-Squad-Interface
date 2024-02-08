modded class SCR_NameTagData
{
	protected string m_sCargo         = "{05CAA2D974A461ED}UI\Textures\HUD\Modded\Icons\imagecargo_ca.edds";
	protected string m_sDriver        = "{9F51D41FDEB5D414}UI\Textures\HUD\Modded\Icons\imagedriver_ca.edds";
	protected string m_sGunner        = "{6049973DED62368F}UI\Textures\HUD\Modded\Icons\imagegunner_ca.edds";
	protected string m_sSquadLeader   = "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds";
	protected string m_sTeamLeader    = "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds";
	protected string m_sMedic         = "{C74F2DD12FEBFEB9}UI\Textures\HUD\Modded\Icons\Iconmanmedic_ca.edds";
	protected string m_sSniper        = "{6CD9D05A934CDA32}UI\Textures\HUD\Modded\Icons\Iconmansniper_ca.edds";
	protected string m_sMachineGunner = "{C0938BB194E60432}UI\Textures\HUD\Modded\Icons\Iconmanmg_ca.edds";
	protected string m_sAntiTank      = "{D0E196FA6DA69F07}UI\Textures\HUD\Modded\Icons\Iconmanat_ca.edds";
	protected string m_sGrenadier     = "{FBC8C841728649FC}UI\Textures\HUD\Modded\Icons\Iconmangrenadier_ca.edds";
	protected string m_sMan           = "{25A0BFBD75253292}UI\Textures\HUD\Modded\Icons\Iconman_ca.edds";
	
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
	override void GetName(out string name, out notnull array<string> nameParams)
	{		
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
			if (!m_AuthorityComponent) return;
			
			string roleNametagVisible = "";
			string roleNametagVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[7];
			switch (true)
			{
				case(roleNametagVisibleSO == "true" || roleNametagVisibleSO == "false") : { roleNametagVisible = roleNametagVisibleSO; break; };
				default : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("roleNametagVisible", roleNametagVisible); break; };
			};
			
			string rankVisible = "";
			string rankVisibleSO = m_AuthorityComponent.ReturnAuthoritySettings()[5];
			switch (true)
			{				
				case(rankVisibleSO == "true" || rankVisibleSO == "false") : { rankVisible = rankVisibleSO; break; };
				default : { GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("rankVisible", rankVisible); break; };
			};
			
			PlayerManager playerMgr = GetGame().GetPlayerManager();
			if (playerMgr)
			{
				m_sName = playerMgr.GetPlayerName(m_iPlayerID);
				if (rankVisible == "true") 
				{
					string rank = SCR_CharacterRankComponent.GetCharacterRankNameShort(GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iPlayerID));
					m_sName = string.Format("%1 %2", rank, m_sName);
				};
				if (roleNametagVisible == "true") 
				{
					SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);
					string icon = m_AuthorityComponent.ReturnLocalPlayerMapValue(group.GetGroupID(), m_iPlayerID, "DisplayIcon");		
					switch (icon) {
						case m_sCargo         : { m_sName = string.Format("%1 [PAX]", m_sName); break;};
						case m_sDriver        : { m_sName = string.Format("%1 [DRV]", m_sName); break;};
						case m_sGunner        : { m_sName = string.Format("%1 [GUN]", m_sName); break;};
						case m_sSquadLeader   : { m_sName = string.Format("%1 [SL]", m_sName);  break;};
						case m_sTeamLeader    : { m_sName = string.Format("%1 [FTL]", m_sName);  break;};
						case m_sMedic         : { m_sName = string.Format("%1 [MED]", m_sName); break;};
						case m_sSniper        : { m_sName = string.Format("%1 [SNP]", m_sName); break;};
						case m_sMachineGunner : { m_sName = string.Format("%1 [AR]", m_sName);  break;};
						case m_sAntiTank      : { m_sName = string.Format("%1 [RAT]", m_sName); break;};
						case m_sGrenadier     : { m_sName = string.Format("%1 [GL]", m_sName);  break;};
					}
				};
			} else { m_sName = "No player manager!" };
		}
		else if (m_eType == ENameTagEntityType.AI)
		{
			SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(m_Entity.FindComponent(SCR_CharacterIdentityComponent));
			if (scrCharIdentity)
			{
				scrCharIdentity.GetFormattedFullName(m_sName, m_aNameParams);
			}
			else
			{
				CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(m_Entity.FindComponent(CharacterIdentityComponent));
				if (charIdentity && charIdentity.GetIdentity())
					m_sName = charIdentity.GetIdentity().GetName();
				else 
					m_sName = "No character identity!";
			}
		}
		
		name = m_sName;
		nameParams.Copy(m_aNameParams);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetGroup(SCR_AIGroup group)
	{
		if (group)
		{
			m_iGroupID = group.GetGroupID();
			if (m_bIsCurrentPlayer)
			{
				m_NTDisplay.CleanupAllTags();	// cleanup other tags because we need to compare groups again
				return;
			}
		}
		else
		{
			m_iGroupID = -1;

			if (m_bIsCurrentPlayer)
				m_NTDisplay.CleanupAllTags();
		}
		
		if (m_VehicleParent && m_VehicleParent.m_MainTag == this)	// update vehicle parent as well if this tag is its main tag
			m_VehicleParent.m_Flags |= ENameTagFlags.NAME_UPDATE;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetGroupName()
	{
		// TODO: Better AI handling
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);
		
		if (!group) return "";
		
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
		
		return m_sGroupName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerColorTeam()
	{
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		if (!m_AuthorityComponent) return 0;
		
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);
		SCR_AIGroup localGroup = m_GroupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		
		if (!group || !localGroup) return 0;
		
		string colorTeam = m_AuthorityComponent.ReturnLocalPlayerMapValue(localGroup.GetGroupID(), m_iPlayerID, "ColorTeam");
		
		if (colorTeam == "" || localGroup != group  || (m_ePriorityEntityState & ENameTagEntityState.VON)) return 0;
		
		m_iColorTeamInt = colorTeam.ToInt();
		return m_iColorTeamInt;
	}
	
	void UpdateAttatchedTo() 
	{
		GetGame().GetGameUserSettings().GetModule("CSI_GameSettings").Get("nametagsPosition", m_sNametagsPos);
		
		if (m_sNametagsPos == "HEAD") {
			m_eAttachedTo = ENameTagPosition.HEAD;
			m_eAttachedToLast = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
			m_eAttachedToLast = ENameTagPosition.BODY;
		};
	}
	
	// Overriding this whole thing to force these fucking tags to the body
	//------------------------------------------------------------------------------------------------
	override void SetTagPosition(ENameTagPosition pos, bool gradualChange = false)
	{
		if (m_sNametagsPos == "HEAD") {
			m_eAttachedTo = ENameTagPosition.HEAD;
			m_eAttachedToLast = ENameTagPosition.HEAD;
		} else {
			m_eAttachedTo = ENameTagPosition.BODY;
			m_eAttachedToLast = ENameTagPosition.BODY;
		};

		m_fTimeSlicePosChange = 0;
		m_vTagWorldPosLast = m_vTagWorldPos;
	}
}