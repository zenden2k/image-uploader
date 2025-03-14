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

#ifndef GUI_DIALOGS_COLORCODEDIALOG_H
#define GUI_DIALOGS_COLORCODEDIALOG_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
#include "Gui/Controls/DialogIndirect.h"
// CColorCodeDialog

class CColorCodeDialog :
    public CCustomDialogIndirectImpl<CColorCodeDialog>
{
    public:
        CColorCodeDialog(COLORREF color);
        ~CColorCodeDialog();
        enum { IDD = IDD_COLORCODEDIALOG };

        BEGIN_MSG_MAP(CColorCodeDialog)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER(IDC_COPYHEXBUTTON, BN_CLICKED, OnBnClickedCopyHexButton)
            COMMAND_HANDLER(IDC_COPYRGBBUTTON, BN_CLICKED, OnBnClickedCopyRgbButton)
            COMMAND_HANDLER(IDC_HEXEDIT, EN_CHANGE, OnHexEditChange)
            COMMAND_HANDLER(IDC_RGBEDIT, EN_CHANGE, OnRgbEditChange)
            MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
        END_MSG_MAP()

        COLORREF getValue() const;

    protected:
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnBnClickedCopyHexButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedCopyRgbButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        HBRUSH OnCtlColorStatic(CDCHandle dc, CStatic wndStatic);
        LRESULT OnHexEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnRgbEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        COLORREF value_;
        CBrush brush_;
        void updateValue(COLORREF ref);
        void generateHexEditText();
        void generateRgbEditText();
};

#endif // ColorCodeDialog_H
