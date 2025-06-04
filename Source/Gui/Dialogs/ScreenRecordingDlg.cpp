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

#include "ScreenRecordingDlg.h"

#include <dxgi1_6.h>

#include "Gui/GuiTools.h"
#include "Func/MyUtils.h"
#include "Core/ScreenCapture/MonitorEnumerator.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Core/ScreenCapture.h"
#include "Gui/IconBitmapUtils.h"

using namespace ScreenCapture;

namespace {

std::vector<CString> GetMonitorsForAdapter(IDXGIAdapter1* pAdapter) {
    std::vector<CString> result;
    std::vector<CComPtr<IDXGIOutput>> outputs;

    CComPtr<IDXGIOutput> pOutput;
    for (UINT i = 0; pAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND; ++i) {
        outputs.push_back(pOutput);
        pOutput.Release(); 
    }

    for (auto& output : outputs) {
        DXGI_OUTPUT_DESC desc;
        if (SUCCEEDED(output->GetDesc(&desc)) && desc.AttachedToDesktop) {
            if (desc.DesktopCoordinates.left == 0 && desc.DesktopCoordinates.top == 0) {
                result.insert(result.begin(), desc.DeviceName);
            } else {
                result.push_back(desc.DeviceName);
            }
        }
    }
    return result;
}

}

CScreenRecordingDlg::CScreenRecordingDlg() {
    m_WhiteBr.CreateSolidBrush(RGB(255,255,255));
    iconBitmapUtils_ = std::make_unique<IconBitmapUtils>();
}

CScreenRecordingDlg::~CScreenRecordingDlg() {
    clearBitmaps();
}

void CScreenRecordingDlg::translateUI() {
    TRC(IDOK, "Start");
    TRC(IDCANCEL, "Cancel");
    TRC(IDC_DELAYLABEL, "Timeout:");
    TRC(IDC_SECLABEL, "sec");
    TRC(IDC_CAPTURECURSORCHECKBOX, "Capture cursor");
    TRC(IDC_AUDIOSOURCELABEL, "Audio source:");
    TRC(IDC_ADAPTERLABEL, "Monitor:");
}

