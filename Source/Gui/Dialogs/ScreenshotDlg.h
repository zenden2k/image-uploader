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

#ifndef SCREENSHOTDLG_H
#define SCREENSHOTDLG_H


#pragma once

#include "atlheaders.h"
#include "resource.h"
#include "regionselect.h"
#include "Gui/Controls/hyperlinkcontrol.h"
#include "Core/ScreenCapture.h"
#include "Gui/Controls/DialogIndirect.h"

#define IDC_FULLSCREEN WM_USER + 219
#define IDC_VIEWSETTINGS WM_USER + 220
#define IDC_SCRACTIVEWINDOW WM_USER + 221
#define IDC_FREEFORMREGION WM_USER + 222
#define IDC_HWNDSREGION WM_USER + 223


class CScreenshotDlg : 
    public /*aero::*/CCustomDialogIndirectImpl<CScreenshotDlg>,
    public CWinDataExchange<CScreenshotDlg>
{
    public:
        CScreenshotDlg();
        ~CScreenshotDlg();
        CaptureMode captureMode() const;
        enum { IDD = IDD_SCREENSHOTDLG };
    
    protected:
        BEGIN_MSG_MAP(CScreenshotDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            /*MSG_WM_CTLCOLORDLG(OnCtlColorMsgDlg)
            MSG_WM_CTLCOLORBTN(OnCtlColorMsgDlg)
            MSG_WM_CTLCOLORSTATIC(OnCtlColorMsgDlg)*/
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER(IDC_FULLSCREEN, BN_CLICKED, OnClickedFullscreenCapture)
            COMMAND_ID_HANDLER(IDC_SCRACTIVEWINDOW,  OnClickedActiveWindowCapture)
            COMMAND_HANDLER(IDC_REGIONSELECT, BN_CLICKED, OnBnClickedRegionselect)
            COMMAND_HANDLER(IDC_FREEFORMREGION, BN_CLICKED, OnBnClickedFreeFormRegion)
            COMMAND_HANDLER(IDC_HWNDSREGION, BN_CLICKED, OnBnClickedWindowHandlesRegion)
        END_MSG_MAP()
    
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        
        // Handlers:
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnEnter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnCtlColorMsgDlg(HDC hdc, HWND hwndChild);

        LRESULT OnBnClickedFreeFormRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedWindowHandlesRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnClickedFullscreenCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnClickedActiveWindowCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedRegionselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

        CBrush m_WhiteBr;
        CHyperLinkControl CommandBox;
        CaptureMode m_CaptureMode;
        CComboBox m_monitorCombobox;
};

#endif // SCREENSHOTDLG_H