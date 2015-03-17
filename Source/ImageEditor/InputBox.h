#ifndef IMAGEEDITOR_INPUTBOX_H
#define IMAGEEDITOR_INPUTBOX_H

#include <Core/3rdpart/FastDelegate.h>
#include <GdiPlus.h>
#include "MovableElement.h"

namespace ImageEditor {
class InputBox {
	public:
		virtual ~InputBox(){};
		virtual void show(bool show) = 0;
		virtual void resize(int x, int y, int w,int h, std::vector<MovableElement::Grip> grips ) = 0;
		virtual void render(Gdiplus::Graphics* graphics, Gdiplus::Bitmap* background, Gdiplus::Rect layoutArea)=0;
		virtual bool isVisible() = 0;
		virtual void invalidate() = 0;
		fastdelegate::FastDelegate1<TCHAR*> onTextChanged;
		fastdelegate::FastDelegate2<int, int> onSizeChanged;
};
}

#endif