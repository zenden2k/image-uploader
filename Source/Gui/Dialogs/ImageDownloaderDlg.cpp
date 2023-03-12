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
#include "ImageDownloaderDlg.h"

#include <boost/format.hpp>

#include "Core/CommonDefs.h"
#include "Core/3rdpart/pcreplusplus.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Core/ServiceLocator.h"
#include "Func/WebUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Core/Utils/StringUtils.h"
#include "Core/AppParams.h"
#include "Core/Network/NetworkClientFactory.h"
#include "Core/DownloadTask.h"
#include "Core/3rdpart/UriParser.h"

namespace {

bool ExtractLinks(CString text, std::vector<CString>& result) {
    pcrepp::Pcre reg("((http|https|ftp)://[\\w\\d:#@%/;$()~_?\\+-=\\\\\\.&]*)", "imcu");
    std::string str = WCstringToUtf8(text);
    size_t pos = 0;
    while (pos <= str.length()) {
        if (reg.search(str, pos)) {
            pos = reg.get_match_end() + 1;
            CString temp = Utf8ToWstring(reg[1]).c_str();
            result.push_back(temp);
        } else
            break;
    }
    return true;
}

}

// CImageDownloaderDlg
CImageDownloaderDlg::CImageDownloaderDlg(CWizardDlg *wizardDlg, const CString &initialBuffer) {
    m_WizardDlg = wizardDlg;
    m_InitialBuffer = initialBuffer;
    m_retCode = 0;
    m_nFilesCount = 0;
    m_nFileDownloaded = 0;
    m_nSuccessfullDownloads = 0;
    isVistaOrLater_ = WinUtils::IsVistaOrLater();
    isRunning_ = false;
    ACCEL accels[] = {
        { FVIRTKEY|FCONTROL , VK_RETURN, IDOK },
        { FVIRTKEY , VK_ESCAPE, IDCANCEL },
    };
    accel_.CreateAcceleratorTable(accels, ARRAY_SIZE(accels));
}

LRESULT CImageDownloaderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();

    AddClipboardFormatListener(m_hWnd);
    
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"
 
    ::SetFocus(GetDlgItem(IDOK));
    SetWindowText(TR("Image Downloader"));
    CString addButtonCaption = TR("Add") + CString(_T(" (Ctrl+Enter)"));
    SetDlgItemText(IDOK, addButtonCaption);
    TRC(IDCANCEL, "Cancel");
    TRC(IDC_WATCHCLIPBOARD, "Watch Clipboard for URLs");
    TRC(IDC_IMAGEDOWNLOADERTIP, "Enter URLs (one http:// or ftp:// link per line)");
    ::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS), SW_HIDE);
    SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_SETCHECK, settings->WatchClipboard?BST_CHECKED:BST_UNCHECKED);

    if(!m_InitialBuffer.IsEmpty())
    {
        ParseBuffer(m_InitialBuffer, false);
        BeginDownloading(); 
    } else {
        CString text;
        if (WinUtils::GetClipboardText(text, m_hWnd)) {
            if (WebUtils::DoesTextLookLikeUrl(text)) {
                SetDlgItemText(IDC_FILEINFOEDIT, text);
            }
        }
    }
    ::SetFocus(GetDlgItem(IDC_FILEINFOEDIT));
    return 0; 
}

LRESULT CImageDownloaderDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RemoveClipboardFormatListener(m_hWnd);
    return 0;
}

LRESULT CImageDownloaderDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    BeginDownloading();
    return 0;
}

LRESULT CImageDownloaderDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (isRunning_) {
        downloadTask_->cancel();
    } else
    {
        auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
        settings->WatchClipboard = SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) != 0;
        EmulateEndDialog(wID);
    }
    return 0;
}

LRESULT CImageDownloaderDlg::OnClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    clipboardUpdated();
    return 0;
}

BOOL CImageDownloaderDlg::PreTranslateMessage(MSG* pMsg) {
    if (accel_.TranslateAccelerator(m_hWnd, pMsg)) {
        return TRUE;
    }
    return FALSE;
}

