class CSI_UIHelper
{	
	static ref array<CSI_EIcon> m_aNonThemedIcons =
	{
		// ----------- MAN -----------
		CSI_EIcon.MAN,
		CSI_EIcon.OFFICER,
		CSI_EIcon.SL,
		CSI_EIcon.TL,
		CSI_EIcon.RTO,

		// ----------- VEHICLE -----------
		CSI_EIcon.DRIVER,
		CSI_EIcon.PASSANGER,
		CSI_EIcon.COMMANDER,
		CSI_EIcon.GUNNER,
		CSI_EIcon.HELIPILOT,
		CSI_EIcon.HELICREW,
	};
	
	//------------------------------------------------------------------------------------------------
	static string CheckEllipsis(TextWidget testWidget, float maxLength, string name)
	{
		float sx = 0;
		float yx = 0;
		
		testWidget.SetText(name);
		testWidget.GetTextSize(sx, yx);

		if (sx > maxLength) 
		{
			for (int e = 0; sx > maxLength - 3.5; e++)
			{
				int nameLength = name.Length();
				nameLength = nameLength - 1;
				name = name.Substring(0, nameLength);

				testWidget.SetText(name);
				testWidget.GetTextSize(sx, yx);
			};
			name = string.Format("%1...", name);
		};
		return name;
	};
}