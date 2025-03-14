/*

    Image Uploader -  free application for uploading images/files to the Internet

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

#include "ColorCodeDialog.h"

#include <boost/format.hpp>

#include "Core/3rdpart/pcreplusplus.h"
#include "Func/WinUtils.h"

// CColorCodeDialog
CColorCodeDialog::CColorCodeDialog(COLORREF color) : value_(color)
{
    updateValue(color);
}

CColorCodeDialog::~CColorCodeDialog()
{
}

LRESULT CColorCodeDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetWindowText(TR("Color codes"));
    TRC(IDCANCEL, "Cancel");
    TRC(IDOK, "OK");
    TRC(IDC_COPYHEXBUTTON, "Copy");
    TRC(IDC_COPYRGBBUTTON, "Copy");
    generateHexEditText();
    generateRgbEditText();
    CenterWindow(GetParent());
    ::SetFocus(GetDlgItem(IDC_HEXEDIT));
    return 1;  // Do not Let the system set the focus
}

LRESULT CColorCodeDialog::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    //value_ = val;
    EndDialog(wID);
    return 0;
}

LRESULT CColorCodeDialog::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

COLORREF CColorCodeDialog::getValue() const
{
    return value_;
}


LRESULT CColorCodeDialog::OnBnClickedCopyHexButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CString val = GuiTools::GetWindowText(GetDlgItem(IDC_HEXEDIT));
    WinUtils::CopyTextToClipboard(val);
    return 0;
}


LRESULT CColorCodeDialog::OnBnClickedCopyRgbButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CString val = GuiTools::GetWindowText(GetDlgItem(IDC_RGBEDIT));
    WinUtils::CopyTextToClipboard(val);
    return 0;
}

HBRUSH CColorCodeDialog::OnCtlColorStatic(CDCHandle dc, CStatic wndStatic) {
    if (wndStatic.m_hWnd == GetDlgItem(IDC_COLORSTATIC)) {
        return brush_;
    } 

    SetMsgHandled(FALSE);
    return GetSysColorBrush(COLOR_BTNFACE);
}

void CColorCodeDialog::updateValue(COLORREF ref) {
    value_ = ref;
    if (brush_.m_hBrush) {
        brush_.DeleteObject();
    }
    brush_.CreateSolidBrush(ref);
    if (m_hWnd) {
        ::InvalidateRect(GetDlgItem(IDC_COLORSTATIC), nullptr, TRUE);
    }
}

LRESULT CColorCodeDialog::OnHexEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    if (GetFocus() != hWndCtl) {
        return 0;
    }
    CString text = GuiTools::GetDlgItemText(m_hWnd, IDC_HEXEDIT);

    text.Trim();
    CString sub;
    if (text.Left(1)  == _T("#")) {
        sub = text.Mid(1);
    } else {
        sub = text;
    }

    if (sub.GetLength() == 3) {
        sub.Format(_T("%c%c%c%c%c%c"), sub[0], sub[0], sub[1], sub[1], sub[2], sub[2]);
    }

    COLORREF crefColor = RGB(0, 0, 0);

    if (sub.GetLength() == 6) {
        LPCTSTR pszTmp = sub;

        LPTSTR pStop;
        INT nTmp = _tcstol(pszTmp, &pStop, 16);
        INT nR = (nTmp & 0xFF0000) >> 16;
        INT nG = (nTmp & 0xFF00) >> 8;
        INT nB = nTmp & 0xFF;

        crefColor = RGB(nR, nG, nB);
    }
    updateValue(crefColor);
    generateRgbEditText();
    return 0;
}

LRESULT CColorCodeDialog::OnRgbEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    if (GetFocus() != hWndCtl) {
        return 0;
    }
    CString text = GuiTools::GetDlgItemText(m_hWnd, IDC_RGBEDIT);
    text.Trim();

    pcrepp::Pcre regexp(R"(rgb\((\d+), *(\d+), *(\d+)\))", "i");

    COLORREF crefColor = RGB(0, 0, 0);

    if (regexp.search(W2U(text))) {
        int r = std::stoi(regexp.get_match(1));
        int g = std::stoi(regexp.get_match(2));
        int b = std::stoi(regexp.get_match(3));
        crefColor = RGB(r, g, b);
    }
    
    updateValue(crefColor);
    generateHexEditText();

    return 0;
}

void CColorCodeDialog::generateHexEditText() {
    BYTE r = GetRValue(value_);
    BYTE g = GetGValue(value_);
    BYTE b = GetBValue(value_);
    std::wstring hexStr = str(boost::wformat(L"#%02x%02x%02x") % r % g % b);
    SetDlgItemText(IDC_HEXEDIT, hexStr.c_str());
}

void CColorCodeDialog::generateRgbEditText() {
    BYTE r = GetRValue(value_);
    BYTE g = GetGValue(value_);
    BYTE b = GetBValue(value_);
    std::wstring rgbStr = str(boost::wformat(L"rgb(%d, %d, %d)") % r % g % b);
    SetDlgItemText(IDC_RGBEDIT, rgbStr.c_str());
}
