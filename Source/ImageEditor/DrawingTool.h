#ifndef IMAGEEDITOR_DRAWINGTOOL_H
#define IMAGEEDITOR_DRAWINGTOOL_H

#include <Gdiplus.h>
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
		virtual void mouseDoubleClick( int x, int y );
		virtual CursorType getCursor(int x, int y);
		void setPenSize(int size);
		void setForegroundColor(Gdiplus::Color color);
		void setBackgroundColor(Gdiplus::Color color);
	protected:
		Canvas* canvas_;
		POINT startPoint_;
		POINT endPoint_;
		int penSize_;
		Gdiplus::Color foregroundColor_;
		Gdiplus::Color backgroundColor_;
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
		MovableElement::Grip draggedBoundary_;
		void createElement();
		MovableElement::Grip checkElementsBoundaries(int x, int y, MovableElement** elem = 0);
		MovableElement::Grip checkElementBoundaries(MovableElement*, int x, int y);
		static void cleanUp();
		virtual CursorType getCursor(int x, int y);
		virtual void mouseDoubleClick(int x, int y);

		static CropOverlay* cropOverlay_;
		bool isMoving_;
		bool allowCreatingElements_;

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
protected:
	POINT oldPoint_;
	AffectedSegments segments_;
	Gdiplus::Region affectedRegion_;
	virtual void drawLine(int x0, int y0, int x1, int y1);
};
#if GDIPVER >= 0x0110 
class BlurTool: public BrushTool  {
public:
	BlurTool( Canvas* canvas );
protected:
	virtual void drawLine(int x0, int y0, int x1, int y1);

	virtual void endDraw(int x, int y);

};
#endif
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

class SelectionTool : public MoveAndResizeTool {
	public:
		SelectionTool( Canvas* canvas );
};
}

#endif