#ifndef IMAGEEDITOR_MOVABLEELEMENTS_H
#define IMAGEEDITOR_MOVABLEELEMENTS_H

#include <GdiPlus.h>
#include "DrawingElement.h"
#include "MovableElement.h"

namespace ImageEditor {
class InputBox;
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
		TextElement( Canvas* canvas, InputBox* inputBox, int startX, int startY, int endX,int endY );
		void render(Painter* gr);
		void getAffectedSegments( AffectedSegments* segments );
		virtual void resize(int width, int height);
		void setInputBox(InputBox* inputBox);
		InputBox* getInputBox() const;
		virtual ElementType getType() const;
		virtual void setSelected(bool selected);
protected:
	InputBox *inputBox_;
	void onTextChanged(TCHAR *text);
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
#if GDIPVER >= 0x0110 
class BlurringRectangle: public MovableElement {
public:
	BlurringRectangle(Canvas* canvas, int startX, int startY, int endX,int endY);
	void render(Painter* gr);
};
#endif
class Rectangle: public MovableElement {
public:
	Rectangle(Canvas* canvas, int startX, int startY, int endX,int endY,bool filled = false );
	void render(Painter* gr);
	void getAffectedSegments( AffectedSegments* segments );

	virtual bool isItemAtPos(int x, int y);
protected:
	bool filled_;

};

class FilledRectangle: public Rectangle {
public:
	FilledRectangle(Canvas* canvas, int startX, int startY, int endX,int endY );
};

class Arrow: public Line {
public:
	Arrow(Canvas* canvas,int startX, int startY, int endX,int endY);
	void render(Painter* gr);


	//virtual void createGrips();

	//virtual bool isItemAtPos(int x, int y);

};

class Selection: public MovableElement {
public:
	Selection(Canvas* canvas, int startX, int startY, int endX,int endY);
	void render(Painter* gr);

	virtual ElementType getType() const;

};

}

#endif