[ComponentEditorProps(category: "GameScripted/Client", description: "CSI Character Component For Setting Default Values", color: "0 0 255 255")]
class CSI_CharacterComponentClass : ScriptComponentClass {};

class CSI_CharacterComponent : ScriptComponent
{		
	[Attribute(defvalue: "false", desc: "Should we set players starting color team/icon every time they respawn", category: "Default Player Settings")]
	protected bool m_bOverrideOnRespawn;
	
	[RplProp()]
	protected ref TStringArray m_aUnitPrefabColorTeams;
	
	[RplProp()]
	protected ref TStringArray m_aUnitPrefabOverrideIcons;
	
	[RplProp()]
	protected int m_iUnitPrefabIndex;
	
	//------------------------------------------------------------------------------------------------
	bool GetOverrideOnSpawn()
	{
		return m_bOverrideOnRespawn;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIndex()
	{
		return m_iUnitPrefabIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	TStringArray GetColorTeamArray()
	{
		return m_aUnitPrefabColorTeams;
	}
	
	//------------------------------------------------------------------------------------------------
	TStringArray GetOverrideIcons()
	{
		return m_aUnitPrefabOverrideIcons;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefaults(int index, array<string> colorTeamArray, array<string> overrideIconArray)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;

		m_aUnitPrefabColorTeams    = colorTeamArray;
		m_aUnitPrefabOverrideIcons = overrideIconArray;
		m_iUnitPrefabIndex         = index;
		
		Replication.BumpMe();
	}
}