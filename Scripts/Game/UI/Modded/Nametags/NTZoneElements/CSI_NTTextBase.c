//------------------------------------------------------------------------------------------------
//! Nametag element for group text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class CSI_NTGroupName : SCR_NTTextBase
{
	//------------------------------------------------------------------------------------------------
	string GetGroupNameStr(SCR_NameTagData data)
	{
		return data.GetGroupName();
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupText(SCR_NameTagData data, string groupName, int index)
	{
		if (!data.m_aNametagElements[index]) return;
		TextWidget.Cast(data.m_aNametagElements[index]).SetText(groupName);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		SetGroupText(data, GetGroupNameStr(data), index);
	}
}

//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
modded class SCR_NTName
{
	//------------------------------------------------------------------------------------------------
	string GetPlayerColorTeamStr(SCR_NameTagData data)
	{
		return data.GetPlayerColorTeam();
	}

	//------------------------------------------------------------------------------------------------
	void SetPlayerColorTeam(SCR_NameTagData data, string colorTeam, int index)
	{
		if (colorTeam.IsEmpty()) return;
		TextWidget.Cast(data.m_aNametagElements[index]).SetColorInt(colorTeam.ToInt());
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);

		data.UpdateAttatchedTo();

		string name;
		array<string> nameParams = {};

		GetText(data, name, nameParams);

		if (name.IsEmpty())
			SetText(data, "GETNAME_ERROR", nameParams, index);
		else
		{
			SetText(data, name, nameParams, index);
			data.m_Flags &= ~ENameTagFlags.NAME_UPDATE;
		}

		SetPlayerColorTeam(data, GetPlayerColorTeamStr(data), index);
	};
}
