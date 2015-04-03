#ifndef SelectionTool_h__
#define SelectionTool_h__

#include <3rdpart/GdiplusH.h>
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include <stdint.h>
#include "MoveAndResizeTool.h"
namespace ImageEditor {

class Canvas;

class SelectionTool : public MoveAndResizeTool {
public:
	SelectionTool( Canvas* canvas );
};

}
#endif // SelectionTool_h__