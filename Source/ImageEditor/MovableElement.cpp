#include "MovableElement.h"

#include <GdiPlus.h>
#include "Canvas.h"

namespace ImageEditor {
using namespace Gdiplus;
MovableElement::MovableElement(){
	startPoint_.x = 0;
	startPoint_.y = 0;
	endPoint_.x   = 0;
	endPoint_.y   = 0;

	color_ = Gdiplus::Color( 0, 0, 0 );
	penSize_ = 1;
	isSelected_ = true;
	memset(grips_, 0, sizeof(grips_));
}


void MovableElement::render(Painter* gr)
{
	

}

void MovableElement::renderGrips(Painter* gr)
{
	Gdiplus::Pen pen( Color( 0, 0, 0) );
	//Gdiplus::SolidBrush brush(Color( 255, 255, 255) );
	pen.SetDashStyle(DashStyleDash);
	REAL dashValues[2] = {4, 4};
	pen.SetDashPattern(dashValues, 2);
	//pen.SetBrush(&brush);
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
	gr->DrawRectangle( &pen, x, y, width, height );
	pen.SetColor(Color( 255, 255, 255) );
	pen.SetDashOffset(3);
	REAL dashValues2[2] = {4, 4};
	pen.SetDashPattern(dashValues2, 2);	
	gr->DrawRectangle( &pen, x, y, width, height );

	if ( isSelected_ ) {
		int rectSize = kGripSize;
		int halfSize = rectSize /2 ;
		Gdiplus::Pen pen( Color( 255,255, 255) );
		//pen.SetDashStyle(DashStyleDash);
		int x = std::min<>( startPoint_.x, endPoint_.x );
		int y = std::min<>( startPoint_.y, endPoint_.y );
		int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
		int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
		Gdiplus::SolidBrush brush(Color( 10, 10, 10) );

		POINT pts[8] = {{x,y}, {x + width / 2, y}, {x+width,y}, {x+width,y+height/2}, 
		{x+width,y+height},  {x+width/2, y+height}, {x,y+height},{x,y+height/2}	
		};
		memcpy(grips_,pts,sizeof(grips_));
		for( int i = 0; i < 8; i++ ) {
			int x = pts[i].x;
			int y = pts[i].y;
			gr->FillRectangle( &brush, x-halfSize, y-halfSize, rectSize, rectSize );
			gr->DrawRectangle( &pen, x-halfSize-1, y-halfSize-1, rectSize+1, rectSize+1 );
		}

	}
}

void MovableElement::setSelected(bool selected)
{
	isSelected_ = selected;
}

bool MovableElement::isSelected() const
{
	return isSelected_;
}

ElementType MovableElement::getType() const
{
	return etUnknown;
}

CursorType MovableElement::GetCursorForBoundary(BoundaryType bt)
{
	switch (bt ) {
		case btTopLeft:
		case btBottomRight:
			return ctResizeDiagonalMain;
		case btTopRight:
		case btBottomLeft:
			return ctResizeDiagonalAnti;
		case btTop:
		case btBottom:
			return ctResizeVertical;
		case btLeft:
		case btRight:
			return ctResizeHorizontal;
		default:
			return ctDefault;
	}
}

int MovableElement::getX() 
{
	return getMinPoint(axisX)->x ;
}

int MovableElement::getY() 
{
	return getMinPoint(axisY)->y;
}

void MovableElement::setX(int x)
{
	int width = getWidth();
	int canvasWidth = canvas_->getWidth();
	if ( x <= 0 ) {
		x = 0;
	} else if ( x + width  >  canvasWidth) {
		x = canvasWidth - width;
	}
	
	getMinPoint(axisX)->x = x;
	resize(width, getHeight());
}

void MovableElement::setY(int y)
{
	int canvasHeight = canvas_->getHeigth();
	int height = getHeight();

	if( y < 0 ) {
		y = 0;
	} else if ( y + height  > canvasHeight ) {
		y = canvasHeight - height;
	}
	
	getMinPoint(axisY)->y = y;
	resize(getWidth(), height );
}

void MovableElement::resize(int width, int height)
{
	if ( width  < 1 ) {
		width = 1;
	}
	if ( height < 1 ) {
		height  = 1;
	}
	getMaxPoint(axisX)->x = width + getX();	
	getMaxPoint(axisY)->y = height + getY();
}

POINT* MovableElement::getMaxPoint(Axis axis)
{
	if ( axis == axisX ) {
		return endPoint_.x > startPoint_.x ? &endPoint_ : &startPoint_;
	} else {
		return endPoint_.y > startPoint_.y ? &endPoint_ : &startPoint_;
	}
}

POINT* MovableElement::getMinPoint(Axis axis)
{
	if ( axis == axisX ) {
		return startPoint_.x < endPoint_.x ? &startPoint_ : &endPoint_;
	} else {
		return startPoint_.y < endPoint_.y ? &startPoint_ : &endPoint_;
	}
}

}
