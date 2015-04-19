#ifndef VectorElementTool_h__
#define VectorElementTool_h__
#include "3rdpart/GdiplusH.h"
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include <stdint.h>
#include "MoveAndResizeTool.h"

namespace ImageEditor {

	class Canvas;

class VectorElementTool: public MoveAndResizeTool {
public:
	VectorElementTool( Canvas* canvas, ElementType type );

	virtual CursorType getCursor(int x, int y);
};


}
#endif // VectorElementTool_h__