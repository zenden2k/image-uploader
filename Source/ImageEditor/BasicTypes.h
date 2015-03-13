#ifndef IMAGEEDITOR_BASICTYPES_H
#define IMAGEEDITOR_BASICTYPES_H

namespace ImageEditor {
	enum ElementType { etUnknown, etCrop , etArrow, etLine, etRectangle };
	enum BoundaryType { btTopLeft, btTop, btTopRight, btRight, btBottomRight, btBottom,  btBottomLeft,  btLeft,   btNone};
	enum CursorType {
		ctDefault, ctEdit, ctResizeVertical, ctResizeHorizontal, ctResizeDiagonalMain, ctResizeDiagonalAnti, ctCross, ctMove
	};
	enum Axis { axisX, axisY };
}

#endif