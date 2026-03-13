class CSI_WidgetOpacityHelper
{
	protected Widget m_Widget;
	
	//------------------------------------------------------------------------------------------------
	void SetWidget(Widget widget)
	{
		m_Widget = widget;
	}
	
	//------------------------------------------------------------------------------------------------
	void FadeAndHideWidget()
	{
		if (AnimateWidget.IsAnimating(m_Widget))
			return;
		
		if (m_Widget.GetOpacity() < 0.1)
			m_Widget.SetVisible(false);
		else
			AnimateWidget.Opacity(m_Widget, 0, 3);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnHideAndShowWidget()
	{
		if (AnimateWidget.IsAnimating(m_Widget))
			return;
		
		m_Widget.SetVisible(true);
		
		if (m_Widget.GetOpacity() == 0)
			AnimateWidget.Opacity(m_Widget, 1, 3);
	}
}