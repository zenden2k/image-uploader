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

#include "UploadDlg.h"

#include <shobjidl.h>
#include "Core/Images/ImageConverter.h"
#include "Core/ServiceLocator.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/InputDialog.h"
#include "Core/Settings.h"
#include "Core/Upload/UploadEngine.h"
#include "Gui/GuiTools.h"
#include "Core/3rdpart/FastDelegate.h"
#include "Core/Upload/FileQueueUploader.h"
#include "Func/IuCommonFunctions.h"
#include "Func/MyUtils.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UploadManager.h"
#include "Func/WinUtils.h"
#include <Core/CoreFunctions.h>
#include "Gui/Dialogs/WizardDlg.h"

// CUploadDlg
CUploadDlg::CUploadDlg(CWizardDlg *dlg,UploadManager* uploadManager) :ResultsWindow(new CResultsWindow(dlg, UrlList, true))
{
    MainDlg = NULL;
    TimerInc = 0;
    IsStopTimer = false;
    Terminated = false;
    m_EngineList = _EngineList;
    LastUpdate = 0;
    backgroundThreadStarted_ = false;
    isEnableNextButtonTimerRunning_ = false;
    //fastdelegate::FastDelegate1<bool> fd;
    //fd.bind(this, &CUploadDlg::onShortenUrlChanged);
    uploadManager_ = uploadManager;
    ResultsWindow->setOnShortenUrlChanged(fastdelegate::MakeDelegate(this, &CUploadDlg::onShortenUrlChanged));
    #if  WINVER    >= 0x0601
        ptl = NULL;
    #endif
}

CUploadDlg::~CUploadDlg()
{
    delete ResultsWindow;
}

//Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance,LPCTSTR szResName, LPCTSTR szResType);

LRESULT CUploadDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Initializing Windows 7 taskbar related stuff
    RECT rc;
    ::GetWindowRect(GetDlgItem(IDC_RESULTSPLACEHOLDER), &rc);
    ::MapWindowPoints(0,m_hWnd, (POINT*)&rc, 2);

    uploadListView_.AttachToDlgItem(m_hWnd, IDC_UPLOADTABLE);
    uploadListView_.AddColumn(TR("File"), 1);
    uploadListView_.AddColumn(TR("Status"), 1);
    uploadListView_.AddColumn(TR("Thumbnail"), 2);
    CDC hdc = GetDC();
    float dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    //float dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    uploadListView_.SetColumnWidth(0, static_cast<int>(170 * dpiScaleX));
    uploadListView_.SetColumnWidth(1, static_cast<int>(170 * dpiScaleX));
    uploadListView_.SetColumnWidth(2, static_cast<int>(170 * dpiScaleX));

    createToolbar();

    ResultsWindow->Create(m_hWnd);
    ResultsWindow->SetWindowPos(0,&rc,0);
    #if  WINVER    >= 0x0601
        const GUID IID_ITaskbarList3 = { 0xea1afb91,0x9e28,0x4b86,{0x90,0xe9,0x9e,0x9f, 0x8a,0x5e,0xef,0xaf}};
        CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&ptl);
    #endif

    TRC(IDC_COMMONPROGRESS, "Progress:");
    bool IsLastVideo = lstrlen(MediaInfoDllPath)!=0;

    CVideoGrabberPage *vg = static_cast<CVideoGrabberPage*>(WizardDlg->Pages[1]);

    if(vg && lstrlen(vg->m_szFileName))
        IsLastVideo=true;

    ResultsWindow->EnableMediaInfo(IsLastVideo);

    SetDlgItemInt(IDC_THUMBSPERLINE, 4);
    SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
    
    GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPROGRESS));
    GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPERCENTS));
    PageWnd = m_hWnd;
    ResultsWindow->SetPage(Settings.CodeLang);
    ResultsWindow->SetCodeType(Settings.CodeType);
    showUploadProgressTab();
    return 1;  
}

