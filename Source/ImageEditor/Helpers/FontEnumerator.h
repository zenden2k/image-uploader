#ifndef IMAGEDITOR_HELPERS_FONT_ENUMERATOR
#define IMAGEDITOR_HELPERS_FONT_ENUMERATOR

#include <windows.h>
#include <vector>
#include "Core/3rdpart/FastDelegate.h"

class FontEnumerator {

public:
    typedef fastdelegate::FastDelegate0<void> OnEnumerationFinishedDelegate;
    FontEnumerator(HDC dc, std::vector<LOGFONT>& fonts, const OnEnumerationFinishedDelegate& onEnumerationFinished);

    void run();
protected:
    OnEnumerationFinishedDelegate onEnumerationFinished_;
    HDC dc_;
    std::vector<LOGFONT>& fonts_;
    static int CALLBACK FontEnumerator::EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam);
};

#endif