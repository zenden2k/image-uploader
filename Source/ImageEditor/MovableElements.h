#ifndef IMAGEEDITOR_MOVABLEELEMENTS_H
#define IMAGEEDITOR_MOVABLEELEMENTS_H

#include <GdiPlus.h>
#include "DrawingElement.h"
#include "MovableElement.h"

namespace ImageEditor {

class TextElement: public DrawingElement {
	public:
		TextElement( int startX, int startY, int endX,int endY );
		void render(Painter* gr);
		void getAffectedSegments( AffectedSegments* segments );

};

class Crop: public MovableElement {
public:

	Crop(int startX, int startY, int endX,int endY);
	void render(Painter* gr);
	void getAffectedSegments( AffectedSegments* segments );

	virtual ElementType getType() const;

};

class CropOverlay: public MovableElement {
public:
	CropOverlay(int startX, int startY, int endX,int endY);
	void render(Painter* gr);
};

}

#endif