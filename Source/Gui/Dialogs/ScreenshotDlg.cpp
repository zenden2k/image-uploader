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

#include "ScreenshotDlg.h"

#include "Gui/GuiTools.h"
#include "Func/MyUtils.h"
#include "ScreenCapture/MonitorEnumerator.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/Helpers/DPIHelper.h"

using namespace ScreenCapture;

// CScreenshotDlg
CScreenshotDlg::CScreenshotDlg(bool enableLastRegionScreenshot)
    : m_CaptureMode(cmFullScreen)
    , enableLastRegionScreenshot_(enableLastRegionScreenshot)
{
    m_WhiteBr.CreateSolidBrush(RGB(255,255,255));
}

LRESULT CScreenshotDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    commandBox_.SubclassWindow(GetDlgItem(IDC_COMMANDBOX));
    CRect clientRect;
    GetClientRect(&clientRect);
    CenterWindow(GetParent());

    fillCommandBox();

    SetWindowText(TR("Screen Capture"));
    TRC(IDC_DELAYLABEL, "Timeout:");
    TRC(IDC_SECLABEL, "sec");
    TRC(IDC_OPENSCREENSHOTINEDITORCHECKBOX, "Open screenshot in the editor");
    TRC(IDC_CAPTURECURSORCHECKBOX, "Capture cursor");

    GuiTools::SetCheck(m_hWnd, IDC_OPENSCREENSHOTINEDITORCHECKBOX, settings->ScreenshotSettings.OpenInEditor);
    GuiTools::SetCheck(m_hWnd, IDC_CAPTURECURSORCHECKBOX, settings->ScreenshotSettings.CaptureCursor);
    SetDlgItemInt(IDC_DELAYEDIT, settings->ScreenshotSettings.Delay);
    SendDlgItemMessage(IDC_DELAYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)30, (short)0) );

    CRect commandBoxRect;
    commandBox_.GetClientRect(commandBoxRect);
    const int dpi = DPIHelper::GetDpiForDialog(m_hWnd);
    int newHeight = commandBox_.desiredHeight() + MulDiv(135, dpi, USER_DEFAULT_SCREEN_DPI);
    GuiTools::SetClientRect(m_hWnd, clientRect.Width(), newHeight);

    m_monitorCombobox.m_hWnd = GetDlgItem(IDC_MONITORSCOMBOBOX);

    int selectedIndex = 0;

    int itemIndex = m_monitorCombobox.AddString(TR("Current monitor"));

    if (itemIndex >= 0) {
        m_monitorCombobox.SetItemData(itemIndex, static_cast<DWORD_PTR>(kCurrentMonitor));
        if (settings->ScreenshotSettings.MonitorMode == kCurrentMonitor) {
            selectedIndex = itemIndex;
        }
    }

    itemIndex =  m_monitorCombobox.AddString(TR("All monitors"));

    if (itemIndex >= 0) {
        m_monitorCombobox.SetItemData(itemIndex, static_cast<DWORD_PTR>(kAllMonitors));
        if (settings->ScreenshotSettings.MonitorMode == kAllMonitors) {
            selectedIndex = itemIndex;
        }
    }


    MonitorEnumerator enumerator;
    int i = 0;

    if (enumerator.enumDisplayMonitors(0, 0)) {

        for (const MonitorEnumerator::MonitorInfo& monitor : enumerator) {
            CString itemTitle;
            itemTitle.Format(_T("%s %d (%dx%d)"), TR("Monitor"), i + 1, monitor.rect.Width(), monitor.rect.Height());
            //
            itemIndex = m_monitorCombobox.AddString(itemTitle);
            if (itemIndex >= 0) {
                m_monitorCombobox.SetItemData(itemIndex, static_cast<DWORD_PTR>(kSelectedMonitor + i));
                if (settings->ScreenshotSettings.MonitorMode == kSelectedMonitor + i) {
                    selectedIndex = itemIndex;
                }
            }
            i++;
        }
    }

    // Monitor count less than 2
    if (i < 2) {
        m_monitorCombobox.EnableWindow(FALSE);
        m_monitorCombobox.SetCurSel(1);
    } else {
        m_monitorCombobox.SetCurSel(selectedIndex);
    }

    return 0;
}

LRESULT CScreenshotDlg::OnClickedFullscreenCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmFullScreen;
    endDialogOK();
    return 0;
}

LRESULT CScreenshotDlg::OnBnClickedRegionselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmRectangles;
    endDialogOK();
    return 0;
}