bool CUploadDlg::startUpload() {
    if(!MainDlg) return 0;
    
    #if  WINVER  >= 0x0601
        if(ptl)
            ptl->SetProgressState(GetParent(), TBPF_NORMAL); // initialise Windows 7 taskbar button progress 
    #endif

    SetDlgItemText(IDC_COMMONPROGRESS2, _T(""));
    showUploadProgressTab();
    int n = MainDlg->FileList.GetCount();
    SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETPOS, 0);
    SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, n));
    SetDlgItemText(IDC_COMMONPERCENTS, _T("0%"));

    TotalUploadProgress(0, n);

    uploadListView_.DeleteAllItems();

    UrlList.clear();
    UrlList.resize(n);
    
    uploadSession_.reset(new UploadSession());
    for (int i = 0; i < n; i++) {
        CString FileName = MainDlg->FileList[i].FileName;
        std::string fileNameA = WCstringToUtf8(FileName);
        std::string displayName = WCstringToUtf8(MainDlg->FileList[i].VirtualFileName);

        FileProcessingStruct* fps = new FileProcessingStruct();
        fps->fileName = FileName;
        fps->tableRow = i;
        files_.push_back(fps);
        uploadListView_.AddItem(i, 0, MainDlg->FileList[i].VirtualFileName);
        uploadListView_.AddItem(i, 1, TR("Queued"));
        uploadListView_.SetItemData(i, reinterpret_cast<DWORD_PTR>(fps));
        bool isImage = IsImage(FileName);
        std::shared_ptr<FileUploadTask> task(new FileUploadTask(fileNameA, displayName));
        task->OnUploadProgress.bind(this, &CUploadDlg::onTaskUploadProgress);
        task->setUserData(fps);
        task->setIsImage(isImage);
        task->OnStatusChanged.bind(this, &CUploadDlg::OnUploaderStatusChanged);
        task->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &CUploadDlg::onTaskFinished));
        task->setServerProfile(isImage ? sessionImageServer_ : sessionFileServer_);
        task->OnChildTaskAdded.bind(this, &CUploadDlg::onChildTaskAdded);
        task->setUrlShorteningServer(Settings.urlShorteningServer);
        uploadSession_->addTask(task);
    }
    uploadSession_->addSessionFinishedCallback(UploadSession::SessionFinishedCallback(this, &CUploadDlg::onSessionFinished));
    uploadManager_->addSession(uploadSession_);
    return 0;
}

LRESULT CUploadDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == kEnableNextButtonTimer)
    {
        EnableNext(true);
        isEnableNextButtonTimerRunning_ = false;
        KillTimer(kEnableNextButtonTimer);
    }
    return 0;
}

int CUploadDlg::ThreadTerminated(void)
{
    WizardDlg->QuickUploadMarker = false;
    TCHAR szBuffer[MAX_PATH];

    //TotalUploadProgress(MainDlg->FileList.GetCount(), MainDlg->FileList.GetCount());
    #if  WINVER    >= 0x0601
        if(ptl)
            ptl->SetProgressState(GetParent(), TBPF_NOPROGRESS);
    #endif
    wsprintf(szBuffer,_T("%d %%"),100);
    SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

    KillTimer(1);
    KillTimer(2);
    LastUpdate = 0;

    SetNextCaption(TR("Finish >"));
    Terminated = true;
    IsStopTimer = false;
    backgroundThreadStarted_ = false;
    EnablePrev();
    EnableNext();
    EnableExit();
    return 0;
}

