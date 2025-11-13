modded class SCR_GroupMenu
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();		
		
		Widget playerSettings = GetRootWidget().FindAnyWidget("PlayersSettings");
		
		if (playerSettings)
			playerSettings.SetVisible(false);
	}
}