#ifndef IMAGEEDITOR_INPUTBOX_H
#define IMAGEEDITOR_INPUTBOX_H

#include <Core/3rdpart/FastDelegate.h>
#include <3rdpart/GdiplusH.h>
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
		virtual void setTextColor(Gdiplus::Color color) =0;
		virtual void setFont(LOGFONT font, DWORD changeMask) = 0;
		virtual void setRawText(const std::string& text) = 0;
		virtual std::string getRawText() = 0;
		fastdelegate::FastDelegate1<TCHAR*> onTextChanged;
		fastdelegate::FastDelegate2<int, int> onSizeChanged;
		fastdelegate::FastDelegate0<void> onEditCanceled;
		fastdelegate::FastDelegate0<void> onEditFinished;
		fastdelegate::FastDelegate2<int,int> onResized;
		fastdelegate::FastDelegate3<int,int,LOGFONT> onSelectionChanged;

};
}

#endif