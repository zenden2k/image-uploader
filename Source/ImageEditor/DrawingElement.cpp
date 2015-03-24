#include "DrawingElement.h"

#include <GdiPlus.h>
#include <Core/Logging.h>
namespace ImageEditor {

DrawingElement::DrawingElement(){
	startPoint_.x = 0;
	startPoint_.y = 0;
	endPoint_.x   = 0;
	endPoint_.y   = 0;
	color_ = Gdiplus::Color( 0, 0, 0 );
	penSize_ = 1;
}

void DrawingElement::resize(int width, int height) {
	LOG(ERROR) << "Not implemented";
	//dimensions_ = newSize;
}

void DrawingElement::setStartPoint(POINT startPoint) {
	startPoint_ = startPoint;
}

void DrawingElement::setEndPoint(POINT endPoint) {
	endPoint_ = endPoint; 
}

POINT DrawingElement::getStartPoint() const
{	
	return startPoint_;
}

POINT DrawingElement::getEndPoint() const
{
	return endPoint_;
}

void DrawingElement::setColor( Gdiplus::Color color ) {
	color_ = color;
}

void DrawingElement::setBackgroundColor(Gdiplus::Color color)
{
	backgroundColor_ = color;
}

Gdiplus::Color DrawingElement::getColor() const
{
	return color_;
}

Gdiplus::Color DrawingElement::getBackgroundColor() const
{
	return backgroundColor_;
}

void DrawingElement::setCanvas(Canvas* canvas)
{
	canvas_ = canvas;
}

void DrawingElement::setPenSize( int penSize ) {
	penSize_ = penSize;
}

int DrawingElement::getWidth()
{
	return abs(endPoint_.x -startPoint_.x)+1;
}

int DrawingElement::getHeight()
{
	return abs(endPoint_.y -startPoint_.y)+1;
}

void DrawingElement::getAffectedSegments( AffectedSegments* segments ) {
	segments->markPoint( startPoint_.x, startPoint_.y );
	segments->markPoint( endPoint_.x, endPoint_.y );
}

AffectedSegments::AffectedSegments()
{
	maxWidth_ = -1;
	maxHeight_ = -1;
}

AffectedSegments::AffectedSegments(int maxWidth, int maxHeight)
{
	maxWidth_ = maxWidth;
	maxHeight_ = maxHeight;
}

void AffectedSegments::markPoint(int x, int y) {
	if ( x >= maxWidth_ || y >= maxHeight_ || x < 0 || y < 0) {
		return;
	}

	int horSegmentIndex = x / kSegmentSize;
	int vertSegmentIndex = y / kSegmentSize;
	unsigned int mapIndex = MAKELONG( horSegmentIndex, vertSegmentIndex );
	segments_[mapIndex] = true;
}

void AffectedSegments::markRect(int x, int y, int width, int height) {
	if ( width < 1 || height <1  ) {
		return;
	}
	if ( x < 0 ) {
		width += x;
		x = 0;
	}

	if ( y < 0) {
		height+= y;
		y = 0;
	}

	if ( maxWidth_ != -1 ) {
		width  = min( width, maxWidth_ - x);
	}
	if ( maxHeight_ != -1 ) {
		height = min ( height , maxHeight_ - y);
	}

	if (  width < 1 || height <1  ) {
		return;
	}

	int xFirstSegmentIndex = x / kSegmentSize;
	int yFirstSegmentIndex = y / kSegmentSize;
	int xLastSegmentIndex  = ( x + width ) / kSegmentSize;
	int yLastSegmentIndex  = ( y + height ) / kSegmentSize;

	for ( int i = xFirstSegmentIndex; i <= xLastSegmentIndex; i++ ) {
		for (int j = yFirstSegmentIndex; j <= yLastSegmentIndex; j++) {
			segments_ [ MAKELONG( i, j ) ] = true;
		}
	}
}

void AffectedSegments::markRect(RECT rc)
{
	markRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

HRGN AffectedSegments::createRegionFromSegments() {
	typedef std::map<unsigned int, bool>::iterator iter;
	CRgn rgn;
	rgn.CreateRectRgn(0, 0, 0, 0);

	for( iter i = segments_.begin(); i != segments_.end(); ++i ) {
		CRgn tempRegion;
		unsigned int index = i->first;
		int xIndex = LOWORD( index );
		int yIndex = HIWORD( index );
		tempRegion.CreateRectRgn( xIndex * kSegmentSize, yIndex * kSegmentSize, (xIndex + 1) * kSegmentSize, (yIndex + 1) * kSegmentSize );
		rgn.CombineRgn( tempRegion, RGN_OR );
	}
	return rgn.Detach();
}

void AffectedSegments::getRects( std::deque<RECT>& rects, int maxWidth, int maxHeight) const {
	typedef std::map<unsigned int, bool>::const_iterator iter;
	RECT bounds = { 0, 0, maxWidth, maxHeight };
	bool checkBounds = maxWidth && maxHeight;
	for( iter i = segments_.begin(); i != segments_.end(); ++i ) {
		CRgn tempRegion;
		unsigned int index = i->first;
		int xIndex = LOWORD( index );
		int yIndex = HIWORD( index );
		RECT rc = { xIndex * kSegmentSize, yIndex * kSegmentSize, (xIndex + 1) * kSegmentSize, (yIndex + 1) * kSegmentSize };
		if ( checkBounds ) {
			IntersectRect( &rc, &bounds, &rc );
		}
		rects.push_back( rc );
	}
}

AffectedSegments& AffectedSegments::operator+= ( const AffectedSegments& segments ) {
	typedef std::map<unsigned int, bool>::const_iterator iter;
	for( iter i = segments.segments_.begin(); i != segments.segments_.end(); ++i ) {
		segments_[ i->first ] = true;
			//segments.segments_[ i->first ];
	}
	return *this;
}

void AffectedSegments::clear() {
	segments_.clear();
}

}
