#ifndef IMAGEEDITOR_BASICELEMENTS_H
#define IMAGEEDITOR_BASICELEMENTS_H

#include <GdiPlus.h>
#include "DrawingElement.h"
#include "MovableElement.h"

namespace ImageEditor {

class Line: public DrawingElement {
	public:
		Line(int startX, int startY, int endX,int endY);
		void render(Gdiplus::Graphics* gr);
		void resize(Gdiplus::Rect newSize);
		void setEndPoint(POINT endPoint);
		void getAffectedSegments( AffectedSegments* segments );
		
};

class Rectangle: public DrawingElement {
	public:
		Rectangle(int startX, int startY, int endX,int endY);
		void render(Gdiplus::Graphics* gr);
		void resize(Gdiplus::Rect newSize);
		void getAffectedSegments( AffectedSegments* segments );
};

class TextElement: public DrawingElement {
	public:
		TextElement( int startX, int startY, int endX,int endY );
		void render(Gdiplus::Graphics* gr);
		void resize(Gdiplus::Rect newSize);
		void getAffectedSegments( AffectedSegments* segments );

};

class Crop: public MovableElement {
public:
	Crop(int startX, int startY, int endX,int endY);
	void render(Gdiplus::Graphics* gr);
	void resize(Gdiplus::Rect newSize);
	void getAffectedSegments( AffectedSegments* segments );
};

}

#endif