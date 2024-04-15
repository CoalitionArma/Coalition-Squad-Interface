modded class SCR_NameTagData
{
	const vector BODY_OFFSET = "0 -0.245 0";			// tag visual position offset for body

	protected string m_sCargo         = "{05CAA2D974A461ED}UI\Textures\HUD\Modded\Icons\imagecargo_ca.edds";
	protected string m_sDriver        = "{9F51D41FDEB5D414}UI\Textures\HUD\Modded\Icons\imagedriver_ca.edds";
	protected string m_sGunner        = "{6049973DED62368F}UI\Textures\HUD\Modded\Icons\imagegunner_ca.edds";
	protected string m_sSquadLeader   = "{039CA0681094CD28}UI\Textures\HUD\Modded\Icons\Iconmanleader_ca.edds";
	protected string m_sTeamLeader    = "{D1A273A0110C4D5C}UI\Textures\HUD\Modded\Icons\Iconmanteamleader_ca.edds";
	protected string m_sMedic         = "{C74F2DD12FEBFEB9}UI\Textures\HUD\Modded\Icons\Iconmanmedic_ca.edds";
	protected string m_sMarksman      = "{6CD9D05A934CDA32}UI\Textures\HUD\Modded\Icons\Iconmansniper_ca.edds";
	protected string m_sMachineGunner = "{C0938BB194E60432}UI\Textures\HUD\Modded\Icons\Iconmanmg_ca.edds";
	protected string m_sAntiTank      = "{D0E196FA6DA69F07}UI\Textures\HUD\Modded\Icons\Iconmanat_ca.edds";
	protected string m_sGrenadier     = "{FBC8C841728649FC}UI\Textures\HUD\Modded\Icons\Iconmangrenadier_ca.edds";

	protected CSI_ClientComponent m_ClientComponent;
	protected CSI_AuthorityComponent m_AuthorityComponent;
	protected string m_sNametagsPos;

	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		super.InitDefaults();
		
		if (Replication.IsServer()) return;

		m_ClientComponent = CSI_ClientComponent.GetInstance();
		if (!m_ClientComponent) return;
		
		m_AuthorityComponent = CSI_AuthorityComponent.GetInstance();
		if (!m_AuthorityComponent) return;

		m_sNametagsPos = m_ClientComponent.ReturnLocalCSISettings()[11];

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
		if (!m_ClientComponent || Replication.IsServer()) return;
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			string roleNametagVisible = m_ClientComponent.ReturnLocalCSISettings()[7];
			string rankVisible = m_ClientComponent.ReturnLocalCSISettings()[5];

			PlayerManager playerMgr = GetGame().GetPlayerManager();
			if (playerMgr)
			{
				m_sName = playerMgr.GetPlayerName(m_iPlayerID);

				if (rankVisible == "true")
				{
					string rank = m_AuthorityComponent.ReturnLocalPlayerMapValue(-1, m_iPlayerID, "PlayerRank");
					if (rank != "") 
						m_sName = string.Format("%1 %2", rank, m_sName);
				};
				if (roleNametagVisible == "true")
				{
					string icon = m_AuthorityComponent.ReturnLocalPlayerMapValue(m_iGroupID, m_iPlayerID, "DisplayIcon");
					switch (icon) {
						case m_sCargo         : { m_sName = string.Format("%1 [PAX]", m_sName); break; };
						case m_sDriver        : { m_sName = string.Format("%1 [DRV]", m_sName); break; };
						case m_sGunner        : { m_sName = string.Format("%1 [GNR]", m_sName); break; };
						case m_sSquadLeader   : { m_sName = string.Format("%1 [SL]",  m_sName); break; };
						case m_sTeamLeader    : { m_sName = string.Format("%1 [FTL]", m_sName); break; };
						case m_sMedic         : { m_sName = string.Format("%1 [MED]", m_sName); break; };
						case m_sMarksman      : { m_sName = string.Format("%1 [MRK]", m_sName); break; };
						case m_sMachineGunner : { m_sName = string.Format("%1 [MG]",  m_sName); break; };
						case m_sAntiTank      : { m_sName = string.Format("%1 [AT]",  m_sName); break; };
						case m_sGrenadier     : { m_sName = string.Format("%1 [GRN]", m_sName); break; };
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
	string GetGroupName()
	{
		// TODO: Better AI handling
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group) return "";

		string groupName = group.GetCustomName();

		if (groupName.IsEmpty())
		{
			string company, platoon, squad, character, format;
			group.GetCallsigns(company, platoon, squad, character, format);
			company = WidgetManager.Translate(company);
			groupName = string.Format(format, company, platoon, squad, character);
		};

		return groupName;
	}

	//------------------------------------------------------------------------------------------------
	string GetPlayerColorTeam()
	{		
		if (!m_ClientComponent || Replication.IsServer()) return "";
		
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);

		if (!group || !m_ClientComponent || (!(m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER) || (m_ePriorityEntityState & ENameTagEntityState.VON))) return "";

		return m_AuthorityComponent.ReturnLocalPlayerMapValue(group.GetGroupID(), m_iPlayerID, "ColorTeam");;
	}

	void UpdateAttatchedTo()
	{
		if (!m_ClientComponent || Replication.IsServer()) return;

		m_sNametagsPos = m_ClientComponent.ReturnLocalCSISettings()[11];

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
