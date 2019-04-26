#include "PercentEdit.h"

#include <algorithm>

#include "Gui/GuiTools.h"
#include "Core/Scripting/API/HtmlElementPrivate_win.h"


CPercentEdit::CPercentEdit()
{
    unit_str_ = "%";
}

CPercentEdit::~CPercentEdit()
{
}

LRESULT CPercentEdit::OnKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetWindowText(GuiTools::GetWindowText(m_hWnd));
    return 0;
}

void CPercentEdit::setUnit(const CString& text)
{
    unit_str_ = text;
}

LRESULT CPercentEdit::OnSetFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     int quality = _wtoi(GuiTools::GetWindowText(m_hWnd));
     SetWindowText(WinUtils::IntToStr(quality));
     return 0;
}

BOOL CPercentEdit::SubclassWindow(HWND hWnd)
{
    Init();
    return CWindowImpl<CPercentEdit>::SubclassWindow(hWnd);
}

void CPercentEdit::Init()
{
}

LRESULT CPercentEdit::OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TCHAR* str =  reinterpret_cast<TCHAR*>(lParam);
    int quality = _wtoi(str);
    CString newStr = WinUtils::IntToStr(quality) + getCurrentPostfix();
    DefWindowProc(uMsg, wParam, (LPARAM)(const TCHAR*)newStr);
    bHandled = true;
    return TRUE;
}

CString  CPercentEdit::getCurrentPostfix()
{
    if(GetFocus() == m_hWnd)
    {
        return _T("");
    }
    else
    {
        return unit_str_;
    }
}

LRESULT CPercentEdit::OnGetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nCount = wParam;
    TCHAR * destination = (TCHAR*)lParam;
    TCHAR buf[256];
    /*LRESULT result = */DefWindowProc(uMsg, 256, (LPARAM)buf);
    int quality = _wtoi(buf);
    CString res = WinUtils::IntToStr(quality);
    int copied = std::min(res.GetLength()+1, nCount);
    lstrcpyn(destination, res, copied);
    bHandled = true;
    return copied;
}