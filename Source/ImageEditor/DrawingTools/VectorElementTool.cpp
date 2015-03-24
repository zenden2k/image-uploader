#include "VectorElementTool.h"

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"

#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>

#include <math.h>
#include <cassert>
#include <gdiplus.h>
#include <math.h>

namespace ImageEditor {

VectorElementTool::VectorElementTool( Canvas* canvas, ElementType type ) : MoveAndResizeTool( canvas, type ) {
	currentElement_       = NULL;
	allowMovingElements_ = false;
}


ImageEditor::CursorType VectorElementTool::getCursor(int x, int y)
{
	return ctCross;
}

}