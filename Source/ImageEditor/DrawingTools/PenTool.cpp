/*
     Image Uploader - program for uploading images/files to the Internet

     Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "PenTool.h"

#include <cassert>

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"
#include "Core/Utils/CoreUtils.h"
#include "3rdpart/GdiplusH.h"

namespace ImageEditor {

/*
 Pen Tool
*/

PenTool::PenTool( Canvas* canvas ): AbstractDrawingTool( canvas )  {
    oldPoint_ = { 0, 0 };
}

void PenTool::beginDraw( int x, int y ) {
    canvas_->beginDocDrawing();
    oldPoint_.x = x;
    oldPoint_.y = y;
}

void PenTool::continueDraw( int x, int y, DWORD flags ) {
    if ( flags & MK_CONTROL ) {
        y = oldPoint_.y;
    }
    Line * line =  new Line( canvas_, oldPoint_.x, oldPoint_.y, x, y) ;

    line->setPenSize(1 );
    line->setColor(foregroundColor_);
    line->setBackgroundColor(backgroundColor_);

    line->setCanvas(canvas_);
    canvas_->currentDocument()->addDrawingElement( line );

    oldPoint_.x = x;
    oldPoint_.y = y;
    canvas_->updateView();
}

void PenTool::endDraw( int x, int y ) {
    canvas_->endDocDrawing();
}

void PenTool::render( Painter* gr ) {

}

CursorType PenTool::getCursor(int x, int y)
{
    return CursorType::ctCross;
}

}
