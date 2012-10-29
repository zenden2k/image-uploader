/*
    Image Uploader - program for uploading images/files to Internet
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

#include "AboutDlg.h"

#include "resource.h"

#include "versioninfo.h"
#include <curl/curl.h>
#include "Func/Settings.h"
#include "Gui/GuiTools.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	GuiTools::MakeLabelBold(GetDlgItem(IDC_THANKSTOLABEL));

	HFONT Font = reinterpret_cast<HFONT>(SendDlgItemMessage(IDC_IMAGEUPLOADERLABEL, WM_GETFONT,0,0));  
	SendDlgItemMessage(IDC_IMAGEUPLOADERLABEL, WM_SETFONT, (LPARAM)GuiTools::MakeFontSmaller(Font), 0);


	LogoImage.SubclassWindow(GetDlgItem(IDC_STATICLOGO));
	LogoImage.SetWindowPos(0, 0,0, 48, 48, SWP_NOMOVE );
	LogoImage.LoadImage(0, 0, IDR_PNG1, false, GetSysColor(COLOR_BTNFACE));
	
	m_WebSiteLink.SubclassWindow(GetDlgItem(IDC_SITELINK));
	m_WebSiteLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 

	m_GoogleCodeLink.SubclassWindow(GetDlgItem(IDC_GOOGLECODELINK));
	m_GoogleCodeLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 

	m_ReportBugLink.SubclassWindow(GetDlgItem(IDC_FOUNDABUG));
	m_ReportBugLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 
	m_ReportBugLink.SetLabel(TR("Нашли ошибку? Сообщите автору"));
	m_ReportBugLink.SetHyperLink(TR("http://code.google.com/p/image-uploader/issues/entry"));


	CString buildInfo = CString("Build ") + _T(BUILD) + _T(" (") + _T(TIME) + _T(")") +
	   (_T("\r\n") + IuCoreUtils::Utf8ToWstring( curl_version())).c_str();

	CString text = CString(TR("v")) + _APP_VER;
	SetDlgItemText(IDC_CURLINFOLABEL, text);
	SetDlgItemText(IDC_IMAGEUPLOADERLABEL, buildInfo);
	CenterWindow(GetParent());

	// Translating
	TRC(IDC_THANKSTOGROUP, "Благодарности");
	TRC(IDC_BETATESTERS, "Бета-тестерам:");
	TRC(IDC_TRANSLATERS, "Переводчикам:");
	TRC(IDC_CONTACTEMAIL, "Контактный e-mail:");
	
	SetWindowText(TR("О программе"));
	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

LRESULT CAboutDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HDC hdcStatic = (HDC) wParam;
	HWND hwndStatic = WindowFromDC(hdcStatic);

	if ( hwndStatic != GetDlgItem(IDC_IMAGEUPLOADERLABEL) ) {
		return 0;
	}
	COLORREF textColor = GetSysColor(COLOR_WINDOWTEXT);
	if ( textColor == 0 ) {
		SetTextColor(hdcStatic, RGB(100,100,100));
		SetBkColor(hdcStatic, GetSysColor(COLOR_BTNFACE));
	}
	

	return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_BTNFACE));
}
