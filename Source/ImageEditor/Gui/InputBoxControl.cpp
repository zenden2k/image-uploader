/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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

#include "InputBoxControl.h"

#include <sstream>
#include <ComDef.h>
#include <strsafe.h>

#include "Core/Images/Utils.h"
#include "Gui/GuiTools.h"
#include "ImageEditor/Canvas.h"
#include "Core/i18n/Translator.h"
#include "Gui/Helpers/DPIHelper.h"

#define DEFINE_GUID_(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID DECLSPEC_SELECTANY name                      \
        = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

DEFINE_GUID_(IID_ITextServices, 0x8d33f740, 0xcf58, 0x11ce, 0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5);
DEFINE_GUID_(IID_ITextHost, 0xc5bdd8d0, 0xd26e, 0x11ce, 0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5);
DEFINE_GUID_(IID_ITextHost2, 0xc5bdd8d7, 0xd26e, 0x11ce, 0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5);

namespace ImageEditor {

static HRESULT(STDAPICALLTYPE* pCreateTextServices)(IUnknown*, ITextHost*, IUnknown**) = nullptr;

InputBoxControl::InputBoxControl(Canvas* canvas)
    : canvas_(canvas) {
    ZeroMemory(&charFormat_, sizeof(charFormat_));
    ZeroMemory(&paraFormat_, sizeof(paraFormat_));
    ZeroMemory(&logFont_, sizeof(logFont_));

    cursor_ = ::LoadCursor(nullptr, IDC_IBEAM);
}

InputBoxControl::~InputBoxControl() {
    Destroy();
}

HWND InputBoxControl::Create(HWND hParent, const RECT& rc, DWORD style, DWORD exStyle) {

    ::LoadLibraryW(L"msftedit.dll");
    HMODULE h = ::GetModuleHandleW(L"msftedit.dll");
    if (!h)
        return nullptr;
    pCreateTextServices = (decltype(pCreateTextServices))::GetProcAddress(h, "CreateTextServices");
    if (!pCreateTextServices)
        return nullptr;
    RECT rcCopy = rc;
    CWindowImpl::Create(hParent, rcCopy, L"", style, exStyle);
    ::GetClientRect(m_hWnd, &clientRect_);
    if (!CreateTextServices()) {
        DestroyWindow();
        return nullptr;
    }
    return m_hWnd;
}

void InputBoxControl::Destroy() {
    services_.Release();
    servicesUnk_.Release();
    ::DestroyCaret();
    /*if (m_hWnd)
        DestroyWindow();*/
}

bool InputBoxControl::CreateTextServices() {
    if (!pCreateTextServices)
        return false;
    CComPtr<IUnknown> punk;
    HRESULT hr = pCreateTextServices(nullptr, static_cast<ITextHost*>(this), &punk);
    if (FAILED(hr))
        return false;

    servicesUnk_ = punk;
    hr = punk->QueryInterface(IID_ITextServices, (void**)&services_);
    if (FAILED(hr))
        return false;

    ApplyDefaults();
    return true;
}

void InputBoxControl::ApplyDefaults() {
    if (!services_)
        return;

    // Прозрачный фон, шрифт/параграф, событийная маска
    services_->TxSendMessage(EM_SETBKGNDCOLOR, 0, CLR_NONE, nullptr);
    services_->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&charFormat_, nullptr);
    services_->TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&paraFormat_, nullptr);

    DWORD mask = ENM_CHANGE | ENM_REQUESTRESIZE | ENM_SELCHANGE;
    services_->TxSendMessage(EM_SETEVENTMASK, 0, mask, nullptr);

    // Многострочность, автоскролл, не скрывать выделение
    LONG style = ES_MULTILINE | ES_AUTOVSCROLL /* | ES_NOHIDESEL*/|  ES_WANTRETURN;
    services_->TxSendMessage(EM_SETOPTIONS, ECOOP_OR, style, nullptr);

    // Плейнтекст режим — при необходимости можно убрать, если нужен RTF ввод по умолчанию
    //services_->TxSendMessage(EM_SETTEXTMODE, TM_PLAINTEXT, 0, nullptr);
    services_->TxSendMessage(EM_SETZOOM, 0, 0, nullptr);
}

// InputBox
void InputBoxControl::show(bool show) {
    visible_ = show;
    ShowWindow(show ? SW_SHOWNA : SW_HIDE);
    if (!show)
        ::SetFocus(GetParent());
}

