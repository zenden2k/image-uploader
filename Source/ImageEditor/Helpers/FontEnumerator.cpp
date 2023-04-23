#include "FontEnumerator.h"

#include <algorithm>
#include <utility>

#include <tchar.h>

namespace {

bool compareLogFont(const LOGFONT& font1, const LOGFONT& font2) {
    return lstrcmp(font1.lfFaceName, font2.lfFaceName) == 0;
}

bool sortCompareLogFont(const LOGFONT& font1, const LOGFONT& font2) {
    return lstrcmp(font1.lfFaceName, font2.lfFaceName) <0;
}

}

int CALLBACK FontEnumerator::EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam) {
    auto* enumerator = reinterpret_cast<FontEnumerator*>(lParam);
    if (lpelfe->lfFaceName[0] != _T('@')) {
        enumerator->fonts_.push_back(*lpelfe);
    }
    return 1;
}

FontEnumerator::FontEnumerator(HDC dc, std::vector<LOGFONT>& fonts, OnEnumerationFinishedDelegate onEnumerationFinished) : fonts_(fonts) {
    onEnumerationFinished_ = std::move(onEnumerationFinished);
    dc_ = dc;
}

void FontEnumerator::run()
{
    LOGFONT logFont;
    memset(&logFont, 0, sizeof(logFont));
    logFont.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamiliesEx(dc_, &logFont, EnumFontFamExProc, reinterpret_cast<LPARAM>(this), 0);
    std::sort(fonts_.begin(), fonts_.end(), sortCompareLogFont);
    auto it = std::unique(fonts_.begin(), fonts_.end(), compareLogFont);
    fonts_.resize(std::distance(fonts_.begin(), it));
    onEnumerationFinished_();
}
/*
std::vector<LOGFONT>::const_iterator FontEnumerator::begin() const {
    return fonts_.begin();
}

std::vector<LOGFONT>::const_iterator FontEnumerator::end() const {
    return fonts_.end();
}

size_t FontEnumerator::count() const {
    return fonts_.size();
}

bool FontEnumerator::contains(LPTSTR fontName) const {
    for (const auto& font : fonts_) {
        if (!lstrcmp(fontName, font.lfFaceName)) {
            return true;
        }
    }
    return false;
}*/