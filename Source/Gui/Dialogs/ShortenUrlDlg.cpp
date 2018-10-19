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
#include "ShortenUrlDlg.h"

#include "Func/Common.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Core/Upload/FileQueueUploader.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Func/WinUtils.h"
#include "Core/CommonDefs.h"
#include "Func/WebUtils.h"
#include "Core/Upload/UploadManager.h"
#include "Core/CoreFunctions.h"
#include "Core/ServiceLocator.h"
#include "Gui/Controls/ServerSelectorControl.h"

// CShortenUrlDlg
CShortenUrlDlg::CShortenUrlDlg(CWizardDlg *wizardDlg, UploadManager* uploadManager, UploadEngineManager* uploadEngineManager, const CString &initialBuffer)
{
    m_WizardDlg = wizardDlg;
    m_InitialBuffer = initialBuffer;
    backgroundBrush_.CreateSysColorBrush(COLOR_BTNFACE);
    uploadManager_ = uploadManager;
    uploadEngineManager_ = uploadEngineManager;
}


CShortenUrlDlg::~CShortenUrlDlg()
{
}

LRESULT CShortenUrlDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    PrevClipboardViewer = SetClipboardViewer();
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"
 
    ::SetFocus(GetDlgItem(IDOK));
    SetWindowText(TR("Url Shortener"));
    TRC(IDOK, "Shorten");
    TRC(IDCANCEL, "Close");
    TRC(IDC_SHORTENURLTIP, "Paste a link to shorten it:");
    GuiTools::MakeLabelBold(GetDlgItem(IDC_RESULTSLABEL));

    ::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS), SW_HIDE);
    HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
    if (hWnd) {
        wndAnimation_.SubclassWindow(hWnd);
        if (wndAnimation_.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF), _T("GIF")))
            wndAnimation_.Draw();
        wndAnimation_.ShowWindow(SW_HIDE);
    }
    outputEditControl_.AttachToDlgItem(m_hWnd, IDC_RESULTSEDIT);

    CRect serverSelectorRect = GuiTools::GetDialogItemRect(m_hWnd, IDC_SHORTENINGSERVERPLACEHOLDER);

    urlShortenerServerSelector_.reset(new CServerSelectorControl(uploadEngineManager_));
    urlShortenerServerSelector_->setServersMask(CServerSelectorControl::smUrlShorteners);
    urlShortenerServerSelector_->setShowImageProcessingParams(false);
    urlShortenerServerSelector_->setShowParamsLink(false);
    urlShortenerServerSelector_->Create(m_hWnd, serverSelectorRect);
    urlShortenerServerSelector_->ShowWindow(SW_SHOW);
    urlShortenerServerSelector_->SetWindowPos(0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);
    urlShortenerServerSelector_->setServerProfile(Settings.urlShorteningServer);
    urlShortenerServerSelector_->setTitle(TR("URL shortening server"));

    if(!m_InitialBuffer.IsEmpty())
    {
        ParseBuffer(m_InitialBuffer);
        //BeginDownloading(); 
    }

    CString clipboardText;
    WinUtils::GetClipboardText(clipboardText);
    if ( !clipboardText.IsEmpty() ) {
        ParseBuffer(clipboardText);
    }
    ::SetFocus(GetDlgItem(IDC_INPUTEDIT));
    return 0; 
}


LRESULT CShortenUrlDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    CString url = GuiTools::GetDlgItemText(m_hWnd, IDC_INPUTEDIT);
    if ( url.IsEmpty() ) {
        MessageBox(TR("Please enter an URL"), TR("Error"), MB_ICONERROR);
        return 0;
    }
    
    if (urlShortenerServerSelector_->serverProfile().serverName().empty()) {
        MessageBox(TR("You have not selected server"), TR("Error"), MB_ICONERROR);
        return 0;
    } else if (!urlShortenerServerSelector_->isAccountChosen()) {
            CString message;
            message.Format(TR("You have not selected account for server \"%s\""), IuCoreUtils::Utf8ToWstring(urlShortenerServerSelector_->serverProfile().serverName()).c_str());
            MessageBox(message, TR("Error"), MB_ICONERROR);
            return 0;
    } 
    
    StartProcess();
    //BeginDownloading();
    return 0;
}

LRESULT CShortenUrlDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if ( uploadManager_->IsRunning() ) {
        uploadManager_->stop();
        return 0;
    }
    /*if(m_FileDownloader.IsRunning()) 
        m_FileDownloader.stop();
    else
    {
        Settings.WatchClipboard = SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) != 0;
        EndDialog(wID);
    }*/
    OnClose();
    EndDialog(wID);
    return 0;
}

