#ifndef SelectionTool_h__
#define SelectionTool_h__

#include "../DrawingElement.h"
#include "MoveAndResizeTool.h"
namespace ImageEditor {

class Canvas;

class SelectionTool : public MoveAndResizeTool {
public:
    explicit SelectionTool( Canvas* canvas );
};

}
#endif // SelectionTool_h__