void InputBoxControl::resize(int x, int y, int w, int h, std::vector<MovableElement::Grip> grips) {
    grips_ = std::move(grips);
    POINT scrollOffset { 0, 0 };
    if (canvas_)
        scrollOffset = canvas_->GetScrollOffset();

    ::SetWindowPos(m_hWnd, 0, x - scrollOffset.x, y - scrollOffset.y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
    ::GetClientRect(m_hWnd, &clientRect_);
    
}

void InputBoxControl::render(Gdiplus::Graphics* graphics, Gdiplus::Bitmap* background, Gdiplus::Rect layoutArea) {
    if (!services_)
        return;

    /*if (background) {
        graphics->DrawImage(background, layoutArea);
    } else {
        Gdiplus::SolidBrush brush(Gdiplus::Color(0, 0, 0, 0));
        graphics->FillRectangle(&brush, layoutArea);
    }*/

    HDC hdc = graphics->GetHDC();
    RECTL rc = { layoutArea.X, layoutArea.Y, layoutArea.GetRight(), layoutArea.GetBottom() };
    services_->TxDraw(DVASPECT_CONTENT, 0, nullptr, nullptr, hdc, nullptr, &rc, nullptr, nullptr, nullptr, 0,0);
    graphics->ReleaseHDC(hdc);
}

bool InputBoxControl::isVisible() {
    return IsWindowVisible() != FALSE;
}

void InputBoxControl::invalidate() {
    if (m_hWnd)
        Invalidate(false);
}

void InputBoxControl::setTextColor(Gdiplus::Color color) {
    textColor_ = color.ToCOLORREF();
    charFormat_.crTextColor = textColor_;
    charFormat_.dwMask |= CFM_COLOR;
    if (services_) {
        services_->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&charFormat_, nullptr);
    }
}

void InputBoxControl::setFont(LOGFONT font, DWORD changeMask) {
    return;
    logFont_ = font;
    charFormat_.cbSize = sizeof(charFormat_);
    // Расширенная маска для всех стилей
    DWORD defaultMask = CFM_FACE | CFM_SIZE | CFM_CHARSET | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
    charFormat_.dwMask |= (changeMask ? changeMask : defaultMask);

    if (logFont_.lfHeight != 0) {
        int dpi = DPIHelper::GetDpiForWindow(m_hWnd);
        int pointSize = MulDiv(-logFont_.lfHeight, 72, dpi);
        charFormat_.yHeight = pointSize * 20;
    }

    // Копируем имя шрифта
    StringCchCopy(charFormat_.szFaceName,std::size(charFormat_.szFaceName), logFont_.lfFaceName);

    // Обрабатываем стили текста из LOGFONT
    charFormat_.dwEffects = 0; // Сбрасываем все эффекты

    // Полужирный (Bold)
    if (logFont_.lfWeight >= FW_BOLD) {
        charFormat_.dwEffects |= CFE_BOLD;
    }

    // Курсив (Italic)
    if (logFont_.lfItalic) {
        charFormat_.dwEffects |= CFE_ITALIC;
    }

    // Подчеркнутый (Underline)
    if (logFont_.lfUnderline) {
        charFormat_.dwEffects |= CFE_UNDERLINE;
    }

    // Перечеркнутый (Strikeout)
    if (logFont_.lfStrikeOut) {
        charFormat_.dwEffects |= CFE_STRIKEOUT;
    }

    if (services_) {
        CHARRANGE selection = { 0 };

        if (services_) {
            services_->TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&selection, nullptr);
        }

        bool hasSelection = (selection.cpMax > selection.cpMin);

        WPARAM flags;
        if (visible_) {
            if (hasSelection) {
                flags = SCF_SELECTION | SCF_DEFAULT;
            } else {
                flags = SCF_DEFAULT;
            }
        } else {
            flags = SCF_ALL;
        }
        LOG(ERROR) << "EM_SETCHARFORMAT " << GetTickCount();

        services_->TxSendMessage(EM_SETCHARFORMAT, flags, (LPARAM)&charFormat_, nullptr);
    }
}

void InputBoxControl::setRawText(const std::string& text) {
    if (!services_)
        return;
    std::stringstream ss(text);
    EDITSTREAM es {};
    es.dwCookie = (DWORD_PTR)&ss;
    es.pfnCallback = &InputBoxControl::EditStreamInCallback;
    services_->TxSendMessage(EM_STREAMIN, SF_RTF, (LPARAM)&es, nullptr);
    onTextChanged(L"");
}

std::string InputBoxControl::getRawText() {
    if (!services_)
        return {};
    std::stringstream ss;
    EDITSTREAM es {};
    es.dwCookie = (DWORD_PTR)&ss;
    es.pfnCallback = &InputBoxControl::EditStreamOutCallback;
    services_->TxSendMessage(EM_STREAMOUT, SF_RTF, (LPARAM)&es, nullptr);
    return ss.str();
}

