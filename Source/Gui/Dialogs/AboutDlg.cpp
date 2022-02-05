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

#include "AboutDlg.h"

#include <curl/curl.h>
#include <sqlite3.h>
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include <libavutil/ffversion.h>
#include <boost/config.hpp>
#include <boost/version.hpp>
#include <webp/decode.h>
#include "Core/AppParams.h"
#include "Func/MediaInfoHelper.h"
#include "Core/Settings/WtlGuiSettings.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    thanksToLabelFont_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_THANKSTOLABEL));
    LogoImage.SubclassWindow(GetDlgItem(IDC_STATICLOGO));
    LogoImage.SetWindowPos(0, 0,0, 48, 48, SWP_NOMOVE|SWP_NOZORDER );
    LogoImage.LoadImage(0, 0, IDR_ICONMAINNEW, false, GetSysColor(COLOR_BTNFACE));

    auto* ver = AppParams::instance()->GetAppVersion();
    auto* translator = ServiceLocator::instance()->translator();

    m_WebSiteLink.SubclassWindow(GetDlgItem(IDC_SITELINK));
    m_WebSiteLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 

    m_GoogleCodeLink.SubclassWindow(GetDlgItem(IDC_GOOGLECODELINK));
    m_GoogleCodeLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 

    m_ReportBugLink.SubclassWindow(GetDlgItem(IDC_FOUNDABUG));
    m_ReportBugLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER; 
    m_ReportBugLink.SetLabel(TR("Found a bug? Send it to the author"));
    m_ReportBugLink.SetHyperLink(_T("https://github.com/zenden2k/image-uploader/issues"));

    m_CommitHashLink.SubclassWindow(GetDlgItem(IDC_COMMITHASH));
    m_CommitHashLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER;
    m_CommitHashLink.SetLabel(CString(ver->CommitHashShort.c_str()));
    m_CommitHashLink.SetHyperLink(CString(("https://github.com/zenden2k/image-uploader/commit/" + ver->CommitHash).c_str()));

    m_EmailLink.SubclassWindow(GetDlgItem(IDC_AUTHORNAMELABEL));
    m_EmailLink.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER;
    m_EmailLink.SetLabel((translator->getCurrentLanguage() == "Russian" ? U2W("\xD0\xA1\xD0\xB5\xD1\x80\xD0\xB3\xD0\xB5\xD0\xB9\x20\xD0\xA1\xD0\xB2\xD0\xB8\xD1\x81\xD1\x82\xD1\x83\xD0\xBD\xD0\xBE\xD0\xB2")
        : _T("Sergey Svistunov")) + CString(_T(" (zenden2k@gmail.com)")));
    m_EmailLink.SetHyperLink(_T("mailto:zenden2k@gmail.com"));

    CString memoText;
    
    memoText += TR("Settings file path:") + CString(_T("\r\n"))+ settings->getSettingsFileName() + _T("\r\n\r\n");
    memoText += TR("Thanks to:") + CString("\r\n\r\n");
    memoText += TR("Contributors:") + CString("\r\n");
    memoText += L"arhangelsoft\thttps://github.com/arhangelsoft\r\nTahir Yilmaz\thttps://github.com/thrylmz\r\nAlex_Qwerty\r\n\r\n";
   
    CString translatorName = TR("translator_name");
    if ( translatorName == "translator_name") {
        translatorName.Empty();
    }
    if ( !translatorName.IsEmpty() ) {
        memoText += TR("Translators:") + CString("\r\n");

        CString trans;
        trans.Format(TR("%s translation:"), (LPCTSTR)U2W(translator->getCurrentLanguage()));
        memoText += L"\r\n"+ trans + L"\r\n"+ translatorName +L"\r\n\r\n";
    } 

    memoText += TR("Beta-testers:")+ CString("\r\n");
    memoText += L"Graf, CKA3O4H1K, Geslot, Alex_Qwerty\r\n\r\n";

    //memoText += CString(T_R("Thanks to the authors of the following open-source libraries:"))+L"\r\n\r\n";
    memoText += CString(TR("Thanks to the authors of the following open-source libraries:"))+L"\r\n\r\n";
    // \u200E is Unicode Left-To-Right order mark
    // We need to use it to display URLs properly when RTL language is chosen
    memoText += CString(L"WTL")+ L"\t\thttp://sourceforge.net/projects/wtl/\u200E\r\n";
    memoText += CString(L"libcurl") + L"\t\thttp://curl.haxx.se/libcurl/\u200E\r\n";
