//------------------------------------------------------------------------------------------------
//! Nametag element for group text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase
{		
	//------------------------------------------------------------------------------------------------
 	string GetGroupText(SCR_NameTagData data)
	{	
		return data.GetGroupName();
	}
	
	//------------------------------------------------------------------------------------------------
 	void SetGroupText(SCR_NameTagData data, string text, int index)
	{	
		if (!data.m_aNametagElements[index]) return;
		
		TextWidget.Cast( data.m_aNametagElements[index] ).SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);

		SetGroupText(data, GetGroupText(data), index);
	}
};

//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTName
{
	//------------------------------------------------------------------------------------------------
	int GetPlayerColorTeamInt(SCR_NameTagData data)
	{	
		return data.GetPlayerColorTeam();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerColorTeam(SCR_NameTagData data, int colorTeam, int index) 
	{
		if (!data.m_aNametagElements[index]) return;
	
		TextWidget.Cast( data.m_aNametagElements[index] ).SetColorInt(colorTeam);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		data.UpdateAttatchedTo();
		
		SetPlayerColorTeam(data, GetPlayerColorTeamInt(data), index);
	};
}