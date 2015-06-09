/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "atlheaders.h"
#include "Func/common.h"
#include "ScreenshotDlg.h"
#include "LogWindow.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Func/MyUtils.h"

// CScreenshotDlg
CScreenshotDlg::CScreenshotDlg()
{
    m_WhiteBr.CreateSolidBrush(RGB(255,255,255));
}

CScreenshotDlg::~CScreenshotDlg()
{
    
}

BOOL SetClientRect(HWND hWnd, int x, int y)
{
    RECT rect = {0,0,x,y}, rect2;
    AdjustWindowRectEx(&rect, GetWindowLong(hWnd,GWL_STYLE), (BOOL)GetMenu(hWnd), GetWindowLong(hWnd, GWL_EXSTYLE));
    GetWindowRect(hWnd, &rect2);
    return MoveWindow(hWnd, rect2.left, rect2.top, rect.right-rect.left,rect.bottom-rect.top, TRUE);
}

#define LOADICO16(x) (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(x), IMAGE_ICON    , 16,16,0)
LRESULT CScreenshotDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CommandBox.SubclassWindow(GetDlgItem(IDC_COMMANDBOX));
    RECT ClientRect;
    GetClientRect(&ClientRect);
    CenterWindow(GetParent());

    CommandBox.m_bHyperLinks = false;
    CommandBox.Init();
    CommandBox.AddString(TR("Capture the Entire Screen"), _T(" "), IDC_FULLSCREEN, LOADICO(IDI_SCREENSHOT));
    CommandBox.AddString(TR("Capture the Active Window"), _T(" "), IDC_SCRACTIVEWINDOW,LOADICO(IDI_WINDOW));
    CommandBox.AddString(TR("Capture Selected Area"), _T(" "), IDC_REGIONSELECT,LOADICO(IDI_ICONREGION));
    CommandBox.AddString(TR("Freehand Capture"), _T(" "), IDC_FREEFORMREGION,LOADICO(IDI_FREEFORM));
    CommandBox.AddString(TR("Capture Selected Object"), _T(" "), IDC_HWNDSREGION,LOADICO(IDI_ICONWINDOWS));
    //CommandBox.AddString(TR(""),0, IDC_VIEWSETTINGS,LOADICO16(IDI_ADDITIONAL));
    //CommandBox.AddString(TR("Close"), 0, IDCANCEL,LOADICO16(IDI_CLOSE),true, 2);
    
    SetWindowText(TR("Screen Capture"));
    TRC(IDC_DELAYLABEL, "Timeout:");
    TRC(IDC_SECLABEL, "sec");
    TRC(IDC_OPENSCREENSHOTINEDITORCHECKBOX, "Open screenshot in the editor");
    GuiTools::SetCheck(m_hWnd, IDC_OPENSCREENSHOTINEDITORCHECKBOX, Settings.ScreenshotSettings.OpenInEditor);
    SetDlgItemInt(IDC_DELAYEDIT, Settings.ScreenshotSettings.Delay);
    SendDlgItemMessage(IDC_DELAYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)30, (short)0) );

    return 0; 
}

LRESULT CScreenshotDlg::OnClickedFullscreenCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmFullScreen;
    return EndDialog(IDOK);
}

LRESULT CScreenshotDlg::OnBnClickedRegionselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmRectangles;
    return EndDialog(IDOK);
}

LRESULT CScreenshotDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return EndDialog(wID);
}

LRESULT CScreenshotDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwndChild)
{
    return (LRESULT)(HBRUSH) m_WhiteBr; // Returning brush solid filled with COLOR_WINDOW color
}

void CScreenshotDlg::ExpandDialog() const
{
    /*m_bExpanded=!m_bExpanded;
    RECT rc, CmdBoxRect;
    CommandBox.GetClientRect(&CmdBoxRect);
    GetClientRect(&rc);

    if(m_bExpanded)
    {
        SetClientRect(m_hWnd, rc.right, nFullWindowHeight);    
    }
    else 
        SetClientRect(m_hWnd, rc.right, CmdBoxRect.bottom);
    EnableNextN(GetDlgItem(IDC_COMMANDBOX),11, m_bExpanded);
    if(m_bExpanded)
        ::SetFocus(GetDlgItem(IDC_DELAYEDIT));*/
}

LRESULT CScreenshotDlg::OnSettingsClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ExpandDialog();
    return 0;
}

LRESULT CScreenshotDlg::OnEnter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CommandBox.NotifyParent((CommandBox.Selected==-1)?0:CommandBox.Selected);
    return 0;
}

LRESULT CScreenshotDlg::OnBnClickedFreeFormRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmFreeform;
    return EndDialog(IDOK);
}

LRESULT CScreenshotDlg::OnClickedActiveWindowCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmActiveWindow;
    return EndDialog(IDOK);
}
        
LRESULT CScreenshotDlg::OnBnClickedWindowHandlesRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_CaptureMode = cmWindowHandles;
    return EndDialog(IDOK);
}
        
CaptureMode CScreenshotDlg::captureMode() const
{
    return m_CaptureMode;
}

LRESULT CScreenshotDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    Settings.ScreenshotSettings.OpenInEditor = GuiTools::GetCheck(m_hWnd, IDC_OPENSCREENSHOTINEDITORCHECKBOX);
    Settings.ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);
    return 0;
}
