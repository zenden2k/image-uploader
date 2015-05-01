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

#include "AboutDlg.h"

#include "resource.h"

#include "versioninfo.h"
#include <curl/curl.h>
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Func/IuCommonFunctions.h"
#include <libavutil/ffversion.h>
#include <boost/config.hpp>
#include <boost/version.hpp>

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    
    GuiTools::MakeLabelBold(GetDlgItem(IDC_THANKSTOLABEL));

    HFONT Font = reinterpret_cast<HFONT>(SendDlgItemMessage(IDC_IMAGEUPLOADERLABEL, WM_GETFONT,0,0));  
    //SendDlgItemMessage(IDC_IMAGEUPLOADERLABEL, WM_SETFONT, (LPARAM)GuiTools::MakeFontSmaller(Font), 0);

    LogoImage.SubclassWindow(GetDlgItem(IDC_STATICLOGO));
    LogoImage.SetWindowPos(0, 0,0, 48, 48, SWP_NOMOVE );
    LogoImage.LoadImage(0, 0, Settings.UseNewIcon ? IDR_ICONMAINNEW : IDR_PNG1, false, GetSysColor(COLOR_BTNFACE));
    
    m_WebSiteLink.SubclassWindow(GetDlgItem(IDC_SITELINK));
    m_WebSiteLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 

    m_GoogleCodeLink.SubclassWindow(GetDlgItem(IDC_GOOGLECODELINK));
    m_GoogleCodeLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 

    m_ReportBugLink.SubclassWindow(GetDlgItem(IDC_FOUNDABUG));
    m_ReportBugLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 
    m_ReportBugLink.SetLabel(TR("Нашли ошибку? Сообщите автору"));
    m_ReportBugLink.SetHyperLink(_T("https://github.com/zenden2k/image-uploader/issues"));

    m_Documentation.SubclassWindow(GetDlgItem(IDC_DOCUMENTATION));
    m_Documentation.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
    m_Documentation.SetLabel(TR("Документация"));

    CString memoText;
    memoText += TR("Путь к файлу с настройками:") + CString(_T("\r\n"))+ Settings.getSettingsFileName() + _T("\r\n\r\n");
    memoText += TR("Благодарности:") + CString("\r\n\r\n");
    memoText += TR("Контрибьюторам:") + CString("\r\n");
    memoText += L"arhangelsoft\thttps://github.com/arhangelsoft\r\nTahir Yilmaz\thttps://github.com/thrylmz\r\nAlex_Qwerty\r\n\r\n";

    memoText += TR("Переводчикам:")+ CString("\r\n");

    CString translatorName = TR("translator_name");
    if ( translatorName == "translator_name") {
        translatorName.Empty();
    }
    if ( !translatorName.IsEmpty() ) {
        CString trans;
        trans.Format(TR("%s translation:"),Lang.GetLanguageName());
        memoText += L"\r\n"+ trans + L"\r\n"+ translatorName +L"\r\n\r\n";
        memoText += L"Other languages: \r\n";
    } 
    memoText += L"Mishunika, Adrianis, \r\nHessam Mohamadi, ozzii.translate@***.com\r\n\r\n";

    memoText += TR("Бета-тестерам:")+ CString("\r\n");
    memoText += L"Graf, CKA3O4H1K, Geslot, Alex_Qwerty\r\n\r\n";

    //memoText += CString(T_R("Thanks to the authors of the following open-source libraries:"))+L"\r\n\r\n";
    memoText += CString(TR("Спасибо авторам следующих библиотек с открытым исходным кодом:"))+L"\r\n\r\n";
    
    memoText += CString(L"WTL")+ "\t\thttp://sourceforge.net/projects/wtl/\r\n";
    memoText += CString(L"libcurl")+ "\t\thttp://curl.haxx.se/libcurl/\r\n";
    
    memoText += CString(L"openssl") +"\t\thttps://www.openssl.org\r\n";
    memoText += CString(L"zlib") +"\t\thttp://www.zlib.net\r\n";
    memoText += CString(L"squirrel") +"\t\thttp://squirrel-lang.org\r\n";
    memoText += CString(L"sqrat") +"\t\thttp://scrat.sourceforge.net\r\n";
    memoText += CString(L"ffmpeg") +"\t\thttps://www.ffmpeg.org\r\n";    
    memoText += CString(L"MediaInfo") +"\thttps://mediaarea.net/\r\n";
    memoText += CString(L"pcre") +"\t\thttp://www.pcre.org\r\n";
    memoText += CString(L"pcre++") +"\t\thttp://www.daemon.de/PCRE\r\n";
    
    memoText += CString(L"tinyxml") +"\t\thttp://sourceforge.net/projects/tinyxml/\r\n";
    memoText += CString(L"glog") +"\t\thttps://github.com/google/glog\r\n";


    memoText += CString(L"minizip") +"\t\thttp://www.winimage.com/zLibDll/minizip.html\r\n";
    memoText += CString(L"jsoncpp") +"\t\thttps://github.com/open-source-parsers/jsoncpp\r\n";
    memoText += CString(L"Boost") +"\t\thttp://www.boost.org\r\n";
    memoText += CString(L"FastDelegate") +"\thttp://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible\r\n";
    memoText += CString(L"QColorQuantizer") +"\thttp://www.codeguru.com/cpp/g-m/gdi/gdi/article.php/c3677/Better-GIFs-with-Octrees.htm\r\n";
    memoText += CString(L"WTL Browser") +"\thttp://www.codeproject.com/Articles/7147/WTL-Browser\r\n";
    memoText += CString(L"CRegistry") +"\t\thttp://www.codeproject.com/Articles/19/Registry-Class\r\n";
    memoText += CString(L"TParser") +"\t\thttps://rsdn.ru/article/files/Classes/tparser.xml\r\n";
    memoText += CString(L"CWinHotkeyCtrl") +"\thttps://rsdn.ru/article/controls/WinHotkeyCtrl.xml\r\n";
    memoText += CString(L"UTF-8 CPP") +"\thttp://sourceforge.net/projects/utfcpp/\r\n";
    memoText += CString(L"CUnzipper") +"\thttp://www.codeproject.com/Articles/4288/Win-Wrapper-classes-for-Gilles-Volant-s-Zip-Unzi\r\n";
    memoText += CString(L"CThread") +"\t\thttp://www.viksoe.dk/code/thread.htm\r\n";
    memoText += CString(L"CPropertyList") +"\thttp://www.viksoe.dk/code/propertylist.htm\r\n";
    memoText += CString(L"GDI+ helper") +"\thttp://www.codeproject.com/Articles/4969/GDI-and-MFC-memory-leak-detection\r\n";
    memoText += CString(_T("Resources:\r\n")) +
        _T("famfamfam icons\thttp://www.famfamfam.com/lab/icons/\r\n\r\n");

    memoText +=  CString(L"Built with: \r\n") + CString(BOOST_COMPILER) +  _T("\r\n");;
    memoText +=  CString(L"Target platform: ") + BOOST_PLATFORM + _T(" (") + WinUtils::IntToStr(sizeof(void*) * CHAR_BIT) + _T(" bit)\r\n");
    memoText +=  CString(L"Date: ") + CString(TIME) +  _T("\r\n\r\n");
    memoText += TR("Libcurl version:")+ CString("\r\n");
    memoText +=  IuCoreUtils::Utf8ToWstring( curl_version()).c_str() + CString("\r\n\r\n");
    CString versionLabel;
    versionLabel.Format(_T("%s version:"), _T("Boost"));
    memoText += versionLabel + CString("\r\n");
    CString boostVersion = CString(BOOST_LIB_VERSION);
    boostVersion.Replace(L'_', L'.');
    memoText += boostVersion  + L"\r\n\r\n";

    
    /*if ( Settings.IsFFmpegAvailable() ) { // Can't determine actual ffmpeg version
        memoText += TR("FFmpeg version:")+ CString("\r\n");
        memoText += FFMPEG_VERSION + CString("\r\n");
    }*/

    SetDlgItemText(IDC_MEMO, memoText);

    CString buildInfo = CString("Build ") + _T(BUILD) + _T(" (") + _T(TIME) + _T(")") +
       _T("\r\n");

    CString text = CString(TR("v")) + _APP_VER;
    SetDlgItemText(IDC_CURLINFOLABEL, text);
    SetDlgItemText(IDC_IMAGEUPLOADERLABEL, buildInfo);
    CenterWindow(GetParent());


    TRC(IDC_CONTACTEMAIL, "Контактный e-mail:");
    TRC(IDC_AUTHORLABEL, "Автор:");
    TRC(IDC_WEBSITELABEL, "Веб-сайт:");
    
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
