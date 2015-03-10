#ifndef IMAGEEDITOR_DRAWINGELEMENT_H
#define IMAGEEDITOR_DRAWINGELEMENT_H

#include <GdiPlus.h>
#include <map>
#include <deque>
namespace ImageEditor {

class AffectedSegments;

class DrawingElement {
	public:
		DrawingElement();
		virtual ~DrawingElement() {};
		virtual void render(Gdiplus::Graphics* gr) = 0;
		virtual void resize(Gdiplus::Rect newSize);
		virtual void setStartPoint(POINT startPoint);
		virtual void setEndPoint(POINT endPoint);
		void setColor( Gdiplus::Color color );
		virtual void getAffectedSegments( AffectedSegments* segments );
		void setPenSize( int penSize );
	protected:
		Gdiplus::Rect dimensions_;
		POINT startPoint_;
		POINT endPoint_;
		Gdiplus::Color color_;
		int penSize_;
};

class AffectedSegments {
	public:
		enum { kSegmentSize = 25 };
		void markPoint(int x, int y);
		void markRect(int x, int y, int width, int height);
		HRGN createRegionFromSegments();
		void clear();
		void getRects( std::deque<RECT>& rects, int maxWidth = 0, int maxHeight = 0) const;
		AffectedSegments& operator+= ( const AffectedSegments& segments_);
	private:
		std::map<unsigned int, bool> segments_;
};

}

#endif