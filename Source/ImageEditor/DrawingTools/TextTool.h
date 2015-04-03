#ifndef TextTool_h__
#define TextTool_h__

#include <3rdpart/GdiplusH.h>
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include <stdint.h>
#include "MoveAndResizeTool.h"
namespace ImageEditor {

	class Canvas;

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
}
#endif // TextTool_h__
