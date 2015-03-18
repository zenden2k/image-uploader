#ifndef IMAGEEDITOR_DRAWINGELEMENT_H
#define IMAGEEDITOR_DRAWINGELEMENT_H

#include "BasicTypes.h"

#include <GdiPlus.h>
#include <map>
#include <deque>

namespace ImageEditor {

class Canvas;

class AffectedSegments;

class DrawingElement {
	public:
		DrawingElement();
		virtual ~DrawingElement() {};
		virtual void render(Painter* gr) = 0;
		virtual void resize(int width, int height);
		virtual void setStartPoint(POINT startPoint);
		virtual void setEndPoint(POINT endPoint);
		void setColor( Gdiplus::Color color );
		void setBackgroundColor( Gdiplus::Color color );
		Gdiplus::Color getColor() const;
		Gdiplus::Color getBackgroundColor() const;

		void setCanvas( Canvas* canvas );
		virtual void getAffectedSegments( AffectedSegments* segments );
		void setPenSize( int penSize );
		int getWidth();
		int getHeight();
	protected:
		POINT startPoint_;
		POINT endPoint_;
		Gdiplus::Color color_;
		Gdiplus::Color backgroundColor_;
		int penSize_;
		Canvas* canvas_;
};

class AffectedSegments {
	public:
		AffectedSegments();
		AffectedSegments(int maxWidth, int maxHeight);
		enum { kSegmentSize = 50 };
		void markPoint(int x, int y);
		void markRect(int x, int y, int width, int height);
		HRGN createRegionFromSegments();
		void clear();
		void getRects( std::deque<RECT>& rects, int maxWidth = 0, int maxHeight = 0) const;
		AffectedSegments& operator+= ( const AffectedSegments& segments_);
	private:
		std::map<unsigned int, bool> segments_;
		int maxWidth_;
		int maxHeight_;
};

}

#endif