//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-19
//! \Source: 		Arma Reforger
//!	\Description: 	CSI changes: adds SetElementColor, which sets an element's shared color only when it changes.
//! 				What an element shows comes from SCR_NameTagData.UpdateCSIState and CSI_NametagSettings.
//! 
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
modded class SCR_NTElementBase
{
	//------------------------------------------------------------------------------------------------
	//! Sets the element's color unless it's the color last set here.
	//! \param[in] data
	//! \param[in] index
	//! \param[in] color a shared color, compared by reference
	protected void SetElementColor(SCR_NameTagData data, int index, notnull Color color)
	{
		Widget widget = data.m_aNametagElements[index];
		if (!widget || color == data.m_aElementColors[index])
			return;

		widget.SetColor(color);
		data.m_aElementColors[index] = color;
	}
};
