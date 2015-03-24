#include "MovableElement.h"

#include <GdiPlus.h>
#include "Canvas.h"

namespace ImageEditor {

using namespace Gdiplus;

MovableElement::MovableElement(Canvas* canvas){
	drawDashedRectangle_ = true;
	startPoint_.x = 0;
	startPoint_.y = 0;
	endPoint_.x   = 0;
	endPoint_.y   = 0;

	color_ = Gdiplus::Color( 0, 0, 0 );
	penSize_ = 1;
	isSelected_ = false;
	
	grips_.resize(8);
	canvas_ = canvas;
	isMoving_ = false;
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
	int x = getX();
	int y = getY();
	int width = getWidth()-1;
	int height = getHeight()-1;
	if (  /*isSelected_ || */drawDashedRectangle_ ) {
		gr->DrawRectangle( &pen, x, y, width, height );
		pen.SetColor(Color( 255, 255, 255) );
		pen.SetDashOffset(3);
		REAL dashValues2[2] = {4, 4};
		pen.SetDashPattern(dashValues2, 2);	
		gr->DrawRectangle( &pen, x, y, width, height );
	}

	if ( isSelected_ || getType() == etCrop ) {
		int rectSize = kGripSize;
		int halfSize = rectSize /2 ;
		Gdiplus::Pen pen( Color( 255,255, 255) );
		//pen.SetDashStyle(DashStyleDash);
		/*int x = getX();
		int y = getY();
		int width = getWidth();
		int height = getHeight();*/
		Gdiplus::SolidBrush brush(Color( 10, 10, 10) );

		createGrips();

		
		for( int i = 0; i < grips_.size(); i++ ) {
			int x = grips_[i].pt.x;
			int y = grips_[i].pt.y;
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

void MovableElement::setDrawDashedRectangle(bool draw)
{
	drawDashedRectangle_ = draw;
}

int MovableElement::getX() 
{
	return getMinPoint(axisX)->x ;
}

int MovableElement::getY() 
{
	return getMinPoint(axisY)->y;
}

RECT MovableElement::getPaintBoundingRect()
{
	int radius = penSize_;
	if ( canvas_->hasBlurRectangles() ) {
		radius += canvas_->getBlurRadius();
	}
	RECT res = { getX()-radius, getY()-radius, getWidth()+radius*2, getHeight()+radius*2};
	res.right += res.left;
	res.bottom += res.top;
	return res;
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

bool MovableElement::isItemAtPos(int x, int y)
{
	int elementX = getX();
	int elementY = getY();
	int elementWidth = getWidth();
	int elementHeight = getHeight();
	return  ( x+kSelectRadius >= elementX && x  <= elementX + elementWidth + kSelectRadius && y+kSelectRadius>= elementY && y <= elementY + elementHeight+kSelectRadius );
}

void MovableElement::resize(int width, int height)
{
	if ( width  < 1 ) {
		width = 1;
	}
	if ( height < 1 ) {
		height  = 1;
	}
	getMaxPoint(axisX)->x = width-1 + getX();	
	getMaxPoint(axisY)->y = height-1 + getY();
}

void MovableElement::createGrips()
{
	int rectSize = kGripSize;
	int halfSize = rectSize /2 ;
	
	int x = getX();
	int y = getY();
	int width = getWidth()-1;
	int height = getHeight()-1;

	// item order is important!!!! FIXME: use std::map
	POINT pts[8] = {
	{x+width,y+height},  {x+width/2, y+height}, {x,y+height},{x+width,y+height/2}, {x,y+height/2}, {x,y}, {x + width / 2, y}, {x+width,y}, 
	};

	for( int i = 0; i < 8; i++ ) {
		int x = pts[i].x;
		int y = pts[i].y;
		Grip grip;
		grip.pt.x = x;
		grip.pt.y = y;
		if ( x == startPoint_.x && y == startPoint_.y ) {
			grip.gpt = gptStartPoint;
		} else if ( x == endPoint_.x &&   y == endPoint_.y ) {
			grip.gpt = gptEndPoint;
		}
		grip.bt = static_cast<BoundaryType>(i);
		grips_[i] = grip;
	}
}

void MovableElement::beginMove()
{
	isMoving_ = true;
}

void MovableElement::endMove()
{
	isMoving_ = false;
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
