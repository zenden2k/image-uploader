#ifndef PenTool_h__
#define PenTool_h__

#include "3rdpart/GdiplusH.h"
#include "../DrawingElement.h"
#include "../MovableElement.h"
#include "AbstractDrawingTool.h"

namespace ImageEditor {
    class Canvas;

class PenTool: public AbstractDrawingTool  {
public:
    explicit PenTool( Canvas* canvas );
    void beginDraw( int x, int y ) override;
    void continueDraw( int x, int y, DWORD flags = 0) override;
    void endDraw( int x, int y ) override;
    void render( Painter* gr ) override;
    CursorType getCursor(int x, int y) override;
private:
    POINT oldPoint_;
};

}

#endif // PenTool_h__
