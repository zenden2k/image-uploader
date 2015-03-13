#include "MovableElement.h"

#include <GdiPlus.h>

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
}


void MovableElement::render(Gdiplus::Graphics* gr)
{
	Gdiplus::Pen pen( Color( 10, 10, 10) );
	pen.SetDashStyle(DashStyleDash);
	REAL dashValues[2] = {5, 3};
	pen.SetDashPattern(dashValues, 2);
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
	gr->DrawRectangle( &pen, x, y, width, height );
	
}

void MovableElement::setSelected(bool selected)
{
	isSelected_ = selected;
}

bool MovableElement::isSelected() const
{
	return isSelected_;
}

MovableElement::ElementType MovableElement::getType() const
{
	return etUnknown;
}

}