bool InputBoxControl::isEmpty() {
    if (!services_)
        return true;
    LRESULT len {};
    services_->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, &len);
    return len == 0;
}

// Stream callbacks
DWORD CALLBACK InputBoxControl::EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb) {
    auto* ss = reinterpret_cast<std::stringstream*>(dwCookie);
    ss->write((const char*)pbBuff, cb);
    *pcb = cb;
    return 0;
}
DWORD CALLBACK InputBoxControl::EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb) {
    auto* ss = reinterpret_cast<std::stringstream*>(dwCookie);
    ss->read((char*)pbBuff, cb);
    *pcb = static_cast<LONG>(ss->gcount());
    return 0;
}

// WTL handlers
LRESULT InputBoxControl::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled) {
    int dpi = DPIHelper::GetDpiForWindow(m_hWnd);
    // Базовый шрифт: Segoe UI 11pt
    wcscpy_s(logFont_.lfFaceName, L"Segoe UI");
    logFont_.lfHeight = -MulDiv(11, dpi, 72);
    logFont_.lfWeight = FW_NORMAL;

    charFormat_.cbSize = sizeof(charFormat_);
    charFormat_.dwMask = CFM_FACE | CFM_SIZE | CFM_COLOR | CFM_CHARSET /* | CFM_WEIGHT*/;
    charFormat_.yHeight = 11 * 20;
    charFormat_.crTextColor = textColor_;
    charFormat_.bCharSet = DEFAULT_CHARSET;
    // charFormat_.dwMask |= CFM_BOLD;
    // charFormat_.dwEffects |= CFE_BOLD;  // для жирного
    // charFormat_.dwEffects &= ~CFE_BOLD; // для обычного
    wcsncpy_s(charFormat_.szFaceName, logFont_.lfFaceName, _TRUNCATE);

    paraFormat_.cbSize = sizeof(paraFormat_);
    paraFormat_.dwMask = PFM_ALIGNMENT;
    paraFormat_.wAlignment = PFA_LEFT;

    bHandled = FALSE; // пусть базовый обработчик выполнится
    return 0;
}
LRESULT InputBoxControl::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
    Destroy();
    return 0;
}
LRESULT InputBoxControl::OnSize(UINT, WPARAM, LPARAM lParam, BOOL&) {
    int w = LOWORD(lParam), h = HIWORD(lParam);
    ::GetClientRect(m_hWnd, &clientRect_);
    if (services_) {
        services_->TxSendMessage(WM_SIZE, 0, lParam, nullptr);
        //onSizeChanged(w, h);
    }
    return 0;
}

LRESULT InputBoxControl::OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
    CPaintDC dc(m_hWnd);
    if (!services_ || !visible_) {
        return 0;
    }

    RECT clientRect;
    GetClientRect(&clientRect);

    RECTL rc = { clientRect.left, clientRect.top, clientRect.right, clientRect.bottom };
    RECTL rcPaint = { dc.m_ps.rcPaint.left, dc.m_ps.rcPaint.top, dc.m_ps.rcPaint.right, dc.m_ps.rcPaint.bottom };
    HRESULT hr = services_->TxDraw(DVASPECT_CONTENT, 0, nullptr, nullptr, dc, nullptr, &rc, nullptr, /*&dc.m_ps.rcPaint*/nullptr, nullptr, 0, 0);
    if (FAILED(hr)) {
        LOG(ERROR) << _com_error(hr).ErrorMessage();
    }
    return 0;
}

