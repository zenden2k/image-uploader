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


#pragma once

#include <memory>

#include "atlheaders.h"
#include "resource.h"
#include "regionselect.h"
#include "Gui/Controls/DialogIndirect.h"
#include "Gui/WizardCommon.h"
#include "Core/ScreenCapture/MonitorEnumerator.h"

constexpr auto IDM_SELECT_REGION = 12001;
constexpr auto IDM_FULL_SCREEN = 12002;
constexpr auto IDM_WINDOW_LIST_FIRST = 13000;
constexpr auto IDM_WINDOW_LIST_LAST = 13999;

class IconBitmapUtils;

struct ScreenRecordingRuntimeParams {
    HWND selectedWindow {};
    CRect selectedRegion;
    int adapterIndex = -1;
    int monitorIndex = -1;
    HMONITOR monitor {};
};

class CScreenRecordingDlg : 
    public /*aero::*/CCustomDialogIndirectImpl<CScreenRecordingDlg>,
    public CWinDataExchange<CScreenRecordingDlg> {
    public:
        CScreenRecordingDlg();
        ~CScreenRecordingDlg();

        enum { IDD = IDD_SCREENRECODINGDLG };
    
    protected:
        BEGIN_MSG_MAP(CScreenRecordingDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_ID_HANDLER(IDM_SELECT_REGION, OnClickedSelectRegion)
            COMMAND_ID_HANDLER(IDM_FULL_SCREEN, OnClickedFullScreen)
            COMMAND_RANGE_HANDLER(IDM_WINDOW_LIST_FIRST, IDM_WINDOW_LIST_LAST, OnClickedWindow)
            COMMAND_HANDLER(IDC_REGIONSELECTBUTTON, BN_CLICKED, OnBnClickedRegionSelectButton)
            NOTIFY_HANDLER(IDC_REGIONSELECTBUTTON, BCN_DROPDOWN, OnBnDropdownRegionSelectButton)
        END_MSG_MAP()

        BEGIN_DDX_MAP(CScreenRecordingDlg)
            DDX_CONTROL_HANDLE(IDC_REGIONSELECTBUTTON, regionSelectButton_)
            DDX_CONTROL_HANDLE(IDC_DELAYSPIN, delaySpin_)
            DDX_CONTROL_HANDLE(IDC_MONITORSCOMBOBOX, monitorCombobox_)
        END_DDX_MAP()

        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        
        // Handlers:
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnCtlColorMsgDlg(HDC hdc, HWND hwndChild);
        LRESULT OnBnClickedRegionSelectButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnBnDropdownRegionSelectButton(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnClickedSelectRegion(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedFullScreen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedWindow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);


        CComboBox monitorCombobox_;
        CButton regionSelectButton_;
        CUpDownCtrl delaySpin_;
        ScreenRecordingRuntimeParams recordingParams() const;
        using HwndVector = std::vector<HWND>;

    private:
        void translateUI();
        void showRegionSelectButtonMenu(HWND hWndCtl);
        void updateRegionSelectButtonTitle();
        std::unique_ptr<IconBitmapUtils> iconBitmapUtils_;
        std::vector<HBITMAP> bitmaps_;
        ScreenRecordingRuntimeParams recordingParams_;
        HwndVector hwnds_;
        CIcon icon_, iconSmall_;
        CBrush m_WhiteBr;
        MonitorEnumerator monitorEnumerator_;
        void clearBitmaps();
};