LRESULT CShortenUrlDlg::OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hwndRemove = reinterpret_cast<HWND>(wParam);  // handle of window being removed 
    HWND hwndNext = reinterpret_cast<HWND>(lParam);

    if(hwndRemove == PrevClipboardViewer) PrevClipboardViewer = hwndNext;
    else ::SendMessage(PrevClipboardViewer, WM_CHANGECBCHAIN, wParam, lParam);
    return 0;
}

void CShortenUrlDlg::OnDrawClipboard()
{
    bool IsClipboard = IsClipboardFormatAvailable(CF_TEXT)!=0;

    if(IsClipboard && SendDlgItemMessage(IDC_WATCHCLIPBOARD,BM_GETCHECK)==BST_CHECKED && !m_FileDownloader.isRunning()    )
    {
        CString str;
        WinUtils::GetClipboardText(str);
        //ParseBuffer(str, true);
        
    }
    //Sending WM_DRAWCLIPBOARD msg to the next window in the chain
    if(PrevClipboardViewer) ::SendMessage(PrevClipboardViewer, WM_DRAWCLIPBOARD, 0, 0); 
}

LRESULT CShortenUrlDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ChangeClipboardChain(PrevClipboardViewer);
    return 0;
}

bool CShortenUrlDlg::StartProcess() {
    ServerProfile profile = urlShortenerServerSelector_->serverProfile();
    if (profile.isNull()) {
        return false;
    }
    profile.setShortenLinks(false);
    ::ShowWindow(GetDlgItem(IDC_RESULTSLABEL), SW_SHOW);
    GuiTools::EnableDialogItem(m_hWnd, IDOK, false);
    wndAnimation_.ShowWindow(SW_SHOW);
    CString url = GuiTools::GetDlgItemText(m_hWnd, IDC_INPUTEDIT);

    std::shared_ptr<UrlShorteningTask> task(new UrlShorteningTask(WCstringToUtf8(url)));
    
    task->setServerProfile(profile);
    task->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &CShortenUrlDlg::OnFileFinished));
    std::shared_ptr<UploadSession> session(new UploadSession());
    session->addTask(task);
    session->addSessionFinishedCallback(UploadSession::SessionFinishedCallback(this, &CShortenUrlDlg::OnQueueFinished));
    uploadManager_->addSession(session);
    uploadManager_->start();

    return true;
}

void CShortenUrlDlg::OnFileFinished(UploadTask* task, bool ok) {
    if ( ok ) {
        CString shortUrl = Utf8ToWCstring(task->uploadResult()->directUrl);
        SetDlgItemText(IDC_RESULTSEDIT, shortUrl);
        WinUtils::CopyTextToClipboard(shortUrl);
        SetDlgItemText(IDC_RESULTSLABEL, TR("The short link has been copied to the clipboard!"));
        ::ShowWindow(GetDlgItem(IDC_RESULTSLABEL), SW_SHOW);
        ::SetFocus(GetDlgItem(IDC_RESULTSEDIT));
    }
}

void CShortenUrlDlg::OnQueueFinished(UploadSession* session) {
    ProcessFinished();
}

void CShortenUrlDlg::ProcessFinished() {
    GuiTools::EnableDialogItem(m_hWnd, IDOK, true);
    wndAnimation_.ShowWindow(SW_HIDE);
}

void CShortenUrlDlg::OnClose() {
    Settings.urlShorteningServer = urlShortenerServerSelector_->serverProfile();
}

bool CShortenUrlDlg::ParseBuffer(const CString& text) {
    CString textCopy = text;
    if (  WebUtils::DoesTextLookLikeUrl(textCopy) ) {
        SetDlgItemText(IDC_INPUTEDIT, textCopy);
    }
    return false;
}

LRESULT CShortenUrlDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwnd) {
    if ( hwnd == GetDlgItem(IDC_RESULTSLABEL ) ) {
        SetTextColor(hdc, RGB(0,180,0));
        SetBkMode(hdc, TRANSPARENT);
        return reinterpret_cast<LRESULT>(static_cast<HBRUSH>(backgroundBrush_)); 
    }
    return 0;
}

bool  CShortenUrlDlg::OnConfigureNetworkClient(CFileQueueUploader* ,NetworkClient* nm) {
    CoreFunctions::ConfigureProxy(nm);
    return true;
}


