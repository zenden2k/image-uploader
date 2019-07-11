/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

// CImageDownloaderDlg
CImageDownloaderDlg::CImageDownloaderDlg(CWizardDlg *wizardDlg, const CString &initialBuffer)
                                            :m_FileDownloader(std::make_shared<NetworkClientFactory>(), AppParams::instance()->tempDirectory())                                    
{
    m_WizardDlg = wizardDlg;
    m_InitialBuffer = initialBuffer;
    fRemoveClipboardFormatListener_ = NULL;
    PrevClipboardViewer = NULL;
    m_retCode = 0;
    m_nFilesCount = 0;
    m_nFileDownloaded = 0;
    isVistaOrLater_ = WinUtils::IsVistaOrLater();
    m_FileDownloader.setThreadCount(1);
    ACCEL accels[] = {
        { FVIRTKEY|FCONTROL , VK_RETURN, IDOK },
        { FVIRTKEY , VK_ESCAPE, IDCANCEL },
    };
    accel_.CreateAcceleratorTable(accels, ARRAY_SIZE(accels));
}

LRESULT CImageDownloaderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();

    if (isVistaOrLater_)
    {
        HMODULE module = GetModuleHandle(_T("user32.dll"));
        AddClipboardFormatListenerFunc fAddClipboardFormatListener = reinterpret_cast<AddClipboardFormatListenerFunc>(GetProcAddress(module, "AddClipboardFormatListener"));
        fAddClipboardFormatListener(m_hWnd);
        fRemoveClipboardFormatListener_ = reinterpret_cast<RemoveClipboardFormatListenerFunc>(GetProcAddress(module, "RemoveClipboardFormatListener"));
    } else {
        PrevClipboardViewer = SetClipboardViewer(); // using old fragile cliboard listening method on pre Vista systems
    }
    
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"
 
    ::SetFocus(GetDlgItem(IDOK));
    SetWindowText(TR("Image Downloader"));
    CString addButtonCaption = TR("Add") + CString(_T(" (Ctrl+Enter)"));
    SetDlgItemText(IDOK, addButtonCaption);
    TRC(IDCANCEL, "Cancel");
    TRC(IDC_WATCHCLIPBOARD, "Watch Clipboard for URLs");
    TRC(IDC_IMAGEDOWNLOADERTIP, "Enter URLs (one http:// or ftp:// link per line)");
    ::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS), SW_HIDE);
    SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_SETCHECK, Settings.WatchClipboard?BST_CHECKED:BST_UNCHECKED);

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
    if (isVistaOrLater_) {
        fRemoveClipboardFormatListener_(m_hWnd);
    } else {
        ChangeClipboardChain(PrevClipboardViewer);
    }

    return 0;
}

bool ExtractLinks(CString text, std::vector<CString> &result)
{
    pcrepp::Pcre reg("((http|https|ftp)://[\\w\\d:#@%/;$()~_?\\+-=\\\\\\.&]*)", "imcu");
    std::string str = WCstringToUtf8(text);
    size_t pos = 0;
    while (pos <= str.length()) 
    {
        if( reg.search(str, pos)) 
        { 
            pos = reg.get_match_end()+1;
            CString temp = Utf8ToWstring(reg[1]).c_str();
            result.push_back(temp);
        }
        else
            break;
    }
    return true;
}

LRESULT CImageDownloaderDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    BeginDownloading();
    return 0;
}

LRESULT CImageDownloaderDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(m_FileDownloader.isRunning()) 
        m_FileDownloader.stop();
    else
    {
        WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
        Settings.WatchClipboard = SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) != 0;
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

LRESULT CImageDownloaderDlg::OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hwndRemove = reinterpret_cast<HWND>(wParam);  // handle of window being removed 
    HWND hwndNext = reinterpret_cast<HWND>(lParam);

    if(hwndRemove == PrevClipboardViewer) PrevClipboardViewer = hwndNext;
    else ::SendMessage(PrevClipboardViewer, WM_CHANGECBCHAIN, wParam, lParam);
    return 0;
}

void CImageDownloaderDlg::OnDrawClipboard()
{
    clipboardUpdated();
    //Sending WM_DRAWCLIPBOARD msg to the next window in the chain
    if (PrevClipboardViewer) ::SendMessage(PrevClipboardViewer, WM_DRAWCLIPBOARD, 0, 0);
}

bool CImageDownloaderDlg::OnFileFinished(bool ok, int statusCode, const CFileDownloader::DownloadFileListItem& it)
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
                    ais.VirtualFileName+=_T(".")+ext;
                }
            }
            else 
            {
                add = false;
                CString errorStr;
                CString url = U2W(it.url);
                errorStr.Format(TR("File '%s' is not an image (Mime-Type: %s)."), static_cast<LPCTSTR>(url),
                    static_cast<LPCTSTR>(U2W(mimeType)));
                ServiceLocator::instance()->logger()->write(ILogger::logError, _T("Image Downloader"), errorStr);
            }
        }
        if (add) {
            if (m_WizardDlg) {
                SendMessage(m_WizardDlg->m_hWnd, WM_MY_ADDIMAGE, reinterpret_cast<WPARAM>(&ais), 0);
            } else {
                m_downloadedFiles.push_back(ais.RealFileName);
            }
            
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
}

bool CImageDownloaderDlg::BeginDownloading()
{
    std::string links = W2U(GuiTools::GetWindowText(GetDlgItem(IDC_FILEINFOEDIT)));
    std::vector<std::string> tokens;
    nm_splitString(links,"\n",tokens,-1);
    m_nFilesCount =0;
    m_nFileDownloaded = 0;
    for(size_t i=0; i<tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        if (token.empty() || IuStringUtils::Trim(token).empty()) {
            continue;
        }
        m_FileDownloader.addFile(nm_trimStr(token), reinterpret_cast<void*>(i));
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
        m_FileDownloader.setOnFileFinishedCallback(std::bind(&CImageDownloaderDlg::OnFileFinished, this, _1, _2, _3)); 
        m_FileDownloader.setOnQueueFinishedCallback(std::bind(&CImageDownloaderDlg::OnQueueFinished, this));
        m_FileDownloader.start();
        return true;
    }
    return false;
}

bool CImageDownloaderDlg::LinksAvailableInText(const CString &text)
{
    std::vector<CString> links;
    ExtractLinks(text,links);
    return links.size()!=0;
}

void CImageDownloaderDlg::ParseBuffer(const CString& buffer,bool OnlyImages)
{
    std::vector<CString> links;
    ExtractLinks(buffer,links);
    CString text = GuiTools::GetWindowText(GetDlgItem(IDC_FILEINFOEDIT));
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

    if (IsClipboard && SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) == BST_CHECKED && !m_FileDownloader.isRunning())
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