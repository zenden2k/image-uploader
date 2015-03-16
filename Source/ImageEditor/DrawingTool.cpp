#include "DrawingTool.h"

#include "Canvas.h"
#include "Document.h"
#include "BasicElements.h"
#include "MovableElements.h"

#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>

#include <math.h>
#include <cassert>
#include <gdiplus.h>
#include <math.h>



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

CursorType AbstractDrawingTool::getCursor(int x, int y)
{
	return ctDefault;
}

VectorElementTool::VectorElementTool( Canvas* canvas, ElementType type ) : MoveAndResizeTool( canvas, type ) {
	currentElement_       = NULL;
}


ImageEditor::CursorType VectorElementTool::getCursor(int x, int y)
{
	return ctCross;
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
	Line * line =  new Line( canvas_, oldPoint_.x, oldPoint_.y, x, y) ;

	line->setPenSize(1 );
	line->setCanvas(canvas_);
	canvas_->currentDocument()->addDrawingElement( line );

	oldPoint_.x = x;
	oldPoint_.y = y;
	canvas_->updateView();

}

void PenTool::endDraw( int x, int y ) {
	canvas_->currentDocument()->endDrawing();
}

void PenTool::render( Painter* gr ) {

}


ImageEditor::CursorType PenTool::getCursor(int x, int y)
{
	return ctCross;
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

void BrushTool::render( Painter* gr ) {

}

ImageEditor::CursorType BrushTool::getCursor(int x, int y)
{
	return ctCross;
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




TextTool::TextTool( Canvas* canvas ) : MoveAndResizeTool( canvas ) {

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

	currentElement_ = new TextElement(canvas_, xStart,yStart, xEnd, yEnd);
	canvas_->addMovableElement(currentElement_);
		canvas_->getInputBox( inputRect );
	canvas_->updateView();

}

void TextTool::render( Painter* gr ) {

}

ImageEditor::CursorType TextTool::getCursor(int x, int y)
{
	return ctEdit;
}

CropOverlay* MoveAndResizeTool::cropOverlay_ = 0;

MoveAndResizeTool::MoveAndResizeTool( Canvas* canvas, ElementType type ) : AbstractDrawingTool( canvas ) {
	currentElement_       = NULL;
	elementType_          = type;
	draggedBoundary_ = btNone;
	isMoving_ = false;
}

void MoveAndResizeTool::beginDraw( int x, int y ) {
	draggedBoundary_ = checkElementsBoundaries(x,y, &currentElement_);
	if ( draggedBoundary_!= btNone ) {
		return;
	}
	MovableElement* el = canvas_->getElementAtPosition(x,y);
	if ( el && ( elementType_ == el->getType() ||  ( elementType_== etNone  && el->getType()  != etCrop )) ) {
		//currentElement_->setSelected(true);
		canvas_->unselectAllElements();
		el->setSelected(true);
		isMoving_ = true;
		LOG(INFO) << "Starting moving!";
		currentElement_ = el;
		startPoint_.x = x;
		startPoint_.y = y;
		canvas_->updateView();
		return;
	}
	if ( elementType_== etCrop  ) {
		canvas_->deleteElementsByType(elementType_);
	}
	canvas_->unselectAllElements();
	POINT pt = { x, y };
	createElement();
	
	if ( currentElement_ ) {
		startPoint_.x = x;
		startPoint_.y = y;
		currentElement_->setStartPoint( pt );
		currentElement_->setEndPoint( pt );
		canvas_->addMovableElement( currentElement_ );
		
	}
	canvas_->updateView();
	
}

void MoveAndResizeTool::continueDraw( int x, int y, DWORD flags ) {

	if ( currentElement_ && draggedBoundary_!= btNone ) {
		int elWidth = currentElement_->getWidth();
		int elHeight = currentElement_->getHeight();
		int elX = currentElement_->getX();
		int elY  = currentElement_->getY();
		switch ( draggedBoundary_ ) {
			case btBottomRight:
				elWidth = x - elX;
				elHeight = y - elY;
				break;
			case btBottom:
				elHeight = y - elY;
				break;
			case btRight:
				elWidth = x - elX;
				break;
			case btTopLeft:
				
				elWidth =  elX - x + elWidth;
				elHeight = elY - y + elHeight;
				elX = x;
				elY = y;
			case btLeft:
				
				elWidth = elX - x + elWidth;
				elX = x;
				break;
			case btTop:
				elHeight =  elY - y + elHeight;
				elY = y;
				break;
			case btBottomLeft:
				
				elWidth = elX - x + elWidth;
				elHeight = y - elY;
				elX = x;
				break;
			case btTopRight:
				
				elWidth =  x - elX;
				elHeight = elY - y + elHeight;
				elY = y;

				//currentElement_->setEndPoint()
		}
		//LOG(INFO) << "x=" << x << " y="<<y<<" object.x = "<<currentElement_->getX()<<" object.y = "<< currentElement_->getY();
		LOG(INFO) << "Resizing object to " << elWidth << " "<<elHeight;
		currentElement_->resize( elWidth,elHeight);
		currentElement_->setX(elX);
		currentElement_->setY(elY);
		if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		canvas_->updateView();
		return;
	}
	
	if ( isMoving_ && currentElement_ ) {
		int newX = currentElement_->getX() + x - startPoint_.x;
		int newY  = currentElement_->getY() + y - startPoint_.y;
		LOG(INFO) << "Moving object to new position " << newX << " "<<newY;
		currentElement_->setX(newX);
		currentElement_->setY(newY);
		startPoint_.x = x;
		startPoint_.y = y;
		if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		canvas_->updateView();

		return;
	}

	if ( currentElement_ ) {
		POINT pt = { x, y };
		currentElement_->setEndPoint( pt );
		/*if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}*/
		canvas_->updateView();
	}
}

void MoveAndResizeTool::endDraw( int x, int y ) {
	if ( currentElement_ ) {
		if ( currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		currentElement_= 0;
		//currentElement_->setSelected(true);
	}
	if ( draggedBoundary_!= btNone ) {
		
		draggedBoundary_ = btNone;
		return;
	}
	if ( isMoving_ ) {
		isMoving_ = false;
		return;
	}
}

void MoveAndResizeTool::render( Painter* gr ) {
	if ( currentElement_ ) {
		currentElement_->render( gr );
	}
}

void MoveAndResizeTool::createElement() {
	delete currentElement_;
	currentElement_ = 0;
	switch( elementType_ ) {
		case etArrow:
			//currentElement_ = new Line( 0, 0, 0, 0 );
			break;
		case etLine:
			currentElement_ = new Line(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etCrop:
			if ( !cropOverlay_ ) {
				cropOverlay_ = new CropOverlay(canvas_, 0,0, canvas_->getWidth(), canvas_->getHeigth());
				
				atexit(&cleanUp);
			}
			canvas_->setOverlay(cropOverlay_);
			currentElement_ = new Crop(canvas_, 0, 0, 0, 0 );
			break;
		case etRectangle:
			currentElement_ = new Rectangle(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
	}

}

BoundaryType MoveAndResizeTool::checkElementsBoundaries( int x, int y, MovableElement** elem)
{
	std::vector<MovableElement*> cropElements;
	canvas_->getElementsByType(elementType_, cropElements);
	int count = cropElements.size();
	for( int i  = 0; i< count; i++ ) {
		BoundaryType bt = checkElementBoundaries(cropElements[i], x , y);
		if ( bt != btNone ) {
			if ( elem ) {
				*elem = cropElements[i];
			}
			return bt;
		}
	}

	return btNone;
}	

BoundaryType MoveAndResizeTool::checkElementBoundaries(MovableElement* element, int x, int y)
{
	for ( int i = 0 ; i < element->grips_.size(); i++ ) {
		if ( abs (x - element->grips_[i].pt.x) <= MovableElement::kGripSize+2 &&  abs (y - element->grips_[i].pt.y) <= MovableElement::kGripSize+2 ) {
			return element->grips_[i].bt;
		}
	}
	return btNone;
}

void MoveAndResizeTool::cleanUp()
{
	delete cropOverlay_;
}

CursorType MoveAndResizeTool::getCursor(int x, int y)
{
	CursorType  ct = ctDefault;
	
	switch( elementType_ ) {
		case etArrow:
			//currentElement_ = new Line( 0, 0, 0, 0 );
			break;
		case etCrop:
			ct = ctCross;
			break;
	}
	BoundaryType bt = checkElementsBoundaries( x, y); 
	if ( bt != btNone ) {
		return MovableElement::GetCursorForBoundary(bt);
	}
	MovableElement* el = canvas_->getElementAtPosition(x,y);
	if ( el &&  (el->getType() != etCrop || elementType_ == etCrop)  ) {

		return ctMove;
	}
	return ct;
}

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
			canvas_->setOverlay(0);
		}
		canvas_->updateView();
		currentElement_ = 0;
	}
}

}