bool CUploadDlg::OnShow()
{
    sessionFileServer_ = WizardDlg->getSessionFileServer();
    sessionImageServer_ = WizardDlg->getSessionImageServer();
    bool IsLastVideo=false;

    if(lstrlen(MediaInfoDllPath))
    { 
        CVideoGrabberPage *vg =(CVideoGrabberPage *) WizardDlg->Pages[1];

        if(vg && lstrlen(vg->m_szFileName))
            IsLastVideo=true;
    }
    ResultsWindow->InitUpload();
    ResultsWindow->EnableMediaInfo(IsLastVideo);
    CancelByUser = false;
    ShowNext();
    ShowPrev();
    EnableNext(false);
    isEnableNextButtonTimerRunning_ = true;
    SetTimer(kEnableNextButtonTimer, 1000);
    MainDlg = (CMainDlg*) WizardDlg->Pages[2];
    //Toolbar.CheckButton(IDC_USETEMPLATE,Settings.UseTxtTemplate);
    UrlList.clear();
    ResultsWindow->Clear();
    ResultsWindow->setShortenUrls(sessionImageServer_.shortenLinks());

    int code = ResultsWindow->GetCodeType();
    int newcode = code;
    bool Thumbs = sessionImageServer_.getImageUploadParams().CreateThumbs!=0;

    // Корректировка типа кода в зависимости от включения превьюшек
    if(Thumbs)
    {
        if(code<4 && code>1)
            newcode = 0;
    }
    else
    {    
        if(code<2)
            newcode=2;
    }
    ResultsWindow->SetCodeType(newcode);
    ResultsWindow->SetPage(Settings.CodeLang);

    ::SetFocus(GetDlgItem(IDC_CODEEDIT));
    alreadyShortened_ = false;
    backgroundThreadStarted();
    startUpload();
    return true;
}

bool CUploadDlg::OnNext()
{
    if(uploadSession_->isRunning())
    {
        if(!IsStopTimer)
        {
            uploadSession_->stop();
            CancelByUser = true;
            //TimerInc = 5;
            //SetTimer(1, 1000, NULL);
            //IsStopTimer = true;
        }
        /*else 
        {
            if(TimerInc<5)
            {
                this->Terminate();
                ThreadTerminated();
                FileProgress(TR("File uploading was cancelled by user."), false);
            }
        }*/

    }
    else 
    {
        MainDlg->ThumbsView.MyDeleteAllItems();
        EnableExit();
        return true;
    }
    return false;
}

bool CUploadDlg::OnHide()
{
    UrlList.clear();
    ResultsWindow->Clear();
    uploadManager_->removeSession(uploadSession_);
    uploadSession_.reset();
    for (auto it : files_)
    {
        delete it;
    }
    files_.clear();
    
    Settings.UseTxtTemplate = (SendDlgItemMessage(IDC_USETEMPLATE, BM_GETCHECK) == BST_CHECKED);
    Settings.CodeType = ResultsWindow->GetCodeType();
    Settings.CodeLang = ResultsWindow->GetPage();
    return true; 
}

int GetWindowLeft(HWND Wnd)
{
    RECT WindowRect = {0,0,0,0};

    GetWindowRect(Wnd,&WindowRect);
    HWND Parent = GetParent(Wnd);
    ScreenToClient(Parent, (LPPOINT)&WindowRect);
    return WindowRect.left;
}

void CUploadDlg::GenerateOutput()
{
    ResultsWindow->UpdateOutput();
}

void CUploadDlg::TotalUploadProgress(int CurPos, int Total, int FileProgress)
{
    SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETPOS, CurPos);
#if  WINVER    >= 0x0601 // Windows 7 related stuff
    if(ptl)
    {
        int NewCurrent = CurPos * 50 + FileProgress;
        int NewTotal = Total * 50;
        ptl->SetProgressValue(GetParent(), NewCurrent, NewTotal);
    }
#endif
    progressCurrent = CurPos;
    progressTotal = Total;
    CString res;
    res.Format(TR("Links (%d)"), CurPos);
    toolbar_.SetButtonInfo(IDC_UPLOADRESULTSTAB, TBIF_TEXT, 0, 0, res,0, 0, 0, 0);
}


void CUploadDlg::OnUploaderStatusChanged(UploadTask* task)
{
    UploadProgress* progress = task->progress();
    FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(task->role() == UploadTask::DefaultRole ? task->userData() : task->parentTask()->userData());
    if (!fps)
    {
        return;
    }
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (fileTask) {
        
        CString statusText = /*UploaderStatusToString(progress->statusType, progress->stage, progress->statusText)*/ IuCoreUtils::Utf8ToWstring(progress->statusText).c_str();

        bool isThumb = task->role() == UploadTask::ThumbRole;
        int columnIndex = isThumb ? 2 : 1;
        uploadListView_.SetItemText(fps->tableRow, columnIndex, statusText);
    }
    /*PrInfo.CS.Lock();
    //m_StatusText = 
    PrInfo.Bytes.clear(); 
    PrInfo.ip.clear();
    PrInfo.CS.Unlock();*/
}






