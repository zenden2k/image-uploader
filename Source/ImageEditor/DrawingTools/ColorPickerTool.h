#ifndef ColorPickerTool_h__
#define ColorPickerTool_h__

#include "../DrawingElement.h"
#include "../MovableElement.h"
#include "AbstractDrawingTool.h"
#include "3rdpart/GdiplusH.h"
namespace ImageEditor {

class  ColorPickerTool : public AbstractDrawingTool {
public:
    ColorPickerTool( Canvas* canvas );
    void beginDraw( int x, int y ) override;
    void continueDraw( int x, int y, DWORD flags ) override;
    void endDraw( int x, int y ) override;
    void render( Painter* gr ) override;
    CursorType getCursor(int x, int y) override;

    void rightButtonClick(int x, int y) override;

};
}

#endif // ColorPickerTool_h__