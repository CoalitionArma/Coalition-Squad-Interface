//------------------------------------------------------------------------------------------------
//!
//! \Author: 		Njpatman
//! \Last Updated: 	2026-11-21
//! \Source: 		Coalition Squad Interface
//!	\Description: 	Draws vehicle outlines (CSI_VehicleOutline) on a canvas: a piece per hull, turret or gun,
//! 				faded out just inside the compass ring, and a ring per empty seat.
//!
//------------------------------------------------------------------------------------------------
class CSI_VehicleOutlineComponent : SCR_ScriptedWidgetComponent
{
//=============================================================================================================================================================================================================================================================================================================================================================
//	 CONSTANTS
//=============================================================================================================================================================================================================================================================================================================================================================

	// Lines by kind: share of the canvas width, and opacity
	protected static const float HULL_LINE_WIDTH = 0.009;
	protected static const float TURRET_LINE_WIDTH = 0.012;
	protected static const float GUN_LINE_WIDTH = 0.009;		//!< guns get no fill or soft edge
	protected static const float MIN_GUN_LINE_WIDTH = 1.25;		//!< px
	protected static const float HULL_LINE_OPACITY = 0.6;
	protected static const float TURRET_LINE_OPACITY = 0.85;
	protected static const float GUN_LINE_OPACITY = 1;

	// Under each line: a dark rim for bright ground, then a soft edge
	protected static const float RIM_WIDTH = 2;					//!< px wider than the line
	protected static const float RIM_OPACITY = 0.6;				//!< of the line's
	protected static const float SOFT_EDGE_WIDTH = 1;			//!< px wider than the line
	protected static const float SOFT_EDGE_OPACITY = 0.35;		//!< of the line's

	// Fills under each piece
	protected static const float HULL_FILL_OPACITY = 0.1;		//!< in the line color
	protected static const float TURRET_FILL_OPACITY = 0.35;	//!< dark; guns aren't filled

	// Fade out toward the compass ring (0.406), in shares of the canvas width
	protected static const float FEATHER_START = 0.33;
	protected static const float FEATHER_MIDDLE = 0.36;
	protected static const float CLIP_RADIUS = 0.39;			//!< nothing past it
	protected static const float FEATHER_MIDDLE_OPACITY = 0.5;	//!< of the line's, from FEATHER_START
	protected static const float FEATHER_END_OPACITY = 0.2;		//!< of the line's, from FEATHER_MIDDLE
	protected static const float CENTER_GAP_OPACITY = 0.4;		//!< of the line's, around the local player's icon
	protected static const float CENTER_GAP_CORE = 0.5;			//!< share of the gap's radius where lines are cut out
	protected static const int CLIP_SIDES = 32;					//!< of the fills' clip polygon, just outside CLIP_RADIUS

	// Seat marks, in shares of the canvas width
	protected static const float SEAT_RING_RADIUS = 0.012;		//!< an empty seat
	protected static const float SEAT_LINE_WIDTH = 0.005;
	protected static const float MIN_SEAT_LINE_WIDTH = 1;		//!< px
	protected static const float SEAT_OPACITY = 0.35;			//!< faint, well under the hull's lines
	protected static const int SEAT_SEGMENTS = 10;

	protected static const float REDRAW_DISTANCE = 0.3;			//!< px a point may move before the outline is drawn again
	protected static const int LAYOUT_RETRY_MS = 10;			//!< retry while the canvas isn't laid out

	// m_aLineColors layout: by kind, then feather band, then pass
	protected static const int CENTER_GAP_BAND = 3;			//!< lines over the local player's icon, dimmed
	protected static const int BAND_COUNT = 4;
	protected static const int PASS_RIM = 0;
	protected static const int PASS_SOFT_EDGE = 1;
	protected static const int PASS_LINE = 2;
	protected static const int PASS_COUNT = 3;

//=============================================================================================================================================================================================================================================================================================================================================================
//	 RUNTIME VARIABLES
//=============================================================================================================================================================================================================================================================================================================================================================

	protected CanvasWidget m_wCanvas;
	protected ref array<ref CSI_VehicleOutlinePiece> m_aPieces = {};	//!< the first m_iPieceCount are shown
	protected int m_iPieceCount;
	protected ref array<float> m_aSeats = {};		//!< right, forward per empty seat, m
	protected float m_fWidthsPerMeter;				//!< canvas widths per meter
	protected float m_fCenterGap;					//!< share of the canvas width
	protected bool m_bCutCenterGap;					//!< leave lines out of the whole gap

	// Packed colors, set by SetColor
	protected int m_iColor;							//!< the line color they were made from
	protected ref array<int> m_aLineColors = {};	//!< by kind, feather band and pass
	protected ref array<int> m_aFillColors = {};	//!< by kind
	protected int m_iSeatColor;
	protected int m_iSeatRimColor;

	protected float m_fRotation;					//!< radians

	// What was last drawn
	protected bool m_bRedraw;						//!< draw on the next Update
	protected bool m_bRelayout;						//!< lay out every piece again first
	protected float m_fDrawnWidth;
	protected float m_fDrawnWidthsPerMeter;
	protected float m_fDrawnRotation;
	protected ref array<float> m_aDrawnSeats = {};

