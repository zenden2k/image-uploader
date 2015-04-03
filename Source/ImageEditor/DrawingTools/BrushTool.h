#ifndef BrushTool_h__
#define BrushTool_h__

#include <3rdpart/GdiplusH.h>
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include <stdint.h>
#include "AbstractDrawingTool.h"
namespace ImageEditor {

	class Canvas;



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

}

#endif // BrushTool_h__
