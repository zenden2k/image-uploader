#include "AbstractDrawingTool.h"

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"

namespace ImageEditor {

using namespace Gdiplus;

AbstractDrawingTool::AbstractDrawingTool( Canvas *canvas ) {
	startPoint_.x = 0;
	startPoint_.y = 0;
	endPoint_.x   = 0;
	endPoint_.y   = 0;
	assert( canvas );
	canvas_ = canvas;
	penSize_ = 1;
}

void AbstractDrawingTool::beginDraw( int x, int y ) {
	startPoint_.x = x;
	startPoint_.y = y;
}

void AbstractDrawingTool::endDraw( int x, int y ) {
	endPoint_.x = x;
	endPoint_.y = y;
}

void AbstractDrawingTool::mouseDoubleClick(int x, int y)
{

}

CursorType AbstractDrawingTool::getCursor(int x, int y)
{
	return ctDefault;
}

void AbstractDrawingTool::rightButtonClick(int x, int y)
{

}

void AbstractDrawingTool::setPenSize(int size)
{
	penSize_ = size;
}

void AbstractDrawingTool::setForegroundColor(Gdiplus::Color color)
{
	foregroundColor_ = color;
}

void AbstractDrawingTool::setBackgroundColor(Gdiplus::Color color)
{
	backgroundColor_ = color;
}

}