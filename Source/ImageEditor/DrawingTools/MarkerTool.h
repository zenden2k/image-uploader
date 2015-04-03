#ifndef MarkerTool_h__
#define MarkerTool_h__


#include <3rdpart/GdiplusH.h>
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include <stdint.h>
#include "AbstractDrawingTool.h"
namespace ImageEditor {

	class Canvas;

class MarkerTool: public AbstractDrawingTool  {
public:
	MarkerTool( Canvas* canvas );
	~MarkerTool();
	void beginDraw( int x, int y );
	void continueDraw( int x, int y, DWORD flags = 0);
	void endDraw( int x, int y );
	void render( Painter* gr );
	virtual CursorType getCursor(int x, int y);
protected:
	POINT oldPoint_;
	int circleStride_;
	uint8_t* circleData_;
	AffectedSegments segments_;
	Gdiplus::Region affectedRegion_;
	virtual void drawLine(int x0, int y0, int x1, int y1);
	void highlightRegion(RECT rc);
	void createCircle();

	virtual void setPenSize(int size);

};
}

#endif // MarkerTool_h__
