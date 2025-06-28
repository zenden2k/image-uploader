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

#ifndef SCREENSHOTDLG_H
#define SCREENSHOTDLG_H


#pragma once

#include "atlheaders.h"
#include "resource.h"
#include "regionselect.h"
#include "Gui/Controls/hyperlinkcontrol.h"
#include "Core/ScreenCapture.h"
#include "Gui/Controls/DialogIndirect.h"
#include "Gui/WizardCommon.h"

#define IDC_FULLSCREEN WM_USER + 219
#define IDC_VIEWSETTINGS WM_USER + 220
#define IDC_SCRACTIVEWINDOW WM_USER + 221
#define IDC_FREEFORMREGION WM_USER + 222
#define IDC_HWNDSREGION WM_USER + 223
#define IDC_TOPWINDOWREGION WM_USER + 224

class CScreenshotDlg : 
    public /*aero::*/CCustomDialogIndirectImpl<CScreenshotDlg>,
    public CWinDataExchange<CScreenshotDlg>,
    public CDialogResize<CScreenshotDlg>
{
    public:
        explicit CScreenshotDlg(bool enableLastRegionScreenshot);
        ~CScreenshotDlg() = default;
        ScreenCapture::CaptureMode captureMode() const;
        enum { IDD = IDD_SCREENSHOTDLG };
    
    protected:
        BEGIN_MSG_MAP(CScreenshotDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged)
            /*MSG_WM_CTLCOLORDLG(OnCtlColorMsgDlg)
            MSG_WM_CTLCOLORBTN(OnCtlColorMsgDlg)
            MSG_WM_CTLCOLORSTATIC(OnCtlColorMsgDlg)*/
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER(IDC_FULLSCREEN, BN_CLICKED, OnClickedFullscreenCapture)
            COMMAND_ID_HANDLER(IDC_SCRACTIVEWINDOW,  OnClickedActiveWindowCapture)
            COMMAND_HANDLER(IDC_REGIONSELECT, BN_CLICKED, OnBnClickedRegionselect)
            COMMAND_HANDLER(IDC_FREEFORMREGION, BN_CLICKED, OnBnClickedFreeFormRegion)
            COMMAND_HANDLER(IDC_HWNDSREGION, BN_CLICKED, OnBnClickedWindowHandlesRegion)
            COMMAND_HANDLER(IDC_TOPWINDOWREGION, BN_CLICKED, OnBnClickedTopWindowRegion)
            COMMAND_HANDLER(IDC_LASTREGIONSCREENSHOT, BN_CLICKED, OnBnClickedLastRegion)
            CHAIN_MSG_MAP(CDialogResize<CScreenshotDlg>)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CScreenshotDlg)
            DLGRESIZE_CONTROL(IDC_COMMANDBOX, DLSZ_SIZE_X | DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDC_DELAYLABEL,  DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_DELAYEDIT, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_DELAYSPIN, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_HORLINESTATIC, DLSZ_SIZE_X | DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_SECLABEL, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_CAPTURECURSORCHECKBOX, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_OPENSCREENSHOTINEDITORCHECKBOX, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_MONITORSCOMBOBOX, DLSZ_MOVE_Y)
        END_DLGRESIZE_MAP()

        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        
        // Handlers:
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnEnter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnCtlColorMsgDlg(HDC hdc, HWND hwndChild);

        LRESULT OnBnClickedFreeFormRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedWindowHandlesRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnClickedFullscreenCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnClickedActiveWindowCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedRegionselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedTopWindowRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedLastRegion(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    private:
        void fillCommandBox();
        BOOL endDialogOK();
        CBrush m_WhiteBr;
        CHyperLinkControl commandBox_;
        ScreenCapture::CaptureMode m_CaptureMode;
        CComboBox m_monitorCombobox;
        bool enableLastRegionScreenshot_ = false;
};

#endif // SCREENSHOTDLG_H