LRESULT CScreenRecordingDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    translateUI();
    DoDataExchange(FALSE);
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    CRect ClientRect;
    GetClientRect(&ClientRect);
    CenterWindow(GetParent());

    icon_.LoadIconMetric(IDI_ICONRECORD, LIM_LARGE);
    iconSmall_.LoadIconMetric(IDI_ICONRECORD, LIM_SMALL);

    SetIcon(icon_, TRUE);
    SetIcon(iconSmall_, FALSE);

    SetWindowText(TR("Screen Recoding"));

    regionSelectButton_.SetButtonStyle(BS_SPLITBUTTON);
    updateRegionSelectButtonTitle();

    delaySpin_.SetRange(0, 90);
    delaySpin_.SetPos(settings->ScreenRecordingSettings.Delay);

    GuiTools::SetCheck(m_hWnd, IDC_CAPTURECURSORCHECKBOX, settings->ScreenRecordingSettings.CaptureCursor);
    /*
    CComPtr<IDXGIFactory1> pFactory;
        HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory);
        if (FAILED(hr))
            return hr;

        CComPtr<IDXGIAdapter1> pAdapter;
        for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 desc;
            hr = pAdapter->GetDesc1(&desc);
            if (SUCCEEDED(hr)) {
                auto monitors = GetMonitorsForAdapter(pAdapter);
                CString title = desc.Description;
                if (!monitors.empty()) {
                    title += " (" + monitors[0] + ")";
                }
                monitorCombobox_.AddString(title);
                /*wprintf(L"Adapter %d: %s\n", i, desc.Description);
                wprintf(L"VendorId: %04X, DeviceId: %04X\n",
                    desc.VendorId, desc.DeviceId);
            }

            pAdapter.Release();
        }
    
*/


   /*m_monitorCombobox.m_hWnd = GetDlgItem(IDC_MONITORSCOMBOBOX);
   */
    int selectedIndex = 0;
    
    int itemIndex = monitorCombobox_.AddString(TR("Current monitor"));

    if (itemIndex >= 0) {
        monitorCombobox_.SetItemData(itemIndex, static_cast<DWORD_PTR>(kCurrentMonitor));
        if (settings->ScreenshotSettings.MonitorMode == kCurrentMonitor) {
            selectedIndex = itemIndex;
        }
    }

    /*itemIndex = monitorCombobox_.AddString(TR("All monitors"));

    if (itemIndex >= 0) {
        monitorCombobox_.SetItemData(itemIndex, static_cast<DWORD_PTR>(kAllMonitors));
        if (settings->ScreenshotSettings.MonitorMode == kAllMonitors) {
            selectedIndex = itemIndex;
        }
    }*/

    int i = 0;

    if (monitorEnumerator_.enumDisplayMonitors(0, 0)) {
        for (const MonitorEnumerator::MonitorInfo& monitor : monitorEnumerator_) {
            CString itemTitle;
            itemTitle.Format(_T("(%dx%d) %s"), monitor.rect.Width(), monitor.rect.Height(), monitor.deviceName);
            
            itemIndex = monitorCombobox_.AddString(itemTitle);
            if (itemIndex >= 0) {
                monitorCombobox_.SetItemData(itemIndex, static_cast<DWORD_PTR>(kSelectedMonitor + i));
                if (settings->ScreenshotSettings.MonitorMode == kSelectedMonitor + i) {
                    selectedIndex = itemIndex;
                }
            }
            i++;
        }
    }

    // Monitor count less than 2
    if (i < 2) {
        //m_monitorCombobox.EnableWindow(FALSE);
        monitorCombobox_.SetCurSel(1);
    } else {
        monitorCombobox_.SetCurSel(selectedIndex);
    }
    
    return 0; 
}

LRESULT CScreenRecordingDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return EndDialog(wID);
}

LRESULT CScreenRecordingDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwndChild)
{
    return reinterpret_cast<LRESULT>(static_cast<HBRUSH>(m_WhiteBr)); // Returning brush solid filled with COLOR_WINDOW color
}

LRESULT CScreenRecordingDlg::OnBnClickedRegionSelectButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    showRegionSelectButtonMenu(hWndCtl);
    return 0;
}

LRESULT CScreenRecordingDlg::OnBnDropdownRegionSelectButton(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
    showRegionSelectButtonMenu(GetDlgItem(idCtrl));
    return 0;
}

LRESULT CScreenRecordingDlg::OnClickedSelectRegion(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    recordingParams_.selectedWindow = NULL;
    updateRegionSelectButtonTitle();
    return 0;
}

LRESULT CScreenRecordingDlg::OnClickedFullScreen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    recordingParams_.selectedRegion.SetRectEmpty();
    recordingParams_.selectedWindow = NULL;
    updateRegionSelectButtonTitle();
    return 0;
}

LRESULT CScreenRecordingDlg::OnClickedWindow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    int index = wID - IDM_WINDOW_LIST_FIRST;
    if (index < 0 || index >= hwnds_.size()) {
        return 0;
    }
    recordingParams_.selectedWindow = hwnds_[index];
    updateRegionSelectButtonTitle();
    return 0;
}

ScreenRecordingRuntimeParams CScreenRecordingDlg::recordingParams() const {
    return recordingParams_;
}

LRESULT CScreenRecordingDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    settings->ScreenRecordingSettings.CaptureCursor = GuiTools::GetCheck(m_hWnd, IDC_CAPTURECURSORCHECKBOX);
    settings->ScreenRecordingSettings.Delay = delaySpin_.GetPos();

    //recordingParams_.monitor = monitorCombobox_.GetCurSel();
    const int itemIndex = monitorCombobox_.GetCurSel();
    if (itemIndex >= 0) {
        int monitorIndex = std::max(0, itemIndex - 1);
        auto info = monitorEnumerator_.getByIndex(monitorIndex);

        recordingParams_.monitorIndex = monitorIndex;
        recordingParams_.monitor = info ? info->monitor : NULL;
    }
    
    return 0;
}

