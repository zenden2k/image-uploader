#include "SelectionTool.h"

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"

#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>


namespace ImageEditor {

SelectionTool::SelectionTool(Canvas* canvas) : MoveAndResizeTool(canvas, etSelection)
{

}
}