	// The draw in progress, px
	protected float m_fCenterX;
	protected float m_fCenterY;
	protected float m_fFeatherStart;
	protected float m_fFeatherMiddle;
	protected float m_fClipRadius;
	protected float m_fGapRadius;
	protected float m_fGapCoreRadius;				//!< lines are cut inside it, dimmed out to m_fGapRadius

	// Reused each draw: the commands, the seat rings (absolute px), and scratch arrays
	protected ref array<ref CanvasWidgetCommand> m_aCommands = {};
	protected ref array<ref LineDrawCommand> m_aLines = {};
	protected int m_iLineCount;
	protected ref array<ref array<float>> m_aSeatRuns = {};
	protected int m_iSeatRunCount;
	protected ref array<float> m_aPoints = {};			//!< a contour's points relative to the canvas center
	protected ref array<float> m_aSegments = {};		//!< pieces of its edges: from x, y, to x, y
	protected ref array<int> m_aSegmentBands = {};
	protected ref array<float> m_aSplits = {};
	protected ref array<float> m_aClipped = {};
	protected ref array<float> m_aClipInput = {};
	protected ref array<int> m_aCuttingSides = {};
	protected ref array<float> m_aClipNormals = {};		//!< x, y per side of the fill's clip polygon
	protected ref array<float> m_aSeatCircle = {};		//!< x, y per point of a seat ring of radius 1

//=============================================================================================================================================================================================================================================================================================================================================================
//	 INITIALIZATION
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Never hidden: a hidden canvas isn't laid out, so it can't be measured
		m_wCanvas = CanvasWidget.Cast(w);
		if (!m_wCanvas)
			PrintFormat("CSI_VehicleOutlineComponent: '%1' isn't a CanvasWidget, vehicle outlines won't show", w.GetName(), level: LogLevel.WARNING);

		for (int side = 0; side < CLIP_SIDES; side++)
		{
			float angle = side * Math.PI2 / CLIP_SIDES;
			m_aClipNormals.Insert(Math.Cos(angle));
			m_aClipNormals.Insert(Math.Sin(angle));
		}

		for (int segment = 0; segment < SEAT_SEGMENTS; segment++)
		{
			float segmentAngle = segment * Math.PI2 / SEAT_SEGMENTS;
			m_aSeatCircle.Insert(Math.Cos(segmentAngle));
			m_aSeatCircle.Insert(Math.Sin(segmentAngle));
		}

		SetColor(Color.FromInt(Color.WHITE));
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(Draw);

