//------------------------------------------------------------------------------------------------
//! Nametag element for group text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase
{
	protected int updateFrame = 15;
	
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
		
		if (updateFrame < 15) {
			updateFrame++;
			return;
		};
		
		updateFrame = 0;
		
		SetGroupText(data, GetGroupText(data), index);
	}
};

//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTName
{
	protected int updateFrame = 30;
	
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
		
		if (updateFrame < 30) {
			updateFrame++;
			return;
		};
		
		updateFrame = 0;
		
		data.UpdateAttatchedTo();
		
		string name;
		array<string> nameParams = {};
			
		GetText(data, name, nameParams);
		
		if (name == string.Empty)
			SetText(data, "GETNAME_ERROR", nameParams, index);
		else
		{
			SetText(data, name, nameParams, index);
			data.m_Flags &= ~ENameTagFlags.NAME_UPDATE;
		}
		
		SetPlayerColorTeam(data, GetPlayerColorTeamInt(data), index);
	};
}