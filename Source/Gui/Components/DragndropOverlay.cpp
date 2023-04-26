#include "DragndropOverlay.h"

#include "Core/i18n/Translator.h"
#include "Func/WinUtils.h"

LRESULT CDragndropOverlay::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    CRect clientRect;
    GetClientRect(&clientRect);
    CPaintDC paintDc(m_hWnd);

    paintDc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), backBufferDc_, 0, 0, SRCCOPY);

    return 0;
}

LRESULT  CDragndropOverlay::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    LOGFONT lf;
    WinUtils::StringToFont(_T("Arial,15,b,204"), &lf);
    font_.CreateFontIndirect(&lf);

    CRect clientRect;
    GetClientRect(&clientRect);
    CWindowDC dc(m_hWnd);
    backBufferDc_.CreateCompatibleDC(dc);
    backBufferBm_.CreateCompatibleBitmap(dc, clientRect.Width(), clientRect.Height());
    oldBm_ = backBufferDc_.SelectBitmap(backBufferBm_);

    CRect firstRect = clientRect;
    firstRect.bottom /= 2;
    items_.emplace_back(kAddToTheList, firstRect, TR("Add to the list"));
    CRect secondRect = clientRect;
    secondRect.top = firstRect.bottom;
    items_.emplace_back(kImportVideoFile, secondRect, TR("Import Video File"));

    updateBackBuffer();
    return 0;
}

LRESULT CDragndropOverlay::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    backBufferDc_.SelectBitmap(oldBm_);
    return 0;
}

void CDragndropOverlay::updateBackBuffer() {
    CRect clientRect;
    GetClientRect(&clientRect);
    
    CBrush br;
    br.CreateSolidBrush(RGB(128, 128, 128));

    backBufferDc_.SetDCPenColor(RGB(255, 255, 255));
    backBufferDc_.SetBkColor(RGB(120, 120, 120));
    backBufferDc_.FillRect(&clientRect, br);

    HFONT oldFont = backBufferDc_.SelectFont(font_);
    int index = 0;
    for (auto& item : items_) {
        ///CRect firstRect = clientRect;
        //firstRect.bottom /= 2;

        if (activeItemIndex_ == index) {
            CBrush br2;
            br2.CreateSolidBrush(RGB(100, 100, 100));

            HBRUSH oldBr = backBufferDc_.SelectBrush(br2);
            backBufferDc_.Rectangle(&item.rc);
            backBufferDc_.SelectBrush(oldBr);
        }
        backBufferDc_.SetTextColor(RGB(255, 255, 255));
        backBufferDc_.SetBkMode(TRANSPARENT);
        backBufferDc_.DrawText(item.text, -1, &item.rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        index++;
    }
   
    backBufferDc_.SelectFont(oldFont);
}

LRESULT CDragndropOverlay::OnEraseBkg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = true;
    return TRUE;
}

LRESULT CDragndropOverlay::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);
    
    dragMove(xPos, yPos);
    return 0;
}

void CDragndropOverlay::dragMove(int x, int y) {
    POINT pt{ x, y };

    int index = 0;
    int oldIndex = activeItemIndex_;
    activeItemIndex_ = -1;
    for (auto& item : items_) {
        if (PtInRect(&item.rc, pt)) {
            activeItemIndex_ = index;
        }

        index++;
    }
    if (oldIndex != activeItemIndex_) {
        updateBackBuffer();
        InvalidateRect(nullptr);
    }
}

CDragndropOverlay::ITEM_ID CDragndropOverlay::itemAtPos(int x, int y) {
    POINT pt{ x, y };

    int index = 0;

    for (auto& item : items_) {
        if (PtInRect(&item.rc, pt)) {
            return item.id;
        }

        index++;
    }
    return kInvalid;
}