void CUploadDlg::onShortenUrlChanged(bool shortenUrl) {
    if ( !alreadyShortened_ && shortenUrl ) {
        GenerateOutput();
        
        uploadManager_->shortenLinksInSession(uploadSession_);
    } else {
        GenerateOutput();
    }
}

void CUploadDlg::AddShortenUrlTask(CUrlListItem* item) {
    /*if ( !item->ImageUrl.IsEmpty() && item->ImageUrlShortened.IsEmpty() ) {
        AddShortenUrlTask(item, _T("ImageUrl") );
    }
    if ( !item->DownloadUrl.IsEmpty() && item->DownloadUrlShortened.IsEmpty() ) {
        AddShortenUrlTask(item, _T("DownloadUrl") );
    }*/
    /*if ( !item->ThumbUrl.IsEmpty() && item->ThumbUrlShortened.IsEmpty() ) {
        AddShortenUrlTask(item, _T("ThumbUrl") );
    }*/
}

void CUploadDlg::AddShortenUrlTask(CUrlListItem* item, CString linkType) {
    /*CUploadEngineData *ue = Settings.urlShorteningServer.uploadEngineData();
    if ( !ue ) {
        WriteLog(logError, _T("Uploader"), _T("Cannot create url shortening engine '" + Settings.urlShorteningServer.serverName() + "'"));
        return;
    }
    CUploadEngineData* newData = new CUploadEngineData();
    *newData = *ue;
    CAbstractUploadEngine * e = m_EngineList->getUploadEngine(ue,Settings.urlShorteningServer.serverSettings());
    e->setUploadData(newData);
    ServerSettingsStruct& settings = Settings.urlShorteningServer.serverSettings();
    e->setServerSettings(settings);
    ShortenUrlUserData* userData = new ShortenUrlUserData;
    userData->item = item;
    userData->linkType = linkType;
    CString url;
    if ( linkType == _T("ImageUrl") ) {
        url = item->ImageUrl;
    } else if ( linkType == _T("DownloadUrl") ) {
        url = item->DownloadUrl;
    } else if ( linkType == _T("ThumbUrl") ) {
        url = item->ThumbUrl;
    }
    
    if ( url.IsEmpty() ) {
        return;
    }
    std::shared_ptr<UrlShorteningTask> task(new UrlShorteningTask(WCstringToUtf8(url)));
    queueUploader_->AddUploadTask(task, reinterpret_cast<void*>(userData), e);
    queueUploader_->start();*/
}

bool CUploadDlg::OnFileFinished(std::shared_ptr<UploadTask> task, bool ok) {
    /*ShortenUrlUserData* shortenUrlUserData  = reinterpret_cast<ShortenUrlUserData*>(result.uploadTask->userData);
        
    if ( shortenUrlUserData->linkType == "ImageUrl" ){
        shortenUrlUserData->item->ImageUrlShortened = Utf8ToWCstring(result.imageUrl);
    }

    if ( shortenUrlUserData->linkType == "DownloadUrl" ) {
        shortenUrlUserData->item->DownloadUrlShortened = Utf8ToWCstring(result.imageUrl);
    }

    if ( shortenUrlUserData->linkType == "ThumbUrl" ) {
        shortenUrlUserData->item->ThumbUrlShortened = Utf8ToWCstring(result.imageUrl);
    }*/

    GenerateOutput();

    return false;
}

bool CUploadDlg::OnConfigureNetworkClient(CFileQueueUploader*, NetworkClient* nm) {
    CoreFunctions::ConfigureProxy(nm);
    return true;
}

