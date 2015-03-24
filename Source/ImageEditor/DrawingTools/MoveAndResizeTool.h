#ifndef MoveAndResizeTool_h__
#define MoveAndResizeTool_h__

#include <Gdiplus.h>
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include <stdint.h>
#include "AbstractDrawingTool.h"
namespace ImageEditor {

class Canvas;

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
	POINT originalStartPoint_;
	POINT originalEndPoint_;
	RECT prevPaintBoundingRect_;
	bool allowMovingElements_;

};

}
#endif // MoveAndResizeTool_h__