#include "ColorPickerTool.h"

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

ColorPickerTool::ColorPickerTool(Canvas* canvas) :AbstractDrawingTool(canvas)
{

}

void ColorPickerTool::beginDraw(int x, int y)
{

}

void ColorPickerTool::continueDraw(int x, int y, DWORD flags)
{

}

void ColorPickerTool::endDraw(int x, int y)
{
	Gdiplus::Color color;
	canvas_->getBufferBitmap()->GetPixel(x,y, &color);
	canvas_->setForegroundColor(color);
	if ( canvas_->onForegroundColorChanged ) {
		canvas_->onForegroundColorChanged(color);
	}
	canvas_->setPreviousDrawingTool();
}

void ColorPickerTool::render(Painter* gr)
{

}

ImageEditor::CursorType ColorPickerTool::getCursor(int x, int y)
{
	return ctColorPicker;
}

void ColorPickerTool::rightButtonClick(int x, int y)
{
	Gdiplus::Color color;
	canvas_->getBufferBitmap()->GetPixel(x,y, &color);
	canvas_->setBackgroundColor(color);
	if ( canvas_->onBackgroundColorChanged ) {
		canvas_->onBackgroundColorChanged(color);
	}
	canvas_->setPreviousDrawingTool();
}

}