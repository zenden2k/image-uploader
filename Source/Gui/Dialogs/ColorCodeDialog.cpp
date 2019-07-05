/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "Func/WinUtils.h"

// CColorCodeDialog
CColorCodeDialog::CColorCodeDialog(COLORREF color) : value_(color)
{
}

CColorCodeDialog::~CColorCodeDialog()
{
}

LRESULT CColorCodeDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetWindowText(TR("Color codes"));
    TRC(IDCANCEL, "Close");
    TRC(IDOK, "OK");
    TRC(IDC_COPYHEXBUTTON, "Copy");
    TRC(IDC_COPYRGBBUTTON, "Copy");
    BYTE r = GetRValue(value_);
    BYTE g = GetGValue(value_);
    BYTE b = GetBValue(value_);
    std::wstring hexStr = str(boost::wformat(L"#%02x%02x%02x") % r % g % b);
    std::wstring rgbStr = str(boost::wformat(L"rgb(%d, %d, %d)") % r % g % b);
    SetDlgItemText(IDC_HEXEDIT, hexStr.c_str());
    SetDlgItemText(IDC_RGBEDIT, rgbStr.c_str());
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