//#ifdef USE_OPENSSL
    memoText += CString(L"openssl") + L"\t\thttps://www.openssl.org\r\n";
//#endif
    memoText += CString(L"zlib") + L"\t\thttp://www.zlib.net\r\n";
    memoText += CString(L"squirrel") + L"\t\thttp://squirrel-lang.org\r\n";
    memoText += CString(L"sqrat") + L"\t\thttp://scrat.sourceforge.net\r\n";
    memoText += CString(L"ffmpeg") + L"\t\thttps://www.ffmpeg.org\r\n";
    memoText += CString(L"MediaInfo") + L"\thttps://mediaarea.net/\u200E\r\n";
    memoText += CString(L"pcre") + L"\t\thttp://www.pcre.org\r\n";
    memoText += CString(L"pcre++") + L"\t\thttp://www.daemon.de/PCRE\r\n";
    
    memoText += CString(L"tinyxml") + L"\t\thttp://sourceforge.net/projects/tinyxml/\u200E\r\n";
    memoText += CString(L"gumbo parser") + L"\thttps://github.com/google/gumbo-parser\r\n";
    memoText += CString(L"gumbo-query") + L"\thttps://github.com/lazytiger/gumbo-query\r\n";
    memoText += CString(L"glog") + L"\t\thttps://github.com/google/glog\r\n";
    memoText += CString(L"libwebp") + L"\t\thttps://github.com/webmproject/libwebp\r\n";


    memoText += CString(L"minizip") + L"\t\thttp://www.winimage.com/zLibDll/minizip.html\r\n";
    memoText += CString(L"jsoncpp") + L"\t\thttps://github.com/open-source-parsers/jsoncpp\r\n";
    memoText += CString(L"Boost") + L"\t\thttp://www.boost.org\r\n";
    memoText += CString(L"FastDelegate") + L"\thttp://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible\r\n";
    memoText += CString(L"QColorQuantizer") + L"\thttp://www.codeguru.com/cpp/g-m/gdi/gdi/article.php/c3677/Better-GIFs-with-Octrees.htm\r\n";
    memoText += CString(L"WTL Browser") + L"\thttp://www.codeproject.com/Articles/7147/WTL-Browser\r\n";
    memoText += CString(L"CRegistry") + L"\t\thttp://www.codeproject.com/Articles/19/Registry-Class\r\n";
    memoText += CString(L"TParser") + L"\t\thttps://rsdn.ru/article/files/Classes/tparser.xml\r\n";
    memoText += CString(L"CWinHotkeyCtrl") + L"\thttps://rsdn.ru/article/controls/WinHotkeyCtrl.xml\r\n";
    memoText += CString(L"UTF-8 CPP") + L"\thttp://sourceforge.net/projects/utfcpp/\u200E\r\n";
    memoText += CString(L"CUnzipper") + L"\thttp://www.codeproject.com/Articles/4288/Win-Wrapper-classes-for-Gilles-Volant-s-Zip-Unzi\r\n";
    memoText += CString(L"CThread") + L"\t\thttp://www.viksoe.dk/code/thread.htm\r\n";
    memoText += CString(L"CPropertyList") + L"\thttp://www.viksoe.dk/code/propertylist.htm\r\n";
    memoText += CString(L"uriparser") + L"\t\thttp://uriparser.sourceforge.net/\u200E\r\n";
    memoText += CString(L"GDI+ helper") + L"\thttp://www.codeproject.com/Articles/4969/GDI-and-MFC-memory-leak-detection\r\n";
    memoText += CString(L"xbbcode") + L"\t\thttps://github.com/patorjk/Extendible-BBCode-Parser\r\n";
    memoText += CString(L"entities.c") + L"\t\thttps://bitbucket.org/cggaertner/cstuff/\u200E\r\n";
    memoText += CString(L"base64") + L"\t\thttps://github.com/aklomp/base64/\u200E\r\n";
    memoText += CString(L"Mega SDK") + L"\thttps://github.com/meganz/sdk\r\n";
    memoText += CString(L"Crypto++") + L" \thttps://www.cryptopp.com/\u200E\r\n";
    memoText += CString(L"c-ares") + L"\t\thttps://c-ares.haxx.se/\u200E\r\n";
    memoText += CString(L"libuv") + L"\t\thttps://github.com/libuv/libuv\r\n";
            
    memoText += CString(_T("Resources:\r\n")) +
        _T("famfamfam icons\thttp://www.famfamfam.com/lab/icons/\u200E\r\n\r\n");
    memoText += CString(L"Build date: ") + CString(ver->BuildDate.c_str()) + _T("\r\n");
    memoText +=  CString(L"Built with: \r\n") + CString(BOOST_COMPILER) +  _T("\r\n");
    memoText +=  CString(L"Target platform: ") + BOOST_PLATFORM + _T(" (") + WinUtils::IntToStr(sizeof(void*) * CHAR_BIT) + _T(" bit)\r\n\r\n");
    memoText += TR("Libraries:")+ CString("\r\n");
    memoText +=  IuCoreUtils::Utf8ToWstring( curl_version()).c_str() + CString("\r\n");
    CString boostVersion = CString(BOOST_LIB_VERSION);
    boostVersion.Replace(L'_', L'.');
    CString versionLabel;
    versionLabel.Format(_T("%s version: "), _T("Boost"));
    memoText += versionLabel + boostVersion + CString("\r\n");
    
    int webpVersion = WebPGetDecoderVersion();
    CString webpVersionStr;
    webpVersionStr.Format(_T("%u.%u.%u"), (webpVersion >> 16) & 0xff, (webpVersion >> 8) & 0xff, webpVersion & 0xff);

    memoText += CString(L"libwebp: v") + webpVersionStr + L"\r\n";
    memoText += CString(L"sqlite: v") + sqlite3_libversion() + L"\r\n";

    /*if ( Settings.IsFFmpegAvailable() ) { // Can't determine actual ffmpeg version
        memoText += TR("FFmpeg version:")+ CString("\r\n");
        memoText += FFMPEG_VERSION + CString("\r\n");
    }*/

    if (MediaInfoHelper::IsMediaInfoAvailable()) {
        memoText += MediaInfoHelper::GetLibraryVersion() + _T("\r\n\r\n")/* +
            CString(L"MediaInfo.DLL path:\r\n") + MediaInfoHelper::GetLibraryPath() + 
             + _T("\r\n")*/;
    }

    SYSTEMTIME systime;
    memset(&systime, 0, sizeof(systime));
    int fieldNum = sscanf_s(ver->BuildDate.c_str(), "%hu.%hu.%hu", &systime.wDay, &systime.wMonth, &systime.wYear);
    CString dateStr;
    if (fieldNum == 3) {
        WCHAR pFmt[MAX_PATH] = { 0 };
        GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, nullptr, pFmt, MAX_PATH);
        dateStr = pFmt;
    } else {
        dateStr = ver->BuildDate.c_str();
    }

    SetDlgItemText(IDC_MEMO, memoText);
   
    CString buildInfo;
    buildInfo.Format(_T("Build %d"), ver->Build);
#ifdef USE_OPENSSL
    buildInfo += _T(" (with OpenSSL) ");
#endif

    buildInfo  +=  CString(_T("\r\n(")) + dateStr + _T(")");

    CString text = ver->FullVersion.c_str();

    SetDlgItemText(IDC_CURLINFOLABEL, text);
    SetDlgItemText(IDC_IMAGEUPLOADERLABEL, buildInfo);
    CenterWindow(GetParent());

    TRC(IDC_AUTHORLABEL, "Author:");
    TRC(IDC_WEBSITELABEL, "Website:");
    
    SetWindowText(TR("About Image Uploader"));
    return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

LRESULT CAboutDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    HDC hdcStatic = reinterpret_cast<HDC>(wParam);
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
