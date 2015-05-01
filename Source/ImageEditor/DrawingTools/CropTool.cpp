/*
     Image Uploader - program for uploading images/files to the Internet

     Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

     Licensed under the Apache License, Version 2.0 (the "License");
     you may not use this file except in compliance with the License.
     You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

     Unless required by applicable law or agreed to in writing, software
     distributed under the License is distributed on an "AS IS" BASIS,
     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     See the License for the specific language governing permissions and
     limitations under the License.
*/

#include "CropTool.h"

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"

#include "Core/Utils/CoreUtils.h"
#include "Core/Logging.h"

#include <math.h>
#include <cassert>
#include "3rdpart/GdiplusH.h"
#include <math.h>

namespace ImageEditor {

CropTool::CropTool(Canvas* canvas) : MoveAndResizeTool(canvas, etCrop) {

}

void CropTool::beginDraw(int x, int y)
{
    MoveAndResizeTool::beginDraw(x,y);
}

void CropTool::continueDraw(int x, int y, DWORD flags)
{
    MoveAndResizeTool::continueDraw(x,y,flags);

}

void CropTool::endDraw(int x, int y)
{
    MoveAndResizeTool::endDraw(x, y);
    if ( currentElement_ ) {
        POINT pt = { x, y };
        if ( x == startPoint_.x && y == startPoint_.y ) {
            canvas_->deleteMovableElement(currentElement_);
            currentElement_ = 0;
            canvas_->showOverlay(false);
        }
        canvas_->updateView();
        currentElement_ = 0;
    }
}

}