LRESULT CScreenRecordingDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled){
    EndDialog(IDOK);
    return 0;
}

void CScreenRecordingDlg::showRegionSelectButtonMenu(HWND hWndCtl) {
    RECT rc;
    ::GetWindowRect(hWndCtl, &rc);
    POINT menuOrigin = { rc.left, rc.bottom };
    CMenu windowsMenu;
    windowsMenu.CreatePopupMenu();

    hwnds_.clear();
    clearBitmaps();

    EnumWindows([](HWND wnd, LPARAM param) -> BOOL {
        auto* hwnds = reinterpret_cast<HwndVector*>(param);
        if (GuiTools::IsWindowVisibleOnScreen(wnd)) {
            hwnds->push_back(wnd);
        }

        return hwnds->size() < IDM_WINDOW_LIST_LAST - IDM_WINDOW_LIST_FIRST + 1;
    }, reinterpret_cast<LPARAM>(&hwnds_));

    int i = 0;

    for (const auto& hwnd : hwnds_) {
        const CString windowTitle = GuiTools::GetWindowText(hwnd);
        HICON ico = GuiTools::GetWindowIcon(hwnd);
        CString itemTitle;
        itemTitle.Format(_T("[%u] %s"), (uint32_t)hwnd, windowTitle.GetString());
        MENUITEMINFO mi;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
        mi.fType = MFT_STRING;
        mi.wID = IDM_WINDOW_LIST_FIRST + i;
        mi.dwTypeData = const_cast<LPWSTR>(itemTitle.GetString());
        mi.cch = itemTitle.GetLength();
        HBITMAP bm = iconBitmapUtils_->HIconToBitmapPARGB32(ico);
        if (bm) { 
            mi.hbmpItem = bm;
            mi.fMask |= MIIM_BITMAP;
            bitmaps_.push_back(bm);
        }
        windowsMenu.InsertMenuItem(i, true, &mi);
        i++;
        if (i > IDM_WINDOW_LIST_LAST) {
            break;
        }
    }

    CMenu popupMenu;
    popupMenu.CreatePopupMenu();

    popupMenu.AppendMenu(MF_STRING, IDM_SELECT_REGION, TR("Select region..."));
    popupMenu.AppendMenu(MF_STRING, IDM_FULL_SCREEN, TR("Full screen"));
    popupMenu.AppendMenu(0, windowsMenu.Detach(), TR("Window"));

    TPMPARAMS excludeArea;
    ZeroMemory(&excludeArea, sizeof(excludeArea));
    excludeArea.cbSize = sizeof(excludeArea);
    excludeArea.rcExclude = rc;
    popupMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON, menuOrigin.x, menuOrigin.y, m_hWnd, &excludeArea);
}

void CScreenRecordingDlg::updateRegionSelectButtonTitle() {
    CString title;
    if (recordingParams_.selectedWindow) {
        CString text = GuiTools::GetWindowText(recordingParams_.selectedWindow);
        title = WinUtils::TrimString(text, 45);
    } else if (recordingParams_.selectedRegion.IsRectEmpty()) {
        title = TR("Full screen");
    } else {
        title.Format(_T("%d,%d %dx%d"), recordingParams_.selectedRegion.left, recordingParams_.selectedRegion.top, recordingParams_.selectedRegion.Width(), recordingParams_.selectedRegion.Height());
    }
    regionSelectButton_.SetWindowText(title);
}

void CScreenRecordingDlg::clearBitmaps(){
    for (const auto& bitmap : bitmaps_) {
        DeleteObject(bitmap);
    }
    bitmaps_.clear();
}
