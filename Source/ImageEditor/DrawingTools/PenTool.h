#ifndef PenTool_h__
#define PenTool_h__

#include "3rdpart/GdiplusH.h"
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include <stdint.h>
#include "AbstractDrawingTool.h"
namespace ImageEditor {

    class Canvas;

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

}

#endif // PenTool_h__
