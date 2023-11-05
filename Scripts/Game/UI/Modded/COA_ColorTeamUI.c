modded enum ChimeraMenuPreset {
    COA_ColorTeamUI
}

class ColorTeamSelectionClass: ChimeraMenuBase
{
	
	protected string m_sSquadLeader = "{5ECE094ED4662B33}Layouts\UI\Textures\Icons\iconmanleader_ca.edds";
	protected string m_sTeamLeader = "{6D45BA2CCC322312}Layouts\UI\Textures\Icons\iconmanteamleader_ca.edds";
	protected string m_sMedic = "{01F2523A4EE5C48B}Layouts\UI\Textures\Icons\iconmanmedic_ca.edds";
	protected string m_sSniper = "{318B797C57BE3C29}Layouts\UI\Textures\Icons\iconmansniper_ca.edds";
	protected string m_sMachineGunner = "{CCF40410BDB53870}Layouts\UI\Textures\Icons\iconmanmg_ca.edds";
	protected string m_sAntiTank = "{DC86195B44F5A345}Layouts\UI\Textures\Icons\iconmanat_ca.edds";
	protected string m_sGrenadier = "{B7757F2024A3DC87}Layouts\UI\Textures\Icons\iconmangrenadier_ca.edds";
	protected string m_sMan = "{71ED761DF5BA041C}Layouts\UI\Textures\Icons\iconman_ca.edds";
	
	
    protected Widget m_wRoot;    
    protected SCR_ButtonTextComponent comp;
    IEntity myCallerEntity;  // (Added) this variable will be used only to show how to transfer parameters to the GUI.            
    
    //------------------------------------------------------------------------------------------------
    override void OnMenuOpen()
    {
        m_wRoot = GetRootWidget();
        
        // SendHint
        comp = SCR_ButtonTextComponent.GetButtonText("SendHint", m_wRoot);
        if (comp)
        {
            GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());            
            comp.m_OnClicked.Insert(myCustomFunction);
        }                
    }
    
    // myCustomFunction called when SendHint Button is pressed
    void myCustomFunction()
    {
      SCR_HintManagerComponent.GetInstance().ShowCustomHint("This executes when pressing SendHint Button (Using OnClick)", "MY GUI", 3.0);
    }   
}