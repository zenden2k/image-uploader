#include "DrawingTool.h"
#include <math.h>
#include <cassert>
#include "BasicElements.h"
#include "Canvas.h"
#include "Document.h"
#include <Gdiplus.h>

void DebugWrite(const TCHAR* szFormat, ...)
{
	TCHAR szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsntprintf(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}

namespace ImageEditor {

using namespace Gdiplus;

AbstractDrawingTool::AbstractDrawingTool( Canvas *canvas ) {
	startPoint_.x = 0;
	startPoint_.y = 0;
	endPoint_.x   = 0;
	endPoint_.y   = 0;
	assert( canvas );
	canvas_ = canvas;
}

void AbstractDrawingTool::beginDraw( int x, int y ) {
	startPoint_.x = x;
	startPoint_.y = y;
}

void AbstractDrawingTool::endDraw( int x, int y ) {
	endPoint_.x = x;
	endPoint_.y = y;
}

Canvas::CursorType AbstractDrawingTool::getCursor()
{
	return Canvas::ctDefault;
}

VectorElementTool::VectorElementTool( Canvas* canvas, ElementType type ) : AbstractDrawingTool( canvas ) {
	currentElement_       = NULL;
	elementType_          = type;
}

void VectorElementTool::beginDraw( int x, int y ) {
	POINT pt = { x, y };
	createElement();
	currentElement_->setStartPoint( pt );
	currentElement_->setEndPoint( pt );
	canvas_->updateView();
}

void VectorElementTool::continueDraw( int x, int y, DWORD flags ) {
	POINT pt = { x, y };
	if ( currentElement_ ) {
		currentElement_->setEndPoint( pt );
	}
	canvas_->updateView();
}

void VectorElementTool::endDraw( int x, int y ) {
	POINT pt = { x, y };
	if ( currentElement_ ) {
		currentElement_->setEndPoint( pt );
	
		canvas_->currentDocument()->addDrawingElement( currentElement_ );
	}
	canvas_->updateView();
	currentElement_ = 0;
}

void VectorElementTool::render( Gdiplus::Graphics* gr ) {
	if ( currentElement_ ) {
		currentElement_->render( gr );
	}
}

void VectorElementTool::createElement() {
	delete currentElement_;
	switch( elementType_ ) {
		case etLine:
			currentElement_ = new Line( 0, 0, 0, 0 );
			break;
		case etRectangle:
			currentElement_ = new (ImageEditor::Rectangle)( 0, 0, 0, 0 );
			break;
	}
}

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
	Line * line =  new Line( oldPoint_.x, oldPoint_.y, x, y) ;

	line->setPenSize(1 );
	canvas_->currentDocument()->addDrawingElement( line );

	oldPoint_.x = x;
	oldPoint_.y = y;
	canvas_->updateView();

}

void PenTool::endDraw( int x, int y ) {
	canvas_->currentDocument()->endDrawing();
}

void PenTool::render( Gdiplus::Graphics* gr ) {

}


BrushTool::BrushTool( Canvas* canvas ) : AbstractDrawingTool( canvas ) {

}

void BrushTool::beginDraw( int x, int y ) {
	canvas_->currentDocument()->beginDrawing();
	oldPoint_.x = x;
	oldPoint_.y = y;
}

void BrushTool::continueDraw( int x, int y, DWORD flags ) {
	if ( flags & MK_CONTROL ) {
		y = oldPoint_.y;
	}
	drawLine( oldPoint_.x, oldPoint_.y, x, y) ;

	//line->setPenSize(25 );
	//_canvas->currentDocument()->addDrawingElement( line );

	oldPoint_.x = x;
	oldPoint_.y = y;
	canvas_->updateView();

}

void BrushTool::endDraw( int x, int y ) {
	canvas_->currentDocument()->endDrawing();
}

void BrushTool::render( Gdiplus::Graphics* gr ) {

}

void BrushTool::drawLine(int x0, int y0, int x1, int y1) {


	if ( y1 < y0 ) {
		std::swap( y0, y1 );
		std::swap( x0, x1 );
	}
	int xStart = min( x0, x1 )/*( min(x0, x1) / AffectedSegments::kSegmentSize +1 ) * AffectedSegments::kSegmentSize*/;
	int xEnd   = max( x0, x1 )/*( max(x0, x1) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize*/;

	int yStart = min( y0, y1 );
	int yEnd   = max( y0, y1 );
	/*x0 = startPoint_.x;
	x1 = endPoint_.x;
	y0 = startPoint_.y;
	y1 = endPoint_.y;*/
/*	segments->markPoint( x0, y0 );
	segments->markPoint( x1, y1 );*/

	Graphics *gr = canvas_->currentDocument()->getGraphicsObject();
	//SolidBrush br( Color( 0, 0, 0 ) );


	float len = sqrt(  pow((float)x1-x0, 2) + pow ((float) y1- y0, 2 ) );
	if ( ((int)len) == 0 ) {
		len = 1;
	}
	float sinA = ( x1 - x0 ) / len;
	float cosA = sqrt( 1 - sinA * sinA );

	//посчитать угол .. и ставить точки по линии
	//int 
	 DebugWrite( _T("len=%d sinA=%3.2f   sinB=%3.2f\r\n"), (int)len, sinA, cosA );
	 DebugWrite( _T("x0=%d  y0=%d  x1=%d y1=%d\r\n"), x0, y0, x1, y1 );
	float x = x0;
	float y = y0;
		SolidBrush br( Color( 255, 0, 0 ) );
	if ( x1 == x0 ) {
		//	MessageBox(0,0,0,0);
		for( int y = yStart; y <= yEnd; y++ ) {
			x = x0;
			gr->FillEllipse( &br, (int)x, y, 10, 10 );
			//segments->markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
		} 
	} else if ( y1 == y0 ) {
		for( int x = xStart; x <= xEnd; x++ ) {
			int y = y0;
			gr->FillEllipse( &br, x, y, 10, 10 );
		//	segments->markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
		} 
	} else {
		for( int a = 0; a <= len; a++ ) {
			x = x0 + a * sinA;
			y = y0 + a * cosA;
				
			DebugWrite( _T("a=%d x=%3.2f  y=%3.2f  \r\n"), a, x, y );
			//int y = -1;
			//y = y0 + (x-x0)*(y1-y0)/(x1-x0);
			gr->FillEllipse( &br, (int)x, (int)y, 10, 10 );
			//			segments->markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
		 } 
	}  
}




TextTool::TextTool( Canvas* canvas ) : AbstractDrawingTool( canvas ) {

}

void TextTool::beginDraw( int x, int y ) {
	AbstractDrawingTool::beginDraw( x, y );
	
}

void TextTool::continueDraw( int x, int y, DWORD flags ) {

}

void TextTool::endDraw( int x, int y ) {
	int xStart = min( startPoint_.x, x );
	int xEnd   = max( startPoint_.x, x );

	int yStart = min( startPoint_.y, y );
	int yEnd   = max( startPoint_.y, y );

	if ( xEnd - xStart < 50 ) {
		xEnd += 100;
	}

	if ( yEnd - yStart < 25 ) {
		yEnd += 25;
	}
	RECT inputRect = { xStart, yStart, xEnd, yEnd };
	canvas_->getInputBox( inputRect );
}

void TextTool::render( Gdiplus::Graphics* gr ) {

}

MovableElementTool::MovableElementTool( Canvas* canvas, MovableElement::ElementType type ) : AbstractDrawingTool( canvas ) {
	currentElement_       = NULL;
	elementType_          = type;
}

void MovableElementTool::beginDraw( int x, int y ) {
	POINT pt = { x, y };
	createElement();
	currentElement_->setStartPoint( pt );
	currentElement_->setEndPoint( pt );
	canvas_->updateView();
}

void MovableElementTool::continueDraw( int x, int y, DWORD flags ) {
	POINT pt = { x, y };
	if ( currentElement_ ) {
		currentElement_->setEndPoint( pt );
	}
	canvas_->updateView();
}

void MovableElementTool::endDraw( int x, int y ) {
	POINT pt = { x, y };
	if ( currentElement_ ) {
		currentElement_->setEndPoint( pt );

		canvas_->addMovableElement( currentElement_ );
	}
	canvas_->updateView();
	currentElement_ = 0;
}

void MovableElementTool::render( Gdiplus::Graphics* gr ) {
	if ( currentElement_ ) {
		currentElement_->render( gr );
	}
}

void MovableElementTool::createElement() {
	delete currentElement_;
	switch( elementType_ ) {
		case MovableElement::etArrow:
			//currentElement_ = new Line( 0, 0, 0, 0 );
			break;
		case MovableElement::etCrop:
			currentElement_ = new Crop( 0, 0, 0, 0 );
			break;
	}

}

MovableElementTool::BoundaryType MovableElementTool::checkElementsBoundaries()
{
	std::vector<MovableElement*> cropElements;
	canvas_->getElementsByType(MovableElement::etCrop, cropElements);
	int count = cropElements.size();
	for( int i  = 0; i< count; i++ ) {
		MovableElementTool::BoundaryType bt = checkElementBoundaries(cropElements[i]);
		if ( bt != btNone ) {
			return bt;
		}
	}

	return MovableElementTool::btNone;
}	

MovableElementTool::BoundaryType MovableElementTool::checkElementBoundaries(MovableElement* element)
{
	return btNone;
}

Canvas::CursorType MovableElementTool::getCursor()
{
	switch( elementType_ ) {
		case MovableElement::etArrow:
			//currentElement_ = new Line( 0, 0, 0, 0 );
			break;
		case MovableElement::etCrop:
			return Canvas::ctCross;
			break;
	}
	return Canvas::ctDefault;
}

}