LRESULT InputBoxControl::OnSetFocus(UINT, WPARAM wParam, LPARAM, BOOL&) {
   services_->OnTxInPlaceActivate(nullptr);
    if (services_)
        services_->TxSendMessage(WM_SETFOCUS, wParam, 0, nullptr);
    return 0;
}
LRESULT InputBoxControl::OnKillFocus(UINT, WPARAM wParam, LPARAM, BOOL&) {
    ::HideCaret(m_hWnd);
    ::DestroyCaret();

    services_->OnTxInPlaceDeactivate();

    LRESULT result = 0;
    if (services_)
        services_->TxSendMessage(WM_KILLFOCUS, wParam, 0, &result);

    return result;
}
LRESULT InputBoxControl::OnTimer(UINT, WPARAM id, LPARAM lParam, BOOL&) {
    LRESULT result = 0;
    if (services_)
        services_->TxSendMessage(WM_TIMER, id, lParam, &result);
    return result;
}
LRESULT InputBoxControl::OnMouse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    LRESULT result = 0;
    if (services_)
        services_->TxSendMessage(uMsg, wParam, lParam, &result);
    return result;
}
LRESULT InputBoxControl::OnKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_ESCAPE) {
            onEditCanceled();
            bHandled = TRUE;
            return 0;
        }
        if (wParam == VK_RETURN && (GetKeyState(VK_CONTROL) & 0x8000)) {
            onEditFinished();
            bHandled = TRUE;
            return 0;
        }
    }

    LRESULT result = 0;
    if (services_)
        services_->TxSendMessage(uMsg, wParam, lParam, &result);
    return result;
}
LRESULT InputBoxControl::OnIme(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&) {
    LRESULT result = 0;
    if (services_)
        services_->TxSendMessage(uMsg, wParam, lParam, &result);
    return result;
}
LRESULT InputBoxControl::OnContextMenu(UINT, WPARAM, LPARAM lParam, BOOL&) {
    int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);

    if (x == -1 && y == -1) {
        POINT pt;
        GetCaretPos(&pt);
        ClientToScreen(&pt);
        x = pt.x;
        y = pt.y;
    }
    LRESULT canUndo = 0, canRedo = 0;
    CHARRANGE selection = { 0 };
    LRESULT textLength = 0;

    if (services_) {
        services_->TxSendMessage(EM_CANUNDO, 0, 0, &canUndo);
        services_->TxSendMessage(EM_CANREDO, 0, 0, &canRedo);
        services_->TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&selection, nullptr);
        services_->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, &textLength);
    }

    bool hasSelection = (selection.cpMax > selection.cpMin);
    bool hasText = (textLength > 0);
    bool canPaste = IsClipboardFormatAvailable(CF_TEXT) || IsClipboardFormatAvailable(CF_UNICODETEXT);

    CMenu contextMenu;
    contextMenu.CreatePopupMenu();
    contextMenu.AppendMenu(MF_STRING | (canUndo ? MF_ENABLED : MF_GRAYED),
        ID_EDIT_UNDO, TR("Undo"));
    contextMenu.AppendMenu(MF_STRING | (canRedo ? MF_ENABLED : MF_GRAYED),
        ID_EDIT_REDO, TR("Redo"));

    contextMenu.AppendMenu(MF_SEPARATOR);

    contextMenu.AppendMenu(MF_STRING | (hasSelection ? MF_ENABLED : MF_GRAYED),
        ID_EDIT_CUT, TR("Cut"));
    contextMenu.AppendMenu(MF_STRING | (hasSelection ? MF_ENABLED : MF_GRAYED),
        ID_EDIT_COPY, TR("Copy"));
    contextMenu.AppendMenu(MF_STRING | (canPaste ? MF_ENABLED : MF_GRAYED),
        ID_EDIT_PASTE, TR("Paste"));
    contextMenu.AppendMenu(MF_STRING | (hasSelection ? MF_ENABLED : MF_GRAYED),
        ID_EDIT_DELETE, TR("Delete"));

    contextMenu.AppendMenu(MF_SEPARATOR);

    contextMenu.AppendMenu(MF_STRING | (hasText ? MF_ENABLED : MF_GRAYED),
        ID_EDIT_SELECT_ALL, TR("Select All"));


    contextMenuOpened_ = true;
    UINT id = contextMenu.TrackPopupMenu(TPM_RETURNCMD | TPM_RIGHTBUTTON, x, y, m_hWnd);
    contextMenuOpened_ = false;

    switch (id) {
    case ID_EDIT_UNDO:
        services_->TxSendMessage(EM_UNDO, 0, 0, nullptr);
        break;

    case ID_EDIT_REDO:
        services_->TxSendMessage(EM_REDO, 0, 0, nullptr);
        break;

    case ID_EDIT_CUT:
        services_->TxSendMessage(WM_CUT, 0, 0, nullptr);
        break;

    case ID_EDIT_COPY:
        services_->TxSendMessage(WM_COPY, 0, 0, nullptr);
        break;

    case ID_EDIT_PASTE:
        SetFocus();
        services_->TxSendMessage(WM_PASTE, 0, 0, nullptr);
        break;

    case ID_EDIT_DELETE:
        services_->TxSendMessage(WM_CLEAR, 0, 0, nullptr);
        break;

    case ID_EDIT_SELECT_ALL:
        services_->TxSendMessage(EM_SETSEL, 0, -1, nullptr);
        break;

    default:
        break;
    }
    return 0;
}
LRESULT InputBoxControl::OnSetCursor(UINT, WPARAM, LPARAM, BOOL& bHandled) {
    if (contextMenuOpened_) {
        bHandled = TRUE;
        HCURSOR arrow = LoadCursor(nullptr, IDC_ARROW);
        ::SetCursor(arrow);
        return TRUE;
    } /* else {
        ::SetCursor(cursor_);
        return TRUE;
    }*/

    POINT position {};
    GetCursorPos(&position);
    ScreenToClient(&position);

    CClientDC hdc(m_hWnd);

    RECT rect = { 0 };
    GetClientRect(&rect);

    services_->OnTxSetCursor(
        DVASPECT_CONTENT,
        0,
        nullptr,
        nullptr,
        hdc,
        nullptr,
        &rect,
        position.x,
        position.y);

    return 0;
}

