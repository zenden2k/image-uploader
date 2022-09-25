#ifndef CropTool_h__
#define CropTool_h__

#include "3rdpart/GdiplusH.h"
#include "../DrawingElement.h"
#include "MoveAndResizeTool.h"

namespace ImageEditor {

class Canvas;

class CropTool : public MoveAndResizeTool {
public:
    explicit CropTool(Canvas* canvas);
    void beginDraw( int x, int y ) override;
    void continueDraw( int x, int y, DWORD flags ) override;
    void endDraw( int x, int y ) override;
    void applyOperation() override;
    void cancelOperation() override;
private:
    MovableElement* lastCropElement_ = nullptr;
};

}

#endif // CropTool_h__