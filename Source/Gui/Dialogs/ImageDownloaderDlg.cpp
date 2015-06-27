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
#include "ImageDownloaderDlg.h"

#include "Core/CommonDefs.h"
#include "Func/Common.h"
#include "Core/3rdpart/pcreplusplus.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Func/myutils.h"
#include "Core/ServiceLocator.h"
#include "Core/CoreFunctions.h"
#include "Func/WebUtils.h"

// CImageDownloaderDlg
CImageDownloaderDlg::CImageDownloaderDlg(CWizardDlg *wizardDlg,const CString &initialBuffer)
{
    m_WizardDlg = wizardDlg;
    m_InitialBuffer = initialBuffer;
    fRemoveClipboardFormatListener_ = NULL;
    PrevClipboardViewer = NULL;
}

CImageDownloaderDlg::~CImageDownloaderDlg()
{
    
}

LRESULT CImageDownloaderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());

    isVistaOrLater_ = WinUtils::IsVista();

    if (isVistaOrLater_)
    {
        HMODULE module = LoadLibrary(_T("user32.dll"));
        AddClipboardFormatListenerFunc fAddClipboardFormatListener = reinterpret_cast<AddClipboardFormatListenerFunc>(GetProcAddress(module, "AddClipboardFormatListener"));
        fAddClipboardFormatListener(m_hWnd);
        fRemoveClipboardFormatListener_ = reinterpret_cast<RemoveClipboardFormatListenerFunc>(GetProcAddress(module, "RemoveClipboardFormatListener"));
    } else {
        PrevClipboardViewer = SetClipboardViewer(); // using old fragile cliboard listening method on pre Vista systems
    }
    
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"
 
    ::SetFocus(GetDlgItem(IDOK));
    SetWindowText(TR("Image Downloader"));
    TRC(IDOK, "Add");
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
    return 1; 
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
    if(m_FileDownloader.IsRunning()) 
        m_FileDownloader.stop();
    else
    {
        Settings.WatchClipboard = SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) != 0;
        EndDialog(wID);
    }
    return 0;
}

LRESULT CImageDownloaderDlg::OnClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    clipboardUpdated();
    return 0;
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

CString GetExtensionByMime(CString mime)
{
    TCHAR szImgTypes[3][4]={_T("jpg"),_T("png"),_T("gif")};
    TCHAR szMimeTypes[3][12]={_T("jpeg"),_T("png"),_T("gif")};
    for(int i=0;i<3;i++)
    {
        if(mime.Find(szMimeTypes[i])>=0)
            return szImgTypes[i];
    }
    return _T(".dat");
}

bool CImageDownloaderDlg::OnFileFinished(bool ok, int statusCode, CFileDownloader::DownloadFileListItem it)
{
    if(ok)
    {
        CWizardDlg::AddImageStruct ais;
        ais.show =true;
        ais.RealFileName = Utf8ToWstring(it.fileName).c_str();
        ais.VirtualFileName =  Utf8ToWstring(it.displayName).c_str();
        bool add = true;
        if(!IsImage(ais.RealFileName))
        {
            CString mimeType = Utf8ToWCstring(IuCoreUtils::GetFileMimeType(WCstringToUtf8(ais.RealFileName)));
            if(mimeType.Find(_T("image/"))>=0)
            {
                CString ext = GetExtensionByMime(mimeType);
                if(!ext.IsEmpty())
                {
                    CString newFileName = ais.RealFileName + _T(".")+ext;
                    MoveFile(ais.RealFileName, newFileName);
                     ais.RealFileName = newFileName;
                    ais.VirtualFileName+=_T(".")+ext;
                }
            }
            else 
            {
                add = false;
                CString errorStr;
                errorStr.Format(TR("File '%s' is not an image (Mime-Type: %s)."),(LPCTSTR)(Utf8ToWstring(it.url).c_str()),
                    static_cast<LPCTSTR>(mimeType));
                ServiceLocator::instance()->logger()->write(logError, _T("Image Downloader"), errorStr);
            }
        }
        if(add)
            SendMessage(m_WizardDlg->m_hWnd, WM_MY_ADDIMAGE, reinterpret_cast<WPARAM>(&ais),  0);

    }
    m_nFileDownloaded++;
    SendDlgItemMessage(IDC_DOWNLOADFILESPROGRESS, PBM_SETPOS,  m_nFileDownloaded);
    return true;
}

void CImageDownloaderDlg::OnQueueFinished()
{
    if(!m_InitialBuffer.IsEmpty())
    {
        EndDialog(0);
        IU_CopyTextToClipboard("");
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
        m_FileDownloader.AddFile(nm_trimStr(tokens[i]), reinterpret_cast<void*>(i));
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
        m_FileDownloader.onFileFinished.bind(this, &CImageDownloaderDlg::OnFileFinished);
        m_FileDownloader.onQueueFinished.bind(this, &CImageDownloaderDlg::OnQueueFinished);
        m_FileDownloader.onConfigureNetworkClient.bind(this, &CImageDownloaderDlg::OnConfigureNetworkClient);
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
        if( ((!OnlyImages && CString(WinUtils::GetFileExt(fileName)).IsEmpty()) || IsImage(fileName)) &&  text.Find(links[i]) == -1 ) {
            if (text.Right(1) != _T("\n")) {
                text += "\r\n";
            }
            text+=links[i]+_T("\r\n");
        }
    }
    SetDlgItemText(IDC_FILEINFOEDIT, text);
}

void CImageDownloaderDlg::OnConfigureNetworkClient(NetworkClient* nm)
{
    CoreFunctions::ConfigureProxy(nm);
}

void CImageDownloaderDlg::clipboardUpdated()
{
    bool IsClipboard = IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;

    if (IsClipboard && SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) == BST_CHECKED && !m_FileDownloader.IsRunning())
    {
        CString str;
        if (WinUtils::GetClipboardText(str, m_hWnd, true))
        {
            ParseBuffer(str, false);
        }
    }
}