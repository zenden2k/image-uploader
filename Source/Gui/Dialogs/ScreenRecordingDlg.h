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


#pragma once

#include <memory>

#include "atlheaders.h"
#include "resource.h"
#include "RegionSelect.h"
#include "Gui/Controls/DialogIndirect.h"
#include "Gui/WizardCommon.h"
#include "ScreenCapture/ScreenRecorder/Common.h"
#include "ScreenCapture/MonitorEnumerator.h"
#include "ScreenCapture/ScreenCaptureWin.h"

constexpr auto IDM_SELECT_REGION = 12001;
constexpr auto IDM_FULL_SCREEN = 12002;
constexpr auto IDM_SELECT_WINDOW = 12003;
constexpr auto IDM_WINDOW_LIST_FIRST = 13000;
constexpr auto IDM_WINDOW_LIST_LAST = 13999;

class IconBitmapUtils;
class DXGIOptionsManager;
class FFMpegOptionsManager;

struct ScreenRecordingRuntimeParams {
    HWND selectedWindow {};
    CRect selectedRegion;
    //int adapterIndex = -1;
    int monitorIndex = -1;
    HMONITOR monitor_ {}; // valid only for fullscreen mode (when both selectedWindow and selectedRegion are empty)

    void setMonitor(HMONITOR mon, int index = -1) {
        monitor_ = mon;
        MonitorEnumerator monitorEnumerator_;
        monitorEnumerator_.enumDisplayMonitors(NULL, nullptr);
        if (index == -1) {
            auto it = std::find_if(monitorEnumerator_.begin(), monitorEnumerator_.end(), [mon](const MonitorEnumerator::MonitorInfo& info) {
                return info.monitor == mon;
            });
            if (it != monitorEnumerator_.end()) {
                monitorIndex = std::distance(monitorEnumerator_.begin(), it);
            } else {
                monitorIndex = -1;
            }
        } else {
            monitorIndex = index;
        }
    }

    HMONITOR monitor() const {
        return monitor_;
    }

};

class CScreenRecordingDlg :
    public /*aero::*/CCustomDialogIndirectImpl<CScreenRecordingDlg>,
    public CWinDataExchange<CScreenRecordingDlg> {
    public:
        CScreenRecordingDlg(ScreenRecordingRuntimeParams params);
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
            COMMAND_ID_HANDLER(IDM_SELECT_WINDOW, OnClickedSelectWindow)
            COMMAND_RANGE_HANDLER(IDM_WINDOW_LIST_FIRST, IDM_WINDOW_LIST_LAST, OnClickedWindow)
            COMMAND_HANDLER(IDC_REGIONSELECTBUTTON, BN_CLICKED, OnBnClickedRegionSelectButton)
            NOTIFY_HANDLER(IDC_REGIONSELECTBUTTON, BCN_DROPDOWN, OnBnDropdownRegionSelectButton)
        END_MSG_MAP()

        BEGIN_DDX_MAP(CScreenRecordingDlg)
            DDX_CONTROL_HANDLE(IDC_REGIONSELECTBUTTON, regionSelectButton_)
            DDX_CONTROL_HANDLE(IDC_DELAYSPIN, delaySpin_)
            DDX_CONTROL_HANDLE(IDC_MONITORSCOMBOBOX, monitorCombobox_)
            DDX_CONTROL_HANDLE(IDC_AUDIOSOURCESLISTVIEW, audioSourcesListView_)
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
        LRESULT OnClickedSelectWindow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

        CComboBox monitorCombobox_;
        CButton regionSelectButton_;
        CUpDownCtrl delaySpin_;
        CListViewCtrl audioSourcesListView_;

        ScreenRecordingRuntimeParams recordingParams() const;
        using HwndVector = std::vector<HWND>;

    private:
        void translateUI();
        void showRegionSelectButtonMenu(HWND hWndCtl);
        void updateRegionSelectButtonTitle();
        std::pair<ScreenCapture::MonitorMode, HMONITOR> getSelectedMonitor();
        std::unique_ptr<IconBitmapUtils> iconBitmapUtils_;
        std::vector<HBITMAP> bitmaps_;
        std::unique_ptr<DXGIOptionsManager> dxgiOptionsManager_;
        std::unique_ptr<FFMpegOptionsManager> ffmpegOptionsManager_;
        IdNameArray audioSources_;
        ScreenRecordingRuntimeParams recordingParams_;
        HwndVector hwnds_;
        CIcon icon_, iconSmall_;
        CBrush m_WhiteBr;
        MonitorEnumerator monitorEnumerator_;
        void clearBitmaps();
};

