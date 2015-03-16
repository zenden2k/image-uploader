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
		virtual void render( Painter* gr ) = NULL;
		virtual CursorType getCursor(int x, int y);
	protected:
		Canvas* canvas_;
		POINT startPoint_;
		POINT endPoint_;
};


class CropOverlay;

class MoveAndResizeTool: public AbstractDrawingTool {
	public:

		MoveAndResizeTool( Canvas* canvas, ElementType type = etNone );
		void beginDraw( int x, int y );
		void continueDraw( int x, int y, DWORD flags );
		void endDraw( int x, int y );
		void render( Painter* gr );
		
	protected:
		MovableElement* currentElement_;
		ElementType elementType_;
		BoundaryType draggedBoundary_;
		void createElement();
		BoundaryType checkElementsBoundaries(int x, int y, MovableElement** elem = 0);
		BoundaryType checkElementBoundaries(MovableElement*, int x, int y);
		static void cleanUp();
		virtual CursorType getCursor(int x, int y);
		static CropOverlay* cropOverlay_;
		bool isMoving_;

};

class VectorElementTool: public MoveAndResizeTool {
public:
	VectorElementTool( Canvas* canvas, ElementType type );

	virtual CursorType getCursor(int x, int y);
};

class CropTool : public MoveAndResizeTool {
public:
	CropTool(Canvas* canvas);
	void beginDraw( int x, int y );
	void continueDraw( int x, int y, DWORD flags );
	void endDraw( int x, int y );
};

class PenTool: public AbstractDrawingTool  {
	public:
		PenTool( Canvas* canvas );
		void beginDraw( int x, int y );
		void continueDraw( int x, int y, DWORD flags = 0);
		void endDraw( int x, int y );
		void render( Painter* gr );
		virtual CursorType getCursor(int x, int y);
	private:
		POINT oldPoint_;
};

class BrushTool: public AbstractDrawingTool  {
public:
	BrushTool( Canvas* canvas );
	void beginDraw( int x, int y );
	void continueDraw( int x, int y, DWORD flags = 0);
	void endDraw( int x, int y );
	void render( Painter* gr );
	virtual CursorType getCursor(int x, int y);
private:
	POINT oldPoint_;
	void drawLine(int x0, int y0, int x1, int y1);
};

class TextTool: public MoveAndResizeTool  {
public:
	TextTool( Canvas* canvas );
	void beginDraw( int x, int y );
	void continueDraw( int x, int y, DWORD flags = 0);
	void endDraw( int x, int y );
	void render( Painter* gr );
	virtual CursorType getCursor(int x, int y);
private:
	
};



};

#endif