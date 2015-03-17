#ifndef IMAGEEDITOR_BASICTYPES_H
#define IMAGEEDITOR_BASICTYPES_H

#ifdef QT_VERSION
	#define IMAGEEDITOR_QT
#else
	#define IMAGEEDITOR_GDIPLUS
	#include <GdiPlus.h>
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
}
#endif

namespace ImageEditor {
	#ifdef IMAGEEDITOR_GDIPLUS
		typedef Gdiplus::Graphics Painter;
	#endif
	enum ElementType { etUnknown, etNone, etCrop , etArrow, etLine, etRectangle, etText, etSelection };
	enum BoundaryType { btTopLeft, btTop, btTopRight, btRight, btBottomRight, btBottom,  btBottomLeft,  btLeft,   btNone};
	enum CursorType {
		ctDefault, ctEdit, ctResizeVertical, ctResizeHorizontal, ctResizeDiagonalMain, ctResizeDiagonalAnti, ctCross, ctMove
	};
	enum Axis { axisX, axisY };
}

#endif