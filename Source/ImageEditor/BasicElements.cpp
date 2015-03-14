#include "BasicElements.h"

#include <algorithm>
#include <GdiPlus.h>
#include <Core/Logging.h>
namespace ImageEditor {

Line::Line(int startX, int startY, int endX, int endY) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
}


void Line::setEndPoint(POINT endPoint) {
	int kAccuracy = 7;
	if (abs (endPoint.y-startPoint_.y) < kAccuracy ) {
		endPoint.y = startPoint_.y;
	} else if (abs (endPoint.x-startPoint_.x) < kAccuracy ) {
		endPoint.x = startPoint_.x;
	}
	DrawingElement::setEndPoint( endPoint );
}

void Line::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	Gdiplus::Pen pen( Color( 10, 10, 10), penSize_ );
	gr->DrawLine( &pen, startPoint_.x, startPoint_.y, endPoint_.x, endPoint_.y );
}

void Line::getAffectedSegments( AffectedSegments* segments ) {
	int x0 = startPoint_.x;
	int y0 = startPoint_.y;
	int x1 = endPoint_.x;
	int y1 = endPoint_.y;
	 
	int xStart = ( min(x0, x1) / AffectedSegments::kSegmentSize +1 ) * AffectedSegments::kSegmentSize;
	int xEnd   = ( max(x0,x1) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize;

	x0 = startPoint_.x;
	x1 = endPoint_.x;
	y0 = startPoint_.y;
	y1 = endPoint_.y;
	segments->markPoint( x0, y0 );
	segments->markPoint( x1, y1 );
	
	for( int x = xStart; x <= xEnd; x += AffectedSegments::kSegmentSize ) {
		int y = -1;
		if ( x1-x0 != 0) {
			y = y0 + (x-x0)*(y1-y0)/(x1-x0);
		}
		
		segments->markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
	}

	int yStart = ( min( y0, y1 ) / AffectedSegments::kSegmentSize + 1) * AffectedSegments::kSegmentSize;
	int yEnd   = ( max( y0, y1 ) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize;

	for( int y = yStart; y <= yEnd; y += AffectedSegments::kSegmentSize ) {
		int x = -1;
		if ( y1-y0 != 0) {
			x = x0 + ( y - y0 ) * ( x1 - x0 ) / ( y1-y0 );
		}
		
		segments->markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
	}
}

// Rectangle
//
Rectangle::Rectangle(int startX, int startY, int endX, int endY) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
}

void Rectangle::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	Gdiplus::Pen pen( Color( 10, 10, 10) );
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
	gr->DrawRectangle( &pen, x, y, width, height );
}


void Rectangle::getAffectedSegments( AffectedSegments* segments ) {
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;

	//segments->markRect( x, y, width, height );
	segments->markRect( x, y, width, penSize_ ); // top
	segments->markRect( x, y, penSize_, height ); // left
	segments->markRect( x, y + height - penSize_, width, penSize_ ); // bottom
	segments->markRect( x + width - penSize_, y, penSize_, height ); // right*/
}

}
