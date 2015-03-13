#ifndef IMAGEEDITOR_DRAWINGTOOL_H
#define IMAGEEDITOR_DRAWINGTOOL_H

#include <Gdiplus.h>
#include "Canvas.h"
#include "DrawingElement.h"
#include "MovableElement.h"

namespace ImageEditor {

class Canvas;

class AbstractDrawingTool {
	public:
		AbstractDrawingTool( Canvas* canvas);
		virtual ~AbstractDrawingTool(){};
		virtual void beginDraw( int x, int y );
		virtual void continueDraw( int x, int y, DWORD flags ) = NULL;
		virtual void endDraw( int x, int y );
		virtual void render( Gdiplus::Graphics* gr ) = NULL;
		virtual Canvas::CursorType getCursor();
	protected:
		Canvas* canvas_;
		POINT startPoint_;
		POINT endPoint_;
};

class VectorElementTool: public AbstractDrawingTool {
	public:
		enum ElementType { etLine, etRectangle };
		VectorElementTool( Canvas* canvas, ElementType type );
		void beginDraw( int x, int y );
		void continueDraw( int x, int y, DWORD flags );
		void endDraw( int x, int y );
		void render( Gdiplus::Graphics* gr );
		
	private:
		DrawingElement* currentElement_;
		ElementType elementType_;
		void createElement();
};

class MovableElementTool: public AbstractDrawingTool {
	public:
		enum BoundaryType { btNone,btTopLeft, btTop, brTopRight, btLeft, btRight, btBottomLeft, btBottom, btBotttomRight};
		MovableElementTool( Canvas* canvas, MovableElement::ElementType type );
		void beginDraw( int x, int y );
		void continueDraw( int x, int y, DWORD flags );
		void endDraw( int x, int y );
		void render( Gdiplus::Graphics* gr );
		
	private:
		MovableElement* currentElement_;
		MovableElement::ElementType elementType_;
		void createElement();
		BoundaryType checkElementsBoundaries();
		BoundaryType checkElementBoundaries(MovableElement*);

		virtual Canvas::CursorType getCursor();

};

class PenTool: public AbstractDrawingTool  {
	public:
		PenTool( Canvas* canvas );
		void beginDraw( int x, int y );
		void continueDraw( int x, int y, DWORD flags = 0);
		void endDraw( int x, int y );
		void render( Gdiplus::Graphics* gr );
	private:
		POINT oldPoint_;
};

class BrushTool: public AbstractDrawingTool  {
public:
	BrushTool( Canvas* canvas );
	void beginDraw( int x, int y );
	void continueDraw( int x, int y, DWORD flags = 0);
	void endDraw( int x, int y );
	void render( Gdiplus::Graphics* gr );
private:
	POINT oldPoint_;
	void drawLine(int x0, int y0, int x1, int y1);
};

class TextTool: public AbstractDrawingTool  {
public:
	TextTool( Canvas* canvas );
	void beginDraw( int x, int y );
	void continueDraw( int x, int y, DWORD flags = 0);
	void endDraw( int x, int y );
	void render( Gdiplus::Graphics* gr );
private:
	
};



};

#endif