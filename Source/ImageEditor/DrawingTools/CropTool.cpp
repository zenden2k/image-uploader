#include "CropTool.h"

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"

#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>

#include <math.h>
#include <cassert>
#include <3rdpart/GdiplusH.h>
#include <math.h>

namespace ImageEditor {

CropTool::CropTool(Canvas* canvas) : MoveAndResizeTool(canvas, etCrop) {

}

void CropTool::beginDraw(int x, int y)
{
	MoveAndResizeTool::beginDraw(x,y);
}

void CropTool::continueDraw(int x, int y, DWORD flags)
{
	MoveAndResizeTool::continueDraw(x,y,flags);

}

void CropTool::endDraw(int x, int y)
{
	MoveAndResizeTool::endDraw(x, y);
	if ( currentElement_ ) {
		POINT pt = { x, y };
		if ( x == startPoint_.x && y == startPoint_.y ) {
			canvas_->deleteMovableElement(currentElement_);
			currentElement_ = 0;
			canvas_->showOverlay(false);
		}
		canvas_->updateView();
		currentElement_ = 0;
	}
}

}