void CUploadDlg::createToolbar()
{
    CBitmap hBitmap;
    HIMAGELIST m_hToolBarImageList;
    if (GuiTools::Is32BPP()) {
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_BITMAP3));
        m_hToolBarImageList = ImageList_Create(16, 16, ILC_COLOR32, 0, 6);
        ImageList_Add(m_hToolBarImageList, hBitmap, NULL);
    }
    else {
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_BITMAP4));
        m_hToolBarImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 6);
        ImageList_AddMasked(m_hToolBarImageList, hBitmap, RGB(255, 0, 255));
    }

    RECT rc = { 0, 0, 100, 24 };
    GetClientRect(&rc);
    rc.top = GuiTools::dlgY(24);
    rc.bottom = rc.top + GuiTools::dlgY(16);
    rc.left = GuiTools::dlgX(6);
    rc.right -= GuiTools::dlgX(6);
    toolbar_.Create(m_hWnd, rc, _T(""), WS_CHILD | WS_CHILD | TBSTYLE_LIST | TBSTYLE_CUSTOMERASE | TBSTYLE_FLAT | CCS_NORESIZE/*|*/ | CCS_BOTTOM | /*CCS_ADJUSTABLE|*/CCS_NODIVIDER | TBSTYLE_AUTOSIZE);

    toolbar_.SetButtonStructSize();
    toolbar_.SetButtonSize(30, 18);
    toolbar_.SetImageList(m_hToolBarImageList);
    toolbar_.AddButton(IDC_UPLOADPROCESSTAB, BTNS_CHECK | BTNS_AUTOSIZE, TBSTATE_ENABLED | TBSTATE_PRESSED, 0, TR("Upload progress"), 0);
    toolbar_.AddButton(IDC_UPLOADRESULTSTAB, BTNS_CHECK | BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Links"), 0);
    toolbar_.AddButton(IDC_VIEWLOG, TBSTYLE_BUTTON | BTNS_AUTOSIZE, TBSTATE_ENABLED, 3, TR("Log"), 0);

    toolbar_.AutoSize();
    toolbar_.SetWindowLong(GWL_ID, IDC_RESULTSTOOLBAR);
    toolbar_.ShowWindow(SW_SHOW);
}

LRESULT CUploadDlg::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    LogWindow.Show();
    return 0;
}

void CUploadDlg::showUploadResultsTab() {
    if (currentTab_ == IDC_UPLOADRESULTSTAB) {
        return;
    }
    toolbar_.SetButtonInfo(IDC_UPLOADPROCESSTAB, TBIF_STATE, 0, TBSTATE_ENABLED, 0, 0, 0, 0, 0);
    toolbar_.SetButtonInfo(IDC_UPLOADRESULTSTAB, TBIF_STATE, 0, TBSTATE_ENABLED | TBSTATE_PRESSED, 0, 0, 0, 0, 0);

    currentTab_ = IDC_UPLOADRESULTSTAB;
    uploadListView_.ShowWindow(SW_HIDE);
    ResultsWindow->ShowWindow(SW_SHOW);
}

void CUploadDlg::showUploadProgressTab() {
    if (currentTab_ == IDC_UPLOADPROCESSTAB) {
        return;
    }
    currentTab_ = IDC_UPLOADPROCESSTAB;
    toolbar_.SetButtonInfo(IDC_UPLOADRESULTSTAB, TBIF_STATE, 0, TBSTATE_ENABLED, 0,
        0, 0, 0, 0);
    toolbar_.SetButtonInfo(IDC_UPLOADPROCESSTAB, TBIF_STATE, 0, TBSTATE_ENABLED | TBSTATE_PRESSED, 0, 0, 0, 0, 0);

    uploadListView_.ShowWindow(SW_SHOW);
    ResultsWindow->ShowWindow(SW_HIDE);
}

void CUploadDlg::onSessionFinished(UploadSession* session)
{
    int successFileCount = session->finishedTaskCount(UploadTask::StatusFinished);
    int failedFileCount = session->finishedTaskCount(UploadTask::StatusFailure);
    int totalFileCount = session->taskCount();
    KillTimer(kEnableNextButtonTimer);
    CString progressLabelText;
    if (successFileCount == totalFileCount) {
        progressLabelText = TR("All files have been successfully uploaded.");
    } else {
        if (CancelByUser) {
            progressLabelText = TR("File uploading was cancelled by user.");
        } else if (failedFileCount) {
            progressLabelText.Format(TR("Errors: %d"), failedFileCount);
            progressLabelText = CString(TR("Uploading has been finished.")) + _T(" ") + progressLabelText;
        }
    }

    SetDlgItemText(IDC_COMMONPROGRESS2, progressLabelText);
    ThreadTerminated();
    if (successFileCount == totalFileCount) {
        showUploadResultsTab();
    }
}

