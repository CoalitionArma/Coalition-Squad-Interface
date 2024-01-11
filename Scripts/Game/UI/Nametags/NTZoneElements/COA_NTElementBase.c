/* NOTE: NOT WORKING YET

	Originally I was going to edit the default configs to include team color definitions in it. Not sure if possible yet.

*/


//------------------------------------------------------------------------------------------------
//! Nametag zone element base class, not for setup
[BaseContainerProps()]
modded class SCR_NTElementBase
{		
	protected ref map<ECOATagEntityState, SCR_NTStateConfigBase> m_aEntityStatesMap = new map< ECOATagEntityState, SCR_NTStateConfigBase>;
}

[BaseContainerProps()]
modded class SCR_NTStateConfigBase
{
	[Attribute("", UIWidgets.ComboBox, "Configure selected entity state", "", ParamEnumArray.FromEnum(ECOATagEntityState))]
	int m_eEntityState;
		
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Color")]
	ref Color m_vColor; 
		
	[Attribute("1", UIWidgets.Slider, desc: "Default opacity when in line of sight", "0 1 1")]
	float m_fOpacityDefault;
	
	[Attribute("true", UIWidgets.CheckBox, "If tag fade time is set, choose whether this state should skip the animation")]
	bool m_bAnimateTransition;
};