// IUnknown
STDMETHODIMP InputBoxControl::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv)
        return E_POINTER;
    *ppv = nullptr;
    if (riid == IID_IUnknown || riid == IID_ITextHost || riid == IID_ITextHost2) {
        *ppv = static_cast<ITextHost2*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

// ITextHost
HDC InputBoxControl::TxGetDC() { return ::GetDC(m_hWnd); }
INT InputBoxControl::TxReleaseDC(HDC hdc) { return ::ReleaseDC(m_hWnd, hdc); }
void InputBoxControl::TxInvalidateRect(LPCRECT prc, BOOL) {
    RECT rc = prc ? *prc : clientRect_;
    InvalidateRect(&rc, FALSE);
}
void InputBoxControl::TxViewChange(BOOL fUpdate) {
    if (fUpdate)
        UpdateWindow();
}

BOOL InputBoxControl::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) {
    return ::CreateCaret(m_hWnd, hbmp, xWidth, yHeight);
}

BOOL InputBoxControl::TxShowCaret(BOOL fShow) {
    if (fShow) {
        return ::ShowCaret(m_hWnd);
    } else {
        return ::HideCaret(m_hWnd);
    }
}

BOOL InputBoxControl::TxSetCaretPos(INT x, INT y) {
    return ::SetCaretPos(x, y);
}

BOOL InputBoxControl::TxSetTimer(UINT idTimer, UINT uTimeout) {
    return SetTimer(idTimer, uTimeout) != 0;
}
void InputBoxControl::TxKillTimer(UINT idTimer) {
    KillTimer(idTimer);
}

void InputBoxControl::TxSetCursor(HCURSOR hcur, BOOL fText) {
    cursor_ = hcur;
    ::SetCursor(hcur ? hcur : ::LoadCursor(nullptr, IDC_ARROW));
}

HRESULT InputBoxControl::TxGetClientRect(LPRECT prc) {
    *prc = clientRect_;
    return S_OK;
}

HRESULT InputBoxControl::TxGetExtent(LPSIZEL lpExtent) {
    int dpi = DPIHelper::GetDpiForWindow(m_hWnd);
    lpExtent->cx = MulDiv(clientRect_.right - clientRect_.left, 2540, dpi);
    lpExtent->cy = MulDiv(clientRect_.bottom - clientRect_.top, 2540, dpi);

    //lpExtent->cx = lpExtent->cy = 0;
    return S_OK;
}

HRESULT InputBoxControl::OnTxCharFormatChange(CONST CHARFORMATW* pCF) {
    if (pCF && (pCF->dwMask & CFM_COLOR))
        textColor_ = pCF->crTextColor;
    return S_OK;
}
HRESULT InputBoxControl::TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits) {
    DWORD bits = TXTBIT_RICHTEXT | TXTBIT_MULTILINE | TXTBIT_WORDWRAP | TXTBIT_USECURRENTBKG | TXTBIT_CLIENTRECTCHANGE;
    *pdwBits = bits & dwMask;
    return S_OK;
}
HRESULT InputBoxControl::TxNotify(DWORD iNotify, void* pv) {
    switch (iNotify) {
    case EN_CHANGE: {
        CComBSTR str;
        services_->TxGetText(&str);
        onTextChanged(L"");
        break;
    }
    case EN_REQUESTRESIZE: {
        auto* rr = reinterpret_cast<REQRESIZE*>(pv);
        CRect windowRect;
        GetClientRect(&windowRect);
        int w = rr->rc.right - rr->rc.left;
        int h = rr->rc.bottom - rr->rc.top;
        onResized(windowRect.Width(), h);
        break;
    }
    case EN_SELCHANGE: {
        auto* sc = reinterpret_cast<SELCHANGE*>(pv);
        onSelectionChanged(sc->chrg.cpMin, sc->chrg.cpMax, logFont_);
        break;
    }
    default:
        break;
    }
    return S_OK;
}

}
