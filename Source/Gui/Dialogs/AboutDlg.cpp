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
#include <Func/WinUtils.h>
#include "Func/IuCommonFunctions.h"
#include <libavutil/ffversion.h>
LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	GuiTools::MakeLabelBold(GetDlgItem(IDC_THANKSTOLABEL));

	HFONT Font = reinterpret_cast<HFONT>(SendDlgItemMessage(IDC_IMAGEUPLOADERLABEL, WM_GETFONT,0,0));  
	//SendDlgItemMessage(IDC_IMAGEUPLOADERLABEL, WM_SETFONT, (LPARAM)GuiTools::MakeFontSmaller(Font), 0);


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
	m_ReportBugLink.SetHyperLink(_T("http://code.google.com/p/image-uploader/issues/entry"));

	m_Documentation.SubclassWindow(GetDlgItem(IDC_DOCUMENTATION));
	m_Documentation.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
	m_Documentation.SetLabel(TR("Документация"));

	CString memoText;
	memoText += TR("Благодарности:") + CString("\r\n\r\n");
	memoText += TR("Бета-тестерам:")+ CString("\r\n");
	memoText += L"Graf, CKA3O4H1K\r\n\r\n";
	memoText += TR("Переводчикам:")+ CString("\r\n");
	memoText += L"Mishunika, Adrianis, \r\nHessam Mohamadi, ozzii.translate@***.com\r\n\r\n";

	memoText += L"Thanks to the authors of the following open-source libraries:\r\n\r\n";
 
	
	memoText += CString(L"WTL")+ "\t\thttp://sourceforge.net/projects/wtl/\r\n";
	memoText += CString(L"libcurl")+ "\t\thttp://curl.haxx.se/libcurl/\r\n";
	
	memoText += CString(L"openssl") +"\t\thttps://www.openssl.org\r\n";
	memoText += CString(L"zlib") +"\t\thttp://www.zlib.net\r\n";
	memoText += CString(L"pcre") +"\t\thttp://www.pcre.org\r\n";
	memoText += CString(L"pcre++") +"\t\thttp://www.daemon.de/PCRE\r\n";
	memoText += CString(L"squirrel") +"\t\thttp://squirrel-lang.org\r\n";
	memoText += CString(L"sqplus") +"\t\thttp://sourceforge.net/projects/sqplus/\r\n";
	memoText += CString(L"ffmpeg") +"\t\thttps://www.ffmpeg.org\r\n";	
	memoText += CString(L"tinyxml") +"\t\thttp://sourceforge.net/projects/tinyxml/\r\n";
	memoText += CString(L"zthreads") +"\t\thttp://zthread.sourceforge.net\r\n";
	memoText += CString(L"glog") +"\t\thttps://github.com/google/glog\r\n";


	memoText += CString(L"minizip") +"\t\thttp://www.winimage.com/zLibDll/minizip.html\r\n";
	memoText += CString(L"jsoncpp") +"\t\thttps://github.com/open-source-parsers/jsoncpp\r\n";
	memoText += CString(L"FastDelegate") +"\thttp://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible\r\n";
	memoText += CString(L"QColorQuantizer") +"\thttp://www.codeguru.com/cpp/g-m/gdi/gdi/article.php/c3677/Better-GIFs-with-Octrees.htm\r\n\r\n";

	
	memoText += TR("Libcurl version:")+ CString("\r\n");
	memoText +=  IuCoreUtils::Utf8ToWstring( curl_version()).c_str() + CString("\r\n\r\n");
	if ( Settings.IsFFmpegAvailable() ) {
		memoText += TR("FFmpeg version:")+ CString("\r\n");
		memoText += FFMPEG_VERSION + CString("\r\n");
	}

	SetDlgItemText(IDC_MEMO, memoText);

	CString buildInfo = CString("Build ") + _T(BUILD) + _T(" (") + _T(TIME) + _T(")") +
	   _T("\r\n");

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

LRESULT CAboutDlg::OnDocumentationClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShellExecute(0,L"open",WinUtils::GetAppFolder()+"Docs\\index.html",0,WinUtils::GetAppFolder()+"Docs\\",SW_SHOWNORMAL);
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
