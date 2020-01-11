#ifndef IMAGEDITOR_HELPERS_FONT_ENUMERATOR
#define IMAGEDITOR_HELPERS_FONT_ENUMERATOR

#include <Windows.h>
#include <vector>
#include <functional>

class FontEnumerator {

public:
    using OnEnumerationFinishedDelegate = std::function<void()>;
    FontEnumerator(HDC dc, std::vector<LOGFONT>& fonts, OnEnumerationFinishedDelegate onEnumerationFinished);

    void run();
protected:
    OnEnumerationFinishedDelegate onEnumerationFinished_;
    HDC dc_;
    std::vector<LOGFONT>& fonts_;
    static int CALLBACK EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam);
};

#endif