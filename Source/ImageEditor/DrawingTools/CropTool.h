#ifndef CropTool_h__
#define CropTool_h__

#include "3rdpart/GdiplusH.h"
#include "../DrawingElement.h"
#include "../MovableElement.h"	
#include <stdint.h>
#include "MoveAndResizeTool.h"
namespace ImageEditor {

	class Canvas;



class CropTool : public MoveAndResizeTool {
public:
	CropTool(Canvas* canvas);
	void beginDraw( int x, int y );
	void continueDraw( int x, int y, DWORD flags );
	void endDraw( int x, int y );
};

}

#endif // CropTool_h__