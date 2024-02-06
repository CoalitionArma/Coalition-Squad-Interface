//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class COA_NTGroupName : SCR_NTTextBase
{		
	//------------------------------------------------------------------------------------------------
 	string GetGroupText(SCR_NameTagData data)
	{	
		return data.GetGroupName();
	}
	
	//------------------------------------------------------------------------------------------------
 	void SetGroupText(SCR_NameTagData data, string text, int index)
	{	
		if (!data.m_aNametagElements[index]) {
			Print("[CSI NT] Wrong name tag element index");
			return;
		}
		
		Print("[CSI NT] Setting nametag");
		TextWidget.Cast( data.m_aNametagElements[index] ).SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		if (data.m_Flags & ECOANameTagFlags.GROUP_UPDATE)
		{
			string groupName = GetGroupText(data);

			SetGroupText(data, groupName, index);
			data.m_Flags &= ~ECOANameTagFlags.GROUP_UPDATE;
		}
	}
};