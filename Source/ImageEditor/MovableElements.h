#ifndef IMAGEEDITOR_MOVABLEELEMENTS_H
#define IMAGEEDITOR_MOVABLEELEMENTS_H

#include <GdiPlus.h>
#include "DrawingElement.h"
#include "MovableElement.h"

namespace ImageEditor {

class Line: public MovableElement {
	public:
		Line(Canvas* canvas,int startX, int startY, int endX,int endY);
		void render(Painter* gr);
		void setEndPoint(POINT endPoint);
		void getAffectedSegments( AffectedSegments* segments );

		virtual void createGrips();

		virtual bool isItemAtPos(int x, int y);

};

class TextElement: public MovableElement{
	public:
		TextElement( Canvas* canvas, int startX, int startY, int endX,int endY );
		void render(Painter* gr);
		void getAffectedSegments( AffectedSegments* segments );

};

class Crop: public MovableElement {
public:

	Crop(Canvas* canvas, int startX, int startY, int endX,int endY);
	void render(Painter* gr);
	void getAffectedSegments( AffectedSegments* segments );

	virtual ElementType getType() const;

};

class CropOverlay: public MovableElement {
public:
	CropOverlay(Canvas* canvas, int startX, int startY, int endX,int endY);
	void render(Painter* gr);
};

class Rectangle: public MovableElement {
public:
	Rectangle(Canvas* canvas, int startX, int startY, int endX,int endY);
	void render(Painter* gr);
	void getAffectedSegments( AffectedSegments* segments );

	virtual bool isItemAtPos(int x, int y);

};
}

#endif