bool CImageDownloaderDlg::OnFileFinished(bool ok, int statusCode, const DownloadTask::DownloadItem& it)
{
    if(ok)
    {
        CWizardDlg::AddImageStruct ais;
        ais.show =true;
        ais.RealFileName = Utf8ToWstring(it.fileName).c_str();
        ais.VirtualFileName =  Utf8ToWstring(it.displayName).c_str();
        if (ais.VirtualFileName.IsEmpty()) {
            ais.VirtualFileName = WinUtils::myExtractFileName(ais.RealFileName);
        }
        bool add = true;
        if(!IuCommonFunctions::IsImage(ais.RealFileName))
        {
            std::string mimeType = IuCoreUtils::GetFileMimeType(W2U(ais.RealFileName));
            if (mimeType.find("image/") != std::string::npos)
            {
                CString ext = U2W(IuCoreUtils::GetDefaultExtensionForMimeType(mimeType));
                if(!ext.IsEmpty())
                {
                    CString newFileName = ais.RealFileName + _T(".") + ext;
                    MoveFile(ais.RealFileName, newFileName);
                    ais.RealFileName = newFileName;
                    if (CString(WinUtils::GetFileExt(ais.VirtualFileName)).MakeLower() != ext) {
                        ais.VirtualFileName += _T(".") + ext;
                    }
                }
            }
            else 
            {
                add = false;
                
                std::wstring url = IuCoreUtils::Utf8ToWstring(it.url);
                std::wstring mimeTypeW = IuCoreUtils::Utf8ToWstring(mimeType);
                std::wstring errorStr = str(boost::wformat(TR("File is not an image.\nUrl: %s\nMime-Type: %s")) % url % mimeTypeW);
                ServiceLocator::instance()->logger()->write(ILogger::logError, _T("Image Downloader"), errorStr.c_str());
            }
        }
        if (add) {
            if (m_WizardDlg) {
                SendMessage(m_WizardDlg->m_hWnd, WM_MY_ADDIMAGE, reinterpret_cast<WPARAM>(&ais), 0);
            } else {
                m_downloadedFiles.push_back(ais.RealFileName);
            }
            m_nSuccessfullDownloads++;
            
        }
           

    }
    m_nFileDownloaded++;
    SendDlgItemMessage(IDC_DOWNLOADFILESPROGRESS, PBM_SETPOS,  m_nFileDownloaded);
    return true;
}

void CImageDownloaderDlg::OnQueueFinished()
{
    if(!m_InitialBuffer.IsEmpty())
    {
        isRunning_ = false;
        EmulateEndDialog(0);
        WinUtils::CopyTextToClipboard("");
        return;
    }
    ::EnableWindow(GetDlgItem(IDOK),true);
    ::EnableWindow(GetDlgItem(IDC_FILEINFOEDIT),true);
    TRC(IDCANCEL, "Close");
    SetDlgItemText(IDC_FILEINFOEDIT, _T(""));
    ::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS),SW_HIDE);
    ::EnableWindow(GetDlgItem(IDC_WATCHCLIPBOARD),true);
    isRunning_ = false;
}

bool CImageDownloaderDlg::BeginDownloading()
{
    std::string links = W2U(GuiTools::GetWindowText(GetDlgItem(IDC_FILEINFOEDIT)));
    std::vector<std::string> tokens;
    IuStringUtils::Split(links,"\n",tokens,-1);
    m_nFilesCount =0;
    m_nFileDownloaded = 0;
    std::vector<DownloadTask::DownloadItem> downloadItems;
    
    for(size_t i=0; i<tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        if (token.empty() || IuStringUtils::Trim(token).empty()) {
            continue;
        }

        DownloadTask::DownloadItem di;
        di.url = IuStringUtils::Trim(token);
        di.id = reinterpret_cast<void*>(i);
        downloadItems.push_back(di);
        m_nFilesCount++;
    }
    if(m_nFilesCount)
    {
        TRC(IDCANCEL, "Cancel");
        ::EnableWindow(GetDlgItem(IDOK),false);
        ::EnableWindow(GetDlgItem(IDC_WATCHCLIPBOARD),false);
        ::EnableWindow(GetDlgItem(IDC_FILEINFOEDIT),false);
        ::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS),SW_SHOW);
        SendDlgItemMessage(IDC_DOWNLOADFILESPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, m_nFilesCount));
        SendDlgItemMessage(IDC_DOWNLOADFILESPROGRESS, PBM_SETPOS,  0);
        using namespace std::placeholders;

        auto networkClientFactory = ServiceLocator::instance()->networkClientFactory();
        downloadTask_ = std::make_shared<DownloadTask>(networkClientFactory, AppParams::instance()->tempDirectory(), downloadItems);
        downloadTask_->onFileFinished.connect(std::bind(&CImageDownloaderDlg::OnFileFinished, this, _1, _2, _3));
        downloadTask_->onTaskFinished.connect(std::bind(&CImageDownloaderDlg::OnQueueFinished, this));
        isRunning_ = true;
        ServiceLocator::instance()->taskDispatcher()->postTask(downloadTask_);
        return true;
    }
    return false;
}

