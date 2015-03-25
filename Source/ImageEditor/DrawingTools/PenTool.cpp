#include "PenTool.h"

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

/*
 Pen Tool
*/

PenTool::PenTool( Canvas* canvas ): AbstractDrawingTool( canvas )  {

}

void PenTool::beginDraw( int x, int y ) {
	canvas_->currentDocument()->beginDrawing();
	oldPoint_.x = x;
	oldPoint_.y = y;
}

void PenTool::continueDraw( int x, int y, DWORD flags ) {
	if ( flags & MK_CONTROL ) {
		y = oldPoint_.y;
	}
	Line * line =  new Line( canvas_, oldPoint_.x, oldPoint_.y, x, y) ;

	line->setPenSize(1 );
	line->setColor(foregroundColor_);
	line->setBackgroundColor(backgroundColor_);

	line->setCanvas(canvas_);
	canvas_->currentDocument()->addDrawingElement( line );

	oldPoint_.x = x;
	oldPoint_.y = y;
	canvas_->updateView();
}

void PenTool::endDraw( int x, int y ) {
	canvas_->endDocDrawing();
}

void PenTool::render( Painter* gr ) {

}


ImageEditor::CursorType PenTool::getCursor(int x, int y)
{
	return ctCross;
}

}