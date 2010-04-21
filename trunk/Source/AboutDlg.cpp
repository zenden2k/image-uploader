/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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

#include "stdafx.h"
#include "resource.h"
#include "aboutdlg.h"
#include "versioninfo.h" 
#include <curl/curl.h>

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	MakeLabelBold(GetDlgItem(IDC_THANKSTOLABEL));
	LogoImage.SubclassWindow(GetDlgItem(IDC_STATICLOGO));
	LogoImage.LoadImage(0, 0, IDR_PNG1, false, GetSysColor(COLOR_BTNFACE));
	SetDlgItemText(IDC_CURLINFOLABEL,(_T("using ") + Utf8ToWstring( curl_version())).c_str());
	
	CString text = CString(_T("v1.2.5."))+_T(BUILD)+_T(" (")+_T(TIME)+_T(")");
	text += _T("\r\n\r\n");
	SetDlgItemText(IDC_IMAGEUPLOADERLABEL, text);
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