bool CImageDownloaderDlg::LinksAvailableInText(CString text)
{
    text.Trim();
    if (WebUtils::IsValidUrl(text)) {
        return true;
    }
    std::vector<CString> links;
    ExtractLinks(text,links);
    return !links.empty();
}

void CImageDownloaderDlg::ParseBuffer(CString buffer,bool OnlyImages)
{
    CString text = GuiTools::GetWindowText(GetDlgItem(IDC_FILEINFOEDIT));

    buffer.Trim();
    if (buffer.Find(_T("\n")) == -1) {
        // Text contains just one link
        uriparser::Uri uri(IuCoreUtils::WstringToUtf8(buffer.GetString()));
        if (uri.isValid()) {
            std::string ext = IuCoreUtils::ExtractFileExt(uri.path());
            if (ext.empty() || IuCommonFunctions::IsImage(U2W(uri.path()))) {
                if (!text.IsEmpty() && text.Right(1) != _T("\n")) {
                    text += "\r\n";
                }
                text += buffer + _T("\r\n");
                SetDlgItemText(IDC_FILEINFOEDIT, text);
                return;
            }
        }
    }
    std::vector<CString> links;
    ExtractLinks(buffer,links);
    
    for(size_t i=0; i<links.size(); i++)
    {
        CString fileName = WinUtils::myExtractFileName(links[i]);
        if( ((!OnlyImages && CString(WinUtils::GetFileExt(fileName)).IsEmpty()) || IuCommonFunctions::IsImage(fileName)) &&  text.Find(links[i]) == -1 ) {
            if (!text.IsEmpty() && text.Right(1) != _T("\n")) {
                text += "\r\n";
            }
            text+=links[i]+_T("\r\n");
        }
    }
    SetDlgItemText(IDC_FILEINFOEDIT, text);
}

void CImageDownloaderDlg::clipboardUpdated()
{
    bool IsClipboard = IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;

    if (IsClipboard && SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) == BST_CHECKED && !isRunning_)
    {
        CString str;
        if (WinUtils::GetClipboardText(str, m_hWnd, true))
        {
            ParseBuffer(str, false);
        }
    }
}

const std::vector<CString>& CImageDownloaderDlg::getDownloadedFiles() const {
    return m_downloadedFiles;
}

int CImageDownloaderDlg::EmulateModal(HWND hWndParent, LPARAM dwInitParam)
{
    ATLASSERT(!m_bModal);
    m_retCode = 0;
    ::EnableWindow(hWndParent, FALSE);
    Create(hWndParent, dwInitParam);
    ShowWindow(SW_SHOW);
    m_loop.AddMessageFilter(this);
    m_loop.Run();
    ::EnableWindow(hWndParent, TRUE);
    ShowWindow(SW_HIDE);
    ::SetActiveWindow(hWndParent);
    DestroyWindow();
    return m_retCode;
}

BOOL CImageDownloaderDlg::EmulateEndDialog(int nRetCode) {
    /*if (m_bModal) {
        EndDialog(nRetCode);
    }*/

    m_loop.RemoveMessageFilter(this);
    m_retCode = nRetCode;
    PostMessage(WM_QUIT);
    
    return TRUE;
}

int CImageDownloaderDlg::successfullDownloadsCount() const {
    return m_nSuccessfullDownloads;
}