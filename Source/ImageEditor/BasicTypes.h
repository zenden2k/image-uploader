#ifndef IMAGEEDITOR_BASICTYPES_H
#define IMAGEEDITOR_BASICTYPES_H

#ifdef QT_VERSION
	#define IMAGEEDITOR_QT
#else
	#define IMAGEEDITOR_GDIPLUS
	#include "3rdpart/GdiplusH.h"
#endif

#ifndef _WIN32
struct POINT {
	int x;
	int y;
};

struct RECT
{
	int    left;
	int    top;
	int    right;
	int    bottom;
};
#endif

namespace ImageEditor {
	#ifdef IMAGEEDITOR_GDIPLUS
		typedef Gdiplus::Graphics Painter;
	#endif
	enum ElementType { etUnknown, etNone, etCrop , etArrow, etLine, etRectangle,etFilledRectangle, etText, etSelection, etBlurringRectangle,
		etRoundedRectangle, etEllipse, etFilledRoundedRectangle, etFilledEllipse
	};

	// item order is important!!!!
	enum BoundaryType { btBottomRight, btBottom,  btBottomLeft,  btRight,  btLeft,  btTopLeft, btTop, btTopRight,  btNone};
	enum CursorType {
		ctDefault, ctEdit, ctResizeVertical, ctResizeHorizontal, ctResizeDiagonalMain, ctResizeDiagonalAnti, ctCross, ctMove, ctColorPicker, ctBrush
	};
	enum Axis { axisX, axisY };
}

#endif