LRESULT CScreenshotDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return EndDialog(wID);
}

LRESULT CScreenshotDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwndChild)
{
    return reinterpret_cast<LRESULT>(static_cast<HBRUSH>(m_WhiteBr)); // Returning brush solid filled with COLOR_WINDOW color
}

LRESULT CScreenshotDlg::OnEnter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    commandBox_.NotifyParent((commandBox_.selectedItemIndex()==-1)?0:commandBox_.selectedItemIndex());
    return 0;
}

LRESULT CScreenshotDlg::OnBnClickedFreeFormRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmFreeform;
    endDialogOK();
    return 0;
}

LRESULT CScreenshotDlg::OnClickedActiveWindowCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmActiveWindow;
    endDialogOK();
    return 0;
}

LRESULT CScreenshotDlg::OnBnClickedWindowHandlesRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmWindowHandles;
    endDialogOK();
    return 0;
}

LRESULT CScreenshotDlg::OnBnClickedTopWindowRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    m_CaptureMode = cmTopWindowHandles;
    endDialogOK();
    return 0;
}

LRESULT CScreenshotDlg::OnBnClickedLastRegion(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    m_CaptureMode = cmLastRegion;
    endDialogOK();
    return 0;
}

void CScreenshotDlg::fillCommandBox() {
    commandBox_.resetContent(false);
    const int dpi = DPIHelper::GetDpiForDialog(m_hWnd);

    const int iconBigWidth = DPIHelper::GetSystemMetricsForDpi(SM_CXICON, dpi);
    const int iconBigHeight = DPIHelper::GetSystemMetricsForDpi(SM_CYICON, dpi);

    auto loadBigIcon = [&](int resourceId) -> HICON {
        CIconHandle icon;
        icon.LoadIconWithScaleDown(MAKEINTRESOURCE(resourceId), iconBigWidth, iconBigHeight);
        return icon.m_hIcon;
    };

    commandBox_.m_bHyperLinks = false;
    commandBox_.Init(GetSysColor(COLOR_WINDOW));
    commandBox_.AddString(TR("Capture the Entire Screen"), _T(" "), IDC_FULLSCREEN, loadBigIcon(IDI_SCREENSHOT));
    commandBox_.AddString(TR("Capture the Active Window"), _T(" "), IDC_SCRACTIVEWINDOW, loadBigIcon(IDI_WINDOW));
    commandBox_.AddString(TR("Capture Selected Area"), _T(" "), IDC_REGIONSELECT, loadBigIcon(IDI_ICONREGION));
    commandBox_.AddString(TR("Freehand Capture"), _T(" "), IDC_FREEFORMREGION, loadBigIcon(IDI_FREEFORM));
    commandBox_.AddString(TR("Capture Selected Window"), _T(" "), IDC_TOPWINDOWREGION, loadBigIcon(IDI_ICONWINDOWS));
    commandBox_.AddString(TR("Capture Selected Object"), _T(" "), IDC_HWNDSREGION, loadBigIcon(IDI_ICONCONTROLS));

    if (enableLastRegionScreenshot_) {
        commandBox_.AddString(TR("Capture Last Region"), _T(" "), IDC_LASTREGIONSCREENSHOT, loadBigIcon(IDI_ICONLASTREGION));
    }
    commandBox_.Invalidate();
}

BOOL CScreenshotDlg::endDialogOK() {
    GuiTools::DisableDwmAnimations(m_hWnd);
    return EndDialog(IDOK);
}

CaptureMode CScreenshotDlg::captureMode() const
{
    return m_CaptureMode;
}

LRESULT CScreenshotDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    settings->ScreenshotSettings.OpenInEditor = GuiTools::GetCheck(m_hWnd, IDC_OPENSCREENSHOTINEDITORCHECKBOX);
    settings->ScreenshotSettings.CaptureCursor = GuiTools::GetCheck(m_hWnd, IDC_CAPTURECURSORCHECKBOX);
    settings->ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);

    const int itemIndex = m_monitorCombobox.GetCurSel();
    if (itemIndex >= 0) {
        auto monitorMode = static_cast<MonitorMode>(m_monitorCombobox.GetItemData(itemIndex));
        settings->ScreenshotSettings.MonitorMode = static_cast<int>(monitorMode);
    }

    return 0;
}

LRESULT CScreenshotDlg::OnDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    commandBox_.SendMessage(WM_MY_DPICHANGED, wParam);
    fillCommandBox();
    return 0;
}
