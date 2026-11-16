//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-15
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Talking bars (CSI_TalkingBars.layout): five bars that bounce out from their middle line, the
//! 				middle one tallest and first, shown in place of an icon while a player talks. Created beside the
//! 				icon in its Overlay (GetOrCreate) and following its visibility, opacity, color and size.
//!
//------------------------------------------------------------------------------------------------
class CSI_TalkingBarsComponent : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	static const ResourceName LAYOUT = "{C6BD3BFE2CA9CDBB}UI/Layouts/WidgetLibrary/CSI_TalkingBars.layout";
	protected static const string ROOT_WIDGET_NAME = "TalkingBars";
	protected static const string BAR_WIDGET_NAME = "Bar";	//!< Bar0 to Bar4, left to right

	// Sizes in sixteenths of the icon
	protected static const float ICON_UNITS = 16;
	protected static const float BAR_WIDTH = 2;
	protected static const float BAR_GAP = 1;

	// Per bar, left to right
	protected static const ref array<float> BAR_HEIGHTS = {5.0, 7.0, 9.0, 7.0, 5.0};				//!< tallest, in sixteenths of the icon
	protected static const ref array<float> BAR_HALF_CYCLES = {0.50, 0.44, 0.46, 0.42, 0.52};	//!< s from shortest to tallest; uneven so it never repeats exactly
	protected static const ref array<float> BAR_LEADS = {0.0, 0.1, 0.2, 0.1, 0.0};			//!< s ahead of the outer bars, so each bounce starts in the middle

	protected static const float SHORTEST = 0.3;	//!< share of its tallest height a bar shrinks to
	protected static const int CLOCK_WRAP_MS = 3600000;	//!< the clock restarts hourly, keeping float precision
	protected static const int ANIMATE_INTERVAL_MS = 33;	//!< ~30 updates a second; the bars move under half a pixel in between

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected ImageWidget m_wIcon;	//!< icon the bars stand in for
	protected ref array<ImageWidget> m_aBars = {};
	protected bool m_bHasAllBars;	//!< Bar0 to Bar4 were found
	protected bool m_bActive;

	// Last values set (Animate)
	protected int m_iAnimatedAtMs;
	protected float m_fUnit = -1;	//!< icon size / ICON_UNITS the bar widths and gaps were set for
	protected float m_fOpacity = -1;
	protected int m_iColor;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		for (int i, count = BAR_HEIGHTS.Count(); i < count; i++)
		{
			ImageWidget bar = ImageWidget.Cast(w.FindAnyWidget(BAR_WIDGET_NAME + i.ToString()));
			if (bar)
				m_aBars.Insert(bar);
		}

		m_bHasAllBars = m_aBars.Count() == BAR_HEIGHTS.Count();
		if (!m_bHasAllBars)
		{
			int lastBar = BAR_HEIGHTS.Count() - 1;
			PrintFormat("CSI_TalkingBarsComponent: expected images '%1' to '%2', the bars won't animate", BAR_WIDGET_NAME + "0", BAR_WIDGET_NAME + lastBar.ToString(), level: LogLevel.WARNING);
		}

		// Shown by SetActive; visible in the layout for editing
		if (!SCR_Global.IsEditMode())
			w.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_bActive && GetGame())
			GetGame().GetCallqueue().Remove(Animate);

		super.HandlerDeattached(w);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 ANIMATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] active true shows and animates the bars every frame, false hides them
	void SetActive(bool active)
	{
		if (active == m_bActive)
			return;

		m_bActive = active;
		if (active)
		{
			// Shown and set on the first Animate
			m_iAnimatedAtMs = System.GetTickCount() - ANIMATE_INTERVAL_MS;
			m_fOpacity = -1;
			m_iColor = 0;
			GetGame().GetCallqueue().CallLater(Animate, 0, true);
			Animate();
		}
		else
		{
			GetGame().GetCallqueue().Remove(Animate);
			m_wRoot.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return true while shown and animating
	bool IsActive()
	{
		return m_bActive;
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame while active, at most every ANIMATE_INTERVAL_MS: follows the icon and moves the bars.
	protected void Animate()
	{
		if (!m_wIcon || !m_bHasAllBars)
			return;

		int clockMs = System.GetTickCount();
		if (clockMs - m_iAnimatedAtMs < ANIMATE_INTERVAL_MS && clockMs >= m_iAnimatedAtMs)
			return;

		m_iAnimatedAtMs = clockMs;

		// Nothing to draw while the icon or its nametag is hidden
		bool shown = m_wIcon.IsVisibleInHierarchy();
		if (shown != m_wRoot.IsVisible())
			m_wRoot.SetVisible(shown);

		if (!shown)
			return;

		// The icon fades and recolors (talking color) on its own; the bars do the same
		float opacity = m_wIcon.GetOpacity();
		if (opacity != m_fOpacity)
		{
			m_fOpacity = opacity;
			m_wRoot.SetOpacity(opacity);
		}

		int color = m_wIcon.GetColorInt();
		if (color != m_iColor)
		{
			m_iColor = color;
			m_wRoot.SetColorInt(color);
		}

		float unit = m_wIcon.GetSize()[1] / ICON_UNITS;
		if (unit != m_fUnit)
		{
			m_fUnit = unit;
			foreach (ImageWidget bar : m_aBars)
			{
				AlignableSlot.SetPadding(bar, BAR_GAP * unit * 0.5, 0, BAR_GAP * unit * 0.5, 0);
			}
		}

		// Each bar eases between SHORTEST and its tallest height, centered on the middle line
		clockMs -= (clockMs / CLOCK_WRAP_MS) * CLOCK_WRAP_MS;	// no % in Enforce Script; wrapped before the float conversion
		float time = clockMs * 0.001;
		float width = BAR_WIDTH * unit;
		foreach (int i, ImageWidget bar : m_aBars)
		{
			float bounce = 0.5 - 0.5 * Math.Cos(Math.PI * (time + BAR_LEADS[i]) / BAR_HALF_CYCLES[i]);
			bar.SetSize(width, BAR_HEIGHTS[i] * unit * (SHORTEST + (1 - SHORTEST) * bounce));
		}
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSORS
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] icon an image inside an OverlayWidget
	//! \return the bars beside the icon, created on first use; null if the icon isn't in an Overlay
	static CSI_TalkingBarsComponent GetOrCreate(notnull ImageWidget icon)
	{
		CSI_TalkingBarsComponent talkingBars = Find(icon);
		if (talkingBars)
			return talkingBars;

		OverlayWidget overlay = OverlayWidget.Cast(icon.GetParent());
		if (!overlay)
			return null;

		Widget root = GetGame().GetWorkspace().CreateWidgets(LAYOUT, overlay);
		if (!root)
			return null;

		talkingBars = CSI_TalkingBarsComponent.Cast(root.FindHandler(CSI_TalkingBarsComponent));
		if (talkingBars)
			talkingBars.m_wIcon = icon;

		return talkingBars;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] icon
	//! \return the bars beside the icon, null until GetOrCreate made them
	static CSI_TalkingBarsComponent Find(notnull ImageWidget icon)
	{
		OverlayWidget overlay = OverlayWidget.Cast(icon.GetParent());
		if (!overlay)
			return null;

		Widget root = overlay.FindAnyWidget(ROOT_WIDGET_NAME);
		if (!root)
			return null;

		return CSI_TalkingBarsComponent.Cast(root.FindHandler(CSI_TalkingBarsComponent));
	}
}