		super.HandlerDeattached(w);
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 OUTLINES
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] count pieces shown from the next Update; SetPiece sets each
	void SetPieceCount(int count)
	{
		if (count == m_iPieceCount)
			return;

		while (m_aPieces.Count() < count)
		{
			m_aPieces.Insert(new CSI_VehicleOutlinePiece());
		}

		m_iPieceCount = count;
		m_bRedraw = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Places a piece in SetRotation's frame, in meters from the canvas center.
	//! \param[in] index below SetPieceCount's count
	//! \param[in] outline null draws nothing for it (still building)
	//! \param[in] rightX right = rightX * x + rightZ * z + offsetRight
	//! \param[in] rightZ
	//! \param[in] forwardX forward = forwardX * x + forwardZ * z + offsetForward
	//! \param[in] forwardZ
	//! \param[in] offsetRight m
	//! \param[in] offsetForward m
	//! \param[in] kind sets its style
	void SetPiece(int index, CSI_VehicleOutline outline, float rightX, float rightZ, float forwardX, float forwardZ, float offsetRight, float offsetForward, CSI_EVehicleOutlineKind kind = CSI_EVehicleOutlineKind.HULL)
	{
		if (index >= m_iPieceCount)
			return;

		CSI_VehicleOutlinePiece piece = m_aPieces[index];
		if (piece.m_Outline != outline || piece.m_eKind != kind)
		{
			piece.m_Outline = outline;
			piece.m_eKind = kind;
			piece.m_bLaidOut = false;
			m_bRedraw = true;
		}

		piece.m_aTransform[0] = rightX;
		piece.m_aTransform[1] = rightZ;
		piece.m_aTransform[2] = forwardX;
		piece.m_aTransform[3] = forwardZ;
		piece.m_aTransform[4] = offsetRight;
		piece.m_aTransform[5] = offsetForward;
	}

	//------------------------------------------------------------------------------------------------
	//! Turns the whole drawing around the canvas center to the view, without laying pieces out again.
	//! \param[in] rotation radians: the view's yaw less the frame's
	void SetRotation(float rotation)
	{
		m_fRotation = rotation;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] cut leave lines out of the whole center gap, else dim them around its middle
	void SetCenterGapCut(bool cut)
	{
		if (cut == m_bCutCenterGap)
			return;

		m_bCutCenterGap = cut;
		m_bRelayout = true;
		m_bRedraw = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps the local player's icon at the canvas center readable by cutting and dimming lines around it.
	//! \param[in] radius share of the canvas width, 0 for no gap
	void SetCenterGap(float radius)
	{
		if (radius == m_fCenterGap)
			return;

		m_fCenterGap = radius;
		m_bRelayout = true;
		m_bRedraw = true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] seats right, forward per empty seat, m from the canvas center in SetRotation's frame
	void SetSeats(notnull array<float> seats)
	{
		if (seats.Count() != m_aSeats.Count())
			m_bRedraw = true;

		m_aSeats.Copy(seats);
	}

	//------------------------------------------------------------------------------------------------
	//! Draws the pieces and seats again if one changed, moved or turned more than REDRAW_DISTANCE.
	//! \param[in] widthsPerMeter canvas widths per meter
	void Update(float widthsPerMeter)
	{
		m_fWidthsPerMeter = widthsPerMeter;
		if (!m_wCanvas)
			return;

		// Always checked: it marks moved pieces to be laid out again
		bool moved = NeedsDraw();
		if (moved || m_bRedraw)
			Draw();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] color line color; each kind of piece draws it at its own opacity
	void SetColor(notnull Color color)
	{
		int packed = color.PackToInt();
		if (packed == m_iColor && !m_aLineColors.IsEmpty())
			return;

		m_iColor = packed;
		m_aLineColors.Clear();
		m_aFillColors.Clear();

		for (int kind = CSI_EVehicleOutlineKind.HULL; kind <= CSI_EVehicleOutlineKind.GUN; kind++)
		{
			float lineOpacity = GetLineOpacity(kind);
			for (int band = 0; band < BAND_COUNT; band++)
			{
				float bandOpacity = lineOpacity * GetBandOpacity(band);
				m_aLineColors.Insert(PackColor(GUIColors.DEFAULT_GLOW, bandOpacity * RIM_OPACITY));
				m_aLineColors.Insert(PackColor(color, bandOpacity * SOFT_EDGE_OPACITY));
				m_aLineColors.Insert(PackColor(color, bandOpacity));
			}

			if (kind == CSI_EVehicleOutlineKind.HULL)
				m_aFillColors.Insert(PackColor(color, HULL_FILL_OPACITY));
			else
				m_aFillColors.Insert(PackColor(GUIColors.DEFAULT_GLOW, TURRET_FILL_OPACITY));
		}

		m_iSeatColor = PackColor(color, SEAT_OPACITY);
		m_iSeatRimColor = PackColor(GUIColors.DEFAULT_GLOW, SEAT_OPACITY * RIM_OPACITY);
		m_bRedraw = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes every piece and seat and empties the canvas.
	void Clear()
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(Draw);

		foreach (CSI_VehicleOutlinePiece piece : m_aPieces)
		{
			piece.m_Outline = null;
			piece.m_bLaidOut = false;
		}

		m_iPieceCount = 0;
		m_aSeats.Clear();
		m_bRedraw = true;

		m_aCommands.Clear();
		if (m_wCanvas)
			m_wCanvas.SetDrawCommands(m_aCommands);
	}

	//------------------------------------------------------------------------------------------------
	//! Marks pieces that moved more than REDRAW_DISTANCE, or all if the size or scale changed, to be laid out again.
	//! \return true if anything moved, turned or changed size since the last draw
	protected bool NeedsDraw()
	{
		float width, height;
		m_wCanvas.GetScreenSize(width, height);
		if (width != m_fDrawnWidth || m_fWidthsPerMeter != m_fDrawnWidthsPerMeter)
		{
			m_bRelayout = true;
			return true;
		}

		float pixelsPerMeter = m_fWidthsPerMeter * width;
		bool changed;
		for (int index = 0; index < m_iPieceCount; index++)
		{
			CSI_VehicleOutlinePiece piece = m_aPieces[index];
			if (!piece.m_Outline)
				continue;

			if (!piece.m_bLaidOut)
			{
				changed = true;
				continue;
			}

			// Farthest a point can have moved, in meters
			array<float> transform = piece.m_aTransform;
			array<float> laidOut = piece.m_aLaidOutTransform;
			float axesChange = Math.AbsFloat(transform[0] - laidOut[0]) + Math.AbsFloat(transform[1] - laidOut[1])
				+ Math.AbsFloat(transform[2] - laidOut[2]) + Math.AbsFloat(transform[3] - laidOut[3]);
			float offsetChange = Math.AbsFloat(transform[4] - laidOut[4]) + Math.AbsFloat(transform[5] - laidOut[5]);
			if ((axesChange * piece.m_Outline.GetRadius() + offsetChange) * pixelsPerMeter > REDRAW_DISTANCE)
			{
				piece.m_bLaidOut = false;
				changed = true;
			}
		}

		// Turned: no drawn point moved farther than the turn's chord at the clip radius
		float turnChord = 2 * Math.AbsFloat(Math.Sin((m_fRotation - m_fDrawnRotation) * 0.5));
		if (turnChord * CLIP_RADIUS * width > REDRAW_DISTANCE)
			changed = true;

		if (m_aSeats.Count() != m_aDrawnSeats.Count())
			return true;

		for (int seat = 0, valueCount = m_aSeats.Count() - 1; seat < valueCount; seat += 2)
		{
			float seatChange = Math.AbsFloat(m_aSeats[seat] - m_aDrawnSeats[seat]) + Math.AbsFloat(m_aSeats[seat + 1] - m_aDrawnSeats[seat + 1]);
			if (seatChange * pixelsPerMeter > REDRAW_DISTANCE)
				return true;
		}

		return changed;
	}

	//------------------------------------------------------------------------------------------------
	//! Lays out moved pieces, turns everything to the view and draws it; retries until the canvas is laid out.
	protected void Draw()
	{
		if (!m_wCanvas)
			return;

		float width, height;
		m_wCanvas.GetScreenSize(width, height);
		if (width <= 0 || height <= 0)
		{
			GetGame().GetCallqueue().Remove(Draw);
			GetGame().GetCallqueue().CallLater(Draw, LAYOUT_RETRY_MS);
			return;
		}

		float pixelsPerMeter = m_fWidthsPerMeter * width;
		m_fCenterX = width * 0.5;
		m_fCenterY = height * 0.5;
		m_fFeatherStart = FEATHER_START * width;
		m_fFeatherMiddle = FEATHER_MIDDLE * width;
		m_fClipRadius = CLIP_RADIUS * width;
		m_fGapRadius = m_fCenterGap * width;
		m_fGapCoreRadius = m_fGapRadius * CENTER_GAP_CORE;
		if (m_bCutCenterGap)
			m_fGapCoreRadius = m_fGapRadius;

		// Hidden pieces too, so none keeps a layout made for another size
		if (m_bRelayout)
		{
			foreach (CSI_VehicleOutlinePiece hiddenPiece : m_aPieces)
			{
				hiddenPiece.m_bLaidOut = false;
			}
		}

		float cosTurn = Math.Cos(m_fRotation);
		float sinTurn = Math.Sin(m_fRotation);
		m_aCommands.Clear();
		m_iLineCount = 0;

		for (int index = 0; index < m_iPieceCount; index++)
		{
			CSI_VehicleOutlinePiece piece = m_aPieces[index];
			if (!piece.m_Outline)
				continue;

			if (!piece.m_bLaidOut)
				LayOutPiece(piece, pixelsPerMeter);

			DrawPiece(piece, width, cosTurn, sinTurn);
		}

		DrawSeats(pixelsPerMeter, width, cosTurn, sinTurn);
		m_wCanvas.SetDrawCommands(m_aCommands);

		m_bRedraw = false;
		m_bRelayout = false;
		m_fDrawnWidth = width;
		m_fDrawnWidthsPerMeter = m_fWidthsPerMeter;
		m_fDrawnRotation = m_fRotation;
		m_aDrawnSeats.Copy(m_aSeats);
	}

	//------------------------------------------------------------------------------------------------
	//! Lays out a piece's fill and line runs in SetRotation's frame, px from the canvas center.
	//! \param[in] piece
	//! \param[in] pixelsPerMeter
	protected void LayOutPiece(notnull CSI_VehicleOutlinePiece piece, float pixelsPerMeter)
	{
		array<float> transform = piece.m_aTransform;
		piece.m_aLaidOutTransform.Copy(transform);
		piece.m_bLaidOut = true;
		piece.m_iRunCount = 0;
		piece.m_aFillPoints.Clear();
		piece.m_aFillIndices.Clear();

		// Guns are lines only
		array<ref array<int>> triangles;
		if (piece.m_eKind != CSI_EVehicleOutlineKind.GUN)
			triangles = piece.m_Outline.GetTriangles();

		foreach (int contourIndex, array<float> contour : piece.m_Outline.GetContours())
		{
			// Points relative to the canvas center, y down
			m_aPoints.Clear();
			float farthestSq = 0;
			for (int i = 0, count = contour.Count() - 1; i < count; i += 2)
			{
				float x = contour[i];
				float z = contour[i + 1];
				float pointX = (transform[0] * x + transform[1] * z + transform[4]) * pixelsPerMeter;
				float pointY = -(transform[2] * x + transform[3] * z + transform[5]) * pixelsPerMeter;
				m_aPoints.Insert(pointX);
				m_aPoints.Insert(pointY);
				farthestSq = Math.Max(farthestSq, pointX * pointX + pointY * pointY);
			}

			if (triangles)
				AddFill(piece, triangles[contourIndex], farthestSq);

			AddRuns(piece, farthestSq);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Turns a laid out piece to the view and adds its fill, then all its rims, soft edges and lines in that order.
	//! \param[in] piece
	//! \param[in] width canvas width in px
	//! \param[in] cosTurn of SetRotation's
	//! \param[in] sinTurn of SetRotation's
	protected void DrawPiece(notnull CSI_VehicleOutlinePiece piece, float width, float cosTurn, float sinTurn)
	{
		if (!piece.m_aFillIndices.IsEmpty())
		{
			if (!piece.m_Fill)
			{
				piece.m_Fill = new TriMeshDrawCommand();
				piece.m_Fill.m_Vertices = {};
			}

			TurnPoints(piece.m_aFillPoints, piece.m_Fill.m_Vertices, cosTurn, sinTurn);
			piece.m_Fill.m_Indices = piece.m_aFillIndices;
			piece.m_Fill.m_iColor = m_aFillColors[piece.m_eKind];
			m_aCommands.Insert(piece.m_Fill);
		}

		while (piece.m_aTurnedRuns.Count() < piece.m_iRunCount)
		{
			piece.m_aTurnedRuns.Insert(new array<float>());
		}

		for (int turnedRun = 0; turnedRun < piece.m_iRunCount; turnedRun++)
		{
			TurnPoints(piece.m_aRuns[turnedRun], piece.m_aTurnedRuns[turnedRun], cosTurn, sinTurn);
		}

		float lineWidth = GetLineWidth(piece.m_eKind, width);
		int colorIndex = piece.m_eKind * BAND_COUNT * PASS_COUNT;
		for (int pass = 0; pass < PASS_COUNT; pass++)
		{
			if (pass == PASS_SOFT_EDGE && piece.m_eKind == CSI_EVehicleOutlineKind.GUN)
				continue;

			float passWidth = lineWidth;
			if (pass == PASS_RIM)
				passWidth += RIM_WIDTH;
			else if (pass == PASS_SOFT_EDGE)
				passWidth += SOFT_EDGE_WIDTH;

			for (int run = 0; run < piece.m_iRunCount; run++)
			{
				AddLine(piece.m_aTurnedRuns[run], piece.m_aRunsClosed[run], passWidth, m_aLineColors[colorIndex + piece.m_aRunBands[run] * PASS_COUNT + pass]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] points x, y pairs, px from the canvas center in SetRotation's frame
	//! \param[out] turned the same points turned to the view, absolute px
	//! \param[in] cosTurn of SetRotation's
	//! \param[in] sinTurn of SetRotation's
	protected void TurnPoints(notnull array<float> points, notnull array<float> turned, float cosTurn, float sinTurn)
	{
		int count = points.Count();
		turned.Resize(count);
		for (int i = 0; i < count - 1; i += 2)
		{
			float x = points[i];
			float y = points[i + 1];
			turned[i] = m_fCenterX + x * cosTurn + y * sinTurn;
			turned[i + 1] = m_fCenterY - x * sinTurn + y * cosTurn;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds m_aPoints' triangles to the piece's fill, cut to the clip circle.
	//! \param[in] piece
	//! \param[in] triangles index triples into m_aPoints
	//! \param[in] farthestSq the farthest point's squared distance from the center
	protected void AddFill(notnull CSI_VehicleOutlinePiece piece, notnull array<int> triangles, float farthestSq)
	{
		array<float> fillPoints = piece.m_aFillPoints;
		array<int> fillIndices = piece.m_aFillIndices;
		int firstVertex = fillPoints.Count() / 2;
		fillPoints.InsertAll(m_aPoints);

		float clipRadiusSq = m_fClipRadius * m_fClipRadius;
		bool allInside = farthestSq < clipRadiusSq;
		for (int triangle = 0, indexCount = triangles.Count() - 2; triangle < indexCount; triangle += 3)
		{
			int a = triangles[triangle];
			int b = triangles[triangle + 1];
			int c = triangles[triangle + 2];
			if (allInside || (GetDistanceSq(a) < clipRadiusSq && GetDistanceSq(b) < clipRadiusSq && GetDistanceSq(c) < clipRadiusSq))
			{
				fillIndices.Insert(firstVertex + a);
				fillIndices.Insert(firstVertex + b);
				fillIndices.Insert(firstVertex + c);
				continue;
			}

			AddClippedTriangle(piece, a, b, c);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds the part of a triangle inside the clip polygon to the piece's fill (Sutherland-Hodgman).
	//! \param[in] piece
	//! \param[in] a index into m_aPoints
	//! \param[in] b index into m_aPoints
	//! \param[in] c index into m_aPoints
	protected void AddClippedTriangle(notnull CSI_VehicleOutlinePiece piece, int a, int b, int c)
	{
		float firstX = m_aPoints[a * 2];
		float firstY = m_aPoints[a * 2 + 1];
		float secondX = m_aPoints[b * 2];
		float secondY = m_aPoints[b * 2 + 1];
		float thirdX = m_aPoints[c * 2];
		float thirdY = m_aPoints[c * 2 + 1];

		// Keep only the sides that cut it; all corners past one side means it's outside
		m_aCuttingSides.Clear();
		for (int testedSide = 0; testedSide < CLIP_SIDES; testedSide++)
		{
			float testX = m_aClipNormals[testedSide * 2];
			float testY = m_aClipNormals[testedSide * 2 + 1];
			int cornersPast = 0;
			if (firstX * testX + firstY * testY > m_fClipRadius)
				cornersPast++;

			if (secondX * testX + secondY * testY > m_fClipRadius)
				cornersPast++;

			if (thirdX * testX + thirdY * testY > m_fClipRadius)
				cornersPast++;

			if (cornersPast == 3)
				return;

			if (cornersPast > 0)
				m_aCuttingSides.Insert(testedSide);
		}

		m_aClipped.Clear();
		m_aClipped.Insert(firstX);
		m_aClipped.Insert(firstY);
		m_aClipped.Insert(secondX);
		m_aClipped.Insert(secondY);
		m_aClipped.Insert(thirdX);
		m_aClipped.Insert(thirdY);

		foreach (int side : m_aCuttingSides)
		{
			if (m_aClipped.IsEmpty())
				break;

			float normalX = m_aClipNormals[side * 2];
			float normalY = m_aClipNormals[side * 2 + 1];
			m_aClipInput.Copy(m_aClipped);
			m_aClipped.Clear();

			int count = m_aClipInput.Count() / 2;
			for (int i = 0; i < count; i++)
			{
				int next = (i + 1) % count;
				float fromX = m_aClipInput[i * 2];
				float fromY = m_aClipInput[i * 2 + 1];
				float toX = m_aClipInput[next * 2];
				float toY = m_aClipInput[next * 2 + 1];

				// Past the side when above 0
				float fromDistance = fromX * normalX + fromY * normalY - m_fClipRadius;
				float toDistance = toX * normalX + toY * normalY - m_fClipRadius;
				if (fromDistance <= 0)
				{
					m_aClipped.Insert(fromX);
					m_aClipped.Insert(fromY);
				}

				if ((fromDistance <= 0) != (toDistance <= 0))
				{
					float t = fromDistance / (fromDistance - toDistance);
					m_aClipped.Insert(fromX + t * (toX - fromX));
					m_aClipped.Insert(fromY + t * (toY - fromY));
				}
			}
		}

		int clippedCount = m_aClipped.Count() / 2;
		if (clippedCount < 3)
			return;

		array<int> fillIndices = piece.m_aFillIndices;
		int firstVertex = piece.m_aFillPoints.Count() / 2;
		piece.m_aFillPoints.InsertAll(m_aClipped);

		for (int corner = 1; corner < clippedCount - 1; corner++)
		{
			fillIndices.Insert(firstVertex);
			fillIndices.Insert(firstVertex + corner);
			fillIndices.Insert(firstVertex + corner + 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds m_aPoints, a closed line, to the piece as runs that each stay in one feather band, cut off at the clip circle.
	//! \param[in] piece
	//! \param[in] farthestSq the farthest point's squared distance from the center
	protected void AddRuns(notnull CSI_VehicleOutlinePiece piece, float farthestSq)
	{
		int pointCount = m_aPoints.Count() / 2;
		if (pointCount < 2)
			return;

		// All before the feather and clear of the center gap: one closed line
		if (farthestSq < m_fFeatherStart * m_fFeatherStart && IsClearOfGap())
		{
			StartRun(piece, 0, true).InsertAll(m_aPoints);
			return;
		}

		// Split each edge where it crosses a band's circle
		m_aSegments.Clear();
		m_aSegmentBands.Clear();
		for (int edge = 0; edge < pointCount; edge++)
		{
			int next = (edge + 1) % pointCount;
			float fromX = m_aPoints[edge * 2];
			float fromY = m_aPoints[edge * 2 + 1];
			float deltaX = m_aPoints[next * 2] - fromX;
			float deltaY = m_aPoints[next * 2 + 1] - fromY;

			m_aSplits.Clear();
			m_aSplits.Insert(0);
			m_aSplits.Insert(1);
			AddCrossings(fromX, fromY, deltaX, deltaY, m_fFeatherStart);
			AddCrossings(fromX, fromY, deltaX, deltaY, m_fFeatherMiddle);
			AddCrossings(fromX, fromY, deltaX, deltaY, m_fClipRadius);
			if (m_fGapRadius > 0)
			{
				AddCrossings(fromX, fromY, deltaX, deltaY, m_fGapRadius);
				AddCrossings(fromX, fromY, deltaX, deltaY, m_fGapCoreRadius);
			}
			m_aSplits.Sort();

			for (int split = 0, splitCount = m_aSplits.Count() - 1; split < splitCount; split++)
			{
				float start = m_aSplits[split];
				float end = m_aSplits[split + 1];
				if (end <= start)
					continue;

				float middle = (start + end) * 0.5;
				m_aSegments.Insert(fromX + start * deltaX);
				m_aSegments.Insert(fromY + start * deltaY);
				m_aSegments.Insert(fromX + end * deltaX);
				m_aSegments.Insert(fromY + end * deltaY);
				m_aSegmentBands.Insert(GetBand(fromX + middle * deltaX, fromY + middle * deltaY));
			}
		}

		int segmentCount = m_aSegmentBands.Count();
		if (segmentCount == 0)
			return;

		// Start where the band changes, so no run wraps past the end
		int first = -1;
		for (int segment = 0; segment < segmentCount; segment++)
		{
			if (m_aSegmentBands[segment] != m_aSegmentBands[(segment + segmentCount - 1) % segmentCount])
			{
				first = segment;
				break;
			}
		}

		// One band all around
		if (first < 0)
		{
			if (m_aSegmentBands[0] >= BAND_COUNT)
				return;

			array<float> loop = StartRun(piece, m_aSegmentBands[0], true);
			for (int loopSegment = 0; loopSegment < segmentCount; loopSegment++)
			{
				loop.Insert(m_aSegments[loopSegment * 4]);
				loop.Insert(m_aSegments[loopSegment * 4 + 1]);
			}

			return;
		}

		array<float> run;
		int runBand = -1;
		for (int step = 0; step < segmentCount; step++)
		{
			int current = (first + step) % segmentCount;
			int band = m_aSegmentBands[current];
			if (band >= BAND_COUNT)
			{
				run = null;
				continue;
			}

			if (!run || band != runBand)
			{
				run = StartRun(piece, band, false);
				runBand = band;
				run.Insert(m_aSegments[current * 4]);
				run.Insert(m_aSegments[current * 4 + 1]);
			}

			run.Insert(m_aSegments[current * 4 + 2]);
			run.Insert(m_aSegments[current * 4 + 3]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds to m_aSplits where an edge crosses a circle around the center, as fractions of the edge.
	//! \param[in] fromX
	//! \param[in] fromY
	//! \param[in] deltaX
	//! \param[in] deltaY
	//! \param[in] radius px
	protected void AddCrossings(float fromX, float fromY, float deltaX, float deltaY, float radius)
	{
		// |from + t * delta| = radius
		float a = deltaX * deltaX + deltaY * deltaY;
		if (a <= 0)
			return;

		float b = 2 * (fromX * deltaX + fromY * deltaY);
		float c = fromX * fromX + fromY * fromY - radius * radius;
		float discriminant = b * b - 4 * a * c;
		if (discriminant <= 0)
			return;

		float root = Math.Sqrt(discriminant);
		float enter = (-b - root) / (2 * a);
		float exit = (-b + root) / (2 * a);
		if (enter > 0 && enter < 1)
			m_aSplits.Insert(enter);

		if (exit > 0 && exit < 1)
			m_aSplits.Insert(exit);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a ring per empty seat over a dark rim, leaving out those past the feather's start.
	//! \param[in] pixelsPerMeter
	//! \param[in] width canvas width in px
	//! \param[in] cosTurn of SetRotation's
	//! \param[in] sinTurn of SetRotation's
	protected void DrawSeats(float pixelsPerMeter, float width, float cosTurn, float sinTurn)
	{
		m_iSeatRunCount = 0;
		if (m_aSeats.IsEmpty())
			return;

		float radius = SEAT_RING_RADIUS * width;
		float lineWidth = Math.Max(SEAT_LINE_WIDTH * width, MIN_SEAT_LINE_WIDTH);
		float maxDistanceSq = m_fFeatherStart * m_fFeatherStart;

		for (int seat = 0, valueCount = m_aSeats.Count() - 1; seat < valueCount; seat += 2)
		{
			float x = m_aSeats[seat] * pixelsPerMeter;
			float y = -m_aSeats[seat + 1] * pixelsPerMeter;
			if (x * x + y * y > maxDistanceSq)
				continue;

			if (m_aSeatRuns.Count() <= m_iSeatRunCount)
				m_aSeatRuns.Insert(new array<float>());

			array<float> circle = m_aSeatRuns[m_iSeatRunCount];
			m_iSeatRunCount++;
			circle.Clear();

			float centerX = m_fCenterX + x * cosTurn + y * sinTurn;
			float centerY = m_fCenterY - x * sinTurn + y * cosTurn;
			for (int point = 0, circleCount = m_aSeatCircle.Count() - 1; point < circleCount; point += 2)
			{
				circle.Insert(centerX + m_aSeatCircle[point] * radius);
				circle.Insert(centerY + m_aSeatCircle[point + 1] * radius);
			}
		}

		for (int rim = 0; rim < m_iSeatRunCount; rim++)
		{
			AddLine(m_aSeatRuns[rim], true, lineWidth + RIM_WIDTH, m_iSeatRimColor);
		}

		for (int ring = 0; ring < m_iSeatRunCount; ring++)
		{
			AddLine(m_aSeatRuns[ring], true, lineWidth, m_iSeatColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] piece
	//! \param[in] band feather band its points are in
	//! \param[in] closed joins its last point to its first
	//! \return a reused, emptied run of the piece's, px from the canvas center in SetRotation's frame
	protected array<float> StartRun(notnull CSI_VehicleOutlinePiece piece, int band, bool closed)
	{
		if (piece.m_aRuns.Count() <= piece.m_iRunCount)
		{
			piece.m_aRuns.Insert(new array<float>());
			piece.m_aRunBands.Insert(0);
			piece.m_aRunsClosed.Insert(false);
		}

		array<float> run = piece.m_aRuns[piece.m_iRunCount];
		run.Clear();
		piece.m_aRunBands[piece.m_iRunCount] = band;
		piece.m_aRunsClosed[piece.m_iRunCount] = closed;
		piece.m_iRunCount++;
		return run;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a reused line drawing a run.
	//! \param[in] vertices absolute px
	//! \param[in] closed joins its last point to its first
	//! \param[in] lineWidth px
	//! \param[in] color packed
	protected void AddLine(notnull array<float> vertices, bool closed, float lineWidth, int color)
	{
		if (m_aLines.Count() <= m_iLineCount)
			m_aLines.Insert(new LineDrawCommand());

		LineDrawCommand line = m_aLines[m_iLineCount];
		m_iLineCount++;

		line.m_Vertices = vertices;
		line.m_bShouldEnclose = closed;
		line.m_fWidth = lineWidth;
		line.m_iColor = color;
		m_aCommands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] point index into m_aPoints
	//! \return its squared distance from the center
	protected float GetDistanceSq(int point)
	{
		float x = m_aPoints[point * 2];
		float y = m_aPoints[point * 2 + 1];
		return x * x + y * y;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if no edge of m_aPoints, a closed line, comes into the center gap
	protected bool IsClearOfGap()
	{
		if (m_fGapRadius <= 0)
			return true;

		float gapSq = m_fGapRadius * m_fGapRadius;
		int pointCount = m_aPoints.Count() / 2;
		for (int edge = 0; edge < pointCount; edge++)
		{
			int next = (edge + 1) % pointCount;
			float fromX = m_aPoints[edge * 2];
			float fromY = m_aPoints[edge * 2 + 1];
			float deltaX = m_aPoints[next * 2] - fromX;
			float deltaY = m_aPoints[next * 2 + 1] - fromY;

			// The edge's nearest point to the center
			float t = 0;
			float lengthSq = deltaX * deltaX + deltaY * deltaY;
			if (lengthSq > 0)
				t = Math.Clamp(-(fromX * deltaX + fromY * deltaY) / lengthSq, 0, 1);

			float nearestX = fromX + t * deltaX;
			float nearestY = fromY + t * deltaY;
			if (nearestX * nearestX + nearestY * nearestY < gapSq)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] x px from the center
	//! \param[in] y px from the center
	//! \return a feather band, CENTER_GAP_BAND, or BAND_COUNT where nothing is drawn
	protected int GetBand(float x, float y)
	{
		float distanceSq = x * x + y * y;
		if (distanceSq < m_fGapCoreRadius * m_fGapCoreRadius)
			return BAND_COUNT;

		if (distanceSq < m_fGapRadius * m_fGapRadius)
			return CENTER_GAP_BAND;

		if (distanceSq < m_fFeatherStart * m_fFeatherStart)
			return 0;

		if (distanceSq < m_fFeatherMiddle * m_fFeatherMiddle)
			return 1;

		if (distanceSq < m_fClipRadius * m_fClipRadius)
			return 2;

		return BAND_COUNT;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] kind
	//! \param[in] width canvas width in px
	//! \return the kind's line width in px
	protected static float GetLineWidth(CSI_EVehicleOutlineKind kind, float width)
	{
		if (kind == CSI_EVehicleOutlineKind.GUN)
			return Math.Max(GUN_LINE_WIDTH * width, MIN_GUN_LINE_WIDTH);

		if (kind == CSI_EVehicleOutlineKind.TURRET)
			return TURRET_LINE_WIDTH * width;

		return HULL_LINE_WIDTH * width;
	}

	//------------------------------------------------------------------------------------------------
	protected static float GetLineOpacity(CSI_EVehicleOutlineKind kind)
	{
		if (kind == CSI_EVehicleOutlineKind.GUN)
			return GUN_LINE_OPACITY;

		if (kind == CSI_EVehicleOutlineKind.TURRET)
			return TURRET_LINE_OPACITY;

		return HULL_LINE_OPACITY;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] band feather band
	//! \return share of the line's opacity drawn in it
	protected static float GetBandOpacity(int band)
	{
		if (band == 1)
			return FEATHER_MIDDLE_OPACITY;

		if (band == 2)
			return FEATHER_END_OPACITY;

		if (band == CENTER_GAP_BAND)
			return CENTER_GAP_OPACITY;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] color
	//! \param[in] opacity
	//! \return the color at that opacity, packed
	protected static int PackColor(notnull Color color, float opacity)
	{
		Color copy = UIColors.CopyColor(color);
		copy.SetA(opacity);
		return copy.PackToInt();
	}

//=============================================================================================================================================================================================================================================================================================================================================================
//	 STATIC ACCESSOR
//=============================================================================================================================================================================================================================================================================================================================================================

	//------------------------------------------------------------------------------------------------
	//! \param[in] name the canvas widget's name
	//! \param[in] parent
	//! \return the component, null if the canvas or its component isn't found
	static CSI_VehicleOutlineComponent GetVehicleOutline(string name, notnull Widget parent)
	{
		Widget canvas = parent.FindAnyWidget(name);
		if (!canvas)
			return null;

		return CSI_VehicleOutlineComponent.Cast(canvas.FindHandler(CSI_VehicleOutlineComponent));
	}
}