void CUploadDlg::onTaskUploadProgress(UploadTask* task)
{
    FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(task->role() == UploadTask::DefaultRole ? task->userData() : task->parentTask()->userData());
    if (!fps)
    {
        return;
    }
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (fileTask) {
        
        TCHAR ProgressBuffer[256] = _T("");
        bool isThumb = task->role() == UploadTask::ThumbRole;
        int percent = 0;
        UploadProgress* progress = task->progress();
        if (progress->totalUpload) {
            percent = static_cast<int>(100 * ((float)progress->uploaded) / progress->totalUpload);
        }
        CString uploadSpeed = Utf8ToWCstring(progress->speed);
        _stprintf(ProgressBuffer, TR("%s of %s (%d%%) %s"), (LPCTSTR)Utf8ToWCstring(IuCoreUtils::fileSizeToString(progress->uploaded)),
            (LPCTSTR)Utf8ToWCstring(IuCoreUtils::fileSizeToString(progress->totalUpload)), percent, (LPCTSTR)uploadSpeed);
        int columnIndex = isThumb ? 2 : 1;
        uploadListView_.SetItemText(fps->tableRow, columnIndex, ProgressBuffer);
    }
}

void CUploadDlg::onTaskFinished(UploadTask* task, bool ok)
{
    
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (!fileTask)
    {
        return;
    }
    if (fileTask->role() == UploadTask::DefaultRole) {
        FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(task->userData());
        if (!fps)
        {
            return;
        }
        CUrlListItem item;
        UploadResult* uploadResult = task->uploadResult();
        item.ImageUrl = Utf8ToWCstring(uploadResult->directUrl);
        item.ImageUrlShortened = Utf8ToWCstring(uploadResult->directUrlShortened);
        item.FileName = Utf8ToWCstring(fileTask->getDisplayName());
        item.DownloadUrl = Utf8ToWCstring(uploadResult->downloadUrl);
        item.DownloadUrlShortened = Utf8ToWCstring(uploadResult->downloadUrlShortened);
        item.ThumbUrl = Utf8ToWCstring(uploadResult->thumbUrl);
        UrlList[fps->tableRow] = item;
        //uploadListView_.SetItemText(fps->tableRow, 1, _T("Готово"));
        TotalUploadProgress(uploadSession_->finishedTaskCount(UploadTask::StatusFinished), uploadSession_->taskCount(), 0);
    }
     else if (fileTask->role() == UploadTask::ThumbRole) {
         FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(task->parentTask()->userData());
         if (!fps)
         {
             return;
         }
         uploadListView_.SetItemText(fps->tableRow, 2, _T("Готово"));
     }
        
    GenerateOutput();    
}

void CUploadDlg::onChildTaskAdded(UploadTask* child)
{

    if (!backgroundThreadStarted_)
    {
        backgroundThreadStarted();
    }
    if (child->role() == UploadTask::UrlShorteningRole)
    {
        FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(child->parentTask()->userData());
        if (fps)
        {
            uploadListView_.SetItemText(fps->tableRow, 1, _T("Сокращение ссылки..."));
        }
    }
    child->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &CUploadDlg::onTaskFinished));
    child->OnUploadProgress.bind(this, &CUploadDlg::onTaskUploadProgress);
    child->OnStatusChanged.bind(this, &CUploadDlg::OnUploaderStatusChanged);
}

void CUploadDlg::backgroundThreadStarted()
{
    std::lock_guard<std::mutex> lock(backgroundThreadStartedMutex_);
    if (backgroundThreadStarted_)
    {
        return;
    }
    backgroundThreadStarted_ = true;
    EnablePrev(false);
    if (!isEnableNextButtonTimerRunning_)
    {
        EnableNext();
    }
    
    EnableExit(false);
    SetNextCaption(TR("Stop"));
}



LRESULT CUploadDlg::OnUploadProcessButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    showUploadProgressTab();
    return 0;
}

LRESULT CUploadDlg::OnUploadResultsButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    showUploadResultsTab();
    return 0;
}