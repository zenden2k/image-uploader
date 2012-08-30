/*
    Image Uploader - application for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "atlheaders.h"
#include "Func/common.h"
#include "ScreenshotDlg.h"
#include "LogWindow.h"
#include "Func/Settings.h"
#include <Func/Myutils.h>


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

#define LOADICO16(x) (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(x), IMAGE_ICON	, 16,16,0)
LRESULT CScreenshotDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CommandBox.SubclassWindow(GetDlgItem(IDC_COMMANDBOX));
	RECT ClientRect;
	GetClientRect(&ClientRect);
	CenterWindow(GetParent());

	CommandBox.m_bHyperLinks = false;
	CommandBox.Init();
	CommandBox.AddString(TR("Снимок всего экрана"), _T(" "), IDC_FULLSCREEN, LOADICO(IDI_SCREENSHOT));
	CommandBox.AddString(TR("Снимок активного окна"), _T(" "), IDC_SCRACTIVEWINDOW,LOADICO(IDI_WINDOW));
	CommandBox.AddString(TR("Снимок прямоугольной области..."), _T(" "), IDC_REGIONSELECT,LOADICO(IDI_ICONREGION));
	CommandBox.AddString(TR("Снимок произвольной области..."), _T(" "), IDC_FREEFORMREGION,LOADICO(IDI_FREEFORM));
	CommandBox.AddString(TR("Снимок элемента управления..."), _T(" "), IDC_HWNDSREGION,LOADICO(IDI_ICONWINDOWS));
	//CommandBox.AddString(TR(""),0, IDC_VIEWSETTINGS,LOADICO16(IDI_ADDITIONAL));
	//CommandBox.AddString(TR("Закрыть"), 0, IDCANCEL,LOADICO16(IDI_CLOSE),true, 2);
	
	SetWindowText(TR("Снимок экрана"));
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