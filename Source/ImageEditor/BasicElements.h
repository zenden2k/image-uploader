#ifndef IMAGEEDITOR_BASICELEMENTS_H
#define IMAGEEDITOR_BASICELEMENTS_H

#include <GdiPlus.h>
#include "DrawingElement.h"
#include "MovableElement.h"

namespace ImageEditor {

class Line: public DrawingElement {
	public:
		Line(int startX, int startY, int endX,int endY);
		void render(Painter* gr);
		void setEndPoint(POINT endPoint);
		void getAffectedSegments( AffectedSegments* segments );
		
};

class Rectangle: public DrawingElement {
	public:
		Rectangle(int startX, int startY, int endX,int endY);
		void render(Painter* gr);
		void getAffectedSegments( AffectedSegments* segments );
};

}

#endif