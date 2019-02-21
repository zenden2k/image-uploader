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

#include "UploadDlg.h"

#include <shobjidl.h>
#include <boost/format.hpp>

#include "Gui/Dialogs/LogWindow.h"
#include "Core/Settings.h"
#include "Core/Upload/UploadEngine.h"
#include "Gui/GuiTools.h"
#include "Core/3rdpart/FastDelegate.h"
#include "Core/Upload/FileQueueUploader.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UploadManager.h"
#include "Func/WinUtils.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Func/MediaInfoHelper.h"
#include "Func/IuCommonFunctions.h"
#include "Core/AppParams.h"
#include "Core/ServiceLocator.h"
#include "Core/Upload/UrlShorteningTask.h"


// CUploadDlg
CUploadDlg::CUploadDlg(CWizardDlg *dlg, UploadManager* uploadManager) : resultsWindow_(new CResultsWindow(dlg, urlList_, true))
{
    MainDlg = nullptr;
    engineList_ = ServiceLocator::instance()->myEngineList();
    backgroundThreadStarted_ = false;
    isEnableNextButtonTimerRunning_ = false;
    uploadManager_ = uploadManager;
    resultsWindow_->setOnShortenUrlChanged(fastdelegate::MakeDelegate(this, &CUploadDlg::onShortenUrlChanged));
    #if  WINVER    >= 0x0601
        ptl = nullptr;
    #endif
}

CUploadDlg::~CUploadDlg()
{
}

LRESULT CUploadDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    uploadProgressBar_ = GetDlgItem(IDC_UPLOADPROGRESS);
    imageViewWindow_.Create(m_hWnd);
    // Initializing Windows 7 taskbar related stuff
    RECT rc;
    ::GetWindowRect(GetDlgItem(IDC_RESULTSPLACEHOLDER), &rc);
    ::MapWindowPoints(0,m_hWnd, reinterpret_cast<POINT*>(&rc), 2);

    uploadListView_.AttachToDlgItem(m_hWnd, IDC_UPLOADTABLE);
    uploadListView_.AddColumn(TR("File"), 1);
    uploadListView_.AddColumn(TR("Status"), 1);
    uploadListView_.AddColumn(TR("Thumbnail"), 2);
    CWindowDC hdc(m_hWnd);
    float dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    //float dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    uploadListView_.SetColumnWidth(0, static_cast<int>(170 * dpiScaleX));
    uploadListView_.SetColumnWidth(1, static_cast<int>(170 * dpiScaleX));
    uploadListView_.SetColumnWidth(2, static_cast<int>(170 * dpiScaleX));

    createToolbar();

    resultsWindow_->Create(m_hWnd);
    resultsWindow_->SetWindowPos(0,&rc,0);
    #if  WINVER    >= 0x0601
        const GUID IID_ITaskbarList3 = { 0xea1afb91,0x9e28,0x4b86,{0x90,0xe9,0x9e,0x9f, 0x8a,0x5e,0xef,0xaf}};
        CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&ptl);
    #endif

    TRC(IDC_COMMONPROGRESS, "Progress:");
    bool IsLastVideo = false;
    if (MediaInfoHelper::IsMediaInfoAvailable()) {
        CVideoGrabberPage *vg = static_cast<CVideoGrabberPage*>(WizardDlg->Pages[1]);

        if (vg && !vg->m_szFileName.IsEmpty())
            IsLastVideo = true;
    }

	resultsWindow_->EnableMediaInfo(IsLastVideo);

    SetDlgItemInt(IDC_THUMBSPERLINE, 4);
    SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
    
    commonProgressLabelFont_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPROGRESS));
    commonPercentLabelFont_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPERCENTS));
    PageWnd = m_hWnd;
    resultsWindow_->SetPage(static_cast<CResultsPanel::TabPage>(Settings.CodeLang));
    resultsWindow_->SetCodeType(Settings.CodeType);
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
    uploadProgressBar_.SetPos(0);
    uploadProgressBar_.SetRange(0, n);
    SetDlgItemText(IDC_COMMONPERCENTS, _T("0%"));

    TotalUploadProgress(0, n);
    SetTimer(kProgressTimer, 1000);
    uploadListView_.DeleteAllItems();

    urlList_.clear();
    urlList_.resize(n);
    
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
        bool isImage = IuCommonFunctions::IsImage(FileName);
        auto task = std::make_shared<FileUploadTask>(fileNameA, displayName);
        task->OnUploadProgress.bind(this, &CUploadDlg::onTaskUploadProgress);
        task->setUserData(fps);
        task->setIndex(i);
        task->setIsImage(isImage);
        task->OnStatusChanged.bind(this, &CUploadDlg::OnUploaderStatusChanged);
        task->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &CUploadDlg::onTaskFinished));
        task->setServerProfile(isImage ? sessionImageServer_ : sessionFileServer_);
        task->OnChildTaskAdded.bind(this, &CUploadDlg::onChildTaskAdded);
        task->setUrlShorteningServer(Settings.urlShorteningServer);
        task->OnFolderUsed.bind(this, &CUploadDlg::OnFolderUsed);
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
    } else if (wParam == kProgressTimer) {
        updateTotalProgress();
    }
    return 0;
}

LRESULT CUploadDlg::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    HWND hwnd = reinterpret_cast<HWND>(wParam);
    POINT ClientPoint, ScreenPoint;
    if (hwnd != GetDlgItem(IDC_UPLOADTABLE)) {
        return 0;
    }

    if (!uploadSession_ || uploadSession_->isRunning()) {
        return 0;
    }

    if (lParam == -1) {
        ClientPoint.x = 0;
        ClientPoint.y = 0;
        ScreenPoint = ClientPoint;
        ::ClientToScreen(hwnd, &ScreenPoint);
    } else {
        ScreenPoint.x = GET_X_LPARAM(lParam);
        ScreenPoint.y = GET_Y_LPARAM(lParam);
        ClientPoint = ScreenPoint;
        ::ScreenToClient(hwnd, &ClientPoint);
    }

    int nCurItem = uploadListView_.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
    bool menuItemEnabled = false;
    if (nCurItem >= 0) {
        auto task = uploadSession_->getTask(nCurItem);
        if (task) {
            auto status = task->status();
            if (status != UploadTask::StatusFinished && status != UploadTask::StatusRunning) {
                menuItemEnabled = true;
            }

        }
    }

    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, ID_RETRYUPLOAD, TR("Retry"));
    menu.EnableMenuItem(ID_RETRYUPLOAD, menuItemEnabled ? MF_ENABLED : MF_GRAYED);
    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
    return 0;
}

int CUploadDlg::ThreadTerminated(void)
{
    WizardDlg->QuickUploadMarker = false;

    #if  WINVER    >= 0x0601
        if(ptl)
            ptl->SetProgressState(GetParent(), TBPF_NOPROGRESS);
    #endif
    KillTimer(kProgressTimer);
    SetNextCaption(TR("Finish >"));
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

    if ( MediaInfoHelper::IsMediaInfoAvailable()) {
        CVideoGrabberPage *vg = WizardDlg->getPage<CVideoGrabberPage>(CWizardDlg::wpVideoGrabberPage);
        
        if(vg && !vg->m_szFileName.IsEmpty())
            IsLastVideo=true;
    }
    resultsWindow_->InitUpload();
    resultsWindow_->EnableMediaInfo(IsLastVideo);
    CancelByUser = false;
    ShowNext();
    ShowPrev();
    EnableNext(false);
    isEnableNextButtonTimerRunning_ = true;
    SetTimer(kEnableNextButtonTimer, 1000);
    MainDlg = WizardDlg->getPage<CMainDlg>(CWizardDlg::wpMainPage);
    //Toolbar.CheckButton(IDC_USETEMPLATE,Settings.UseTxtTemplate);
    urlList_.clear();
    resultsWindow_->Clear();
    resultsWindow_->setShortenUrls(sessionImageServer_.shortenLinks());

    int code = resultsWindow_->GetCodeType();
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
    resultsWindow_->SetCodeType(newcode);
    resultsWindow_->SetPage(static_cast<CResultsPanel::TabPage>(Settings.CodeLang));

    ::SetFocus(GetDlgItem(IDC_CODEEDIT));
    alreadyShortened_ = false;
    backgroundThreadStarted();
    startUpload();
    return true;
}

bool CUploadDlg::OnNext() {
    if (uploadSession_->isRunning()) {
        uploadSession_->stop();
        CancelByUser = true;
    }
    else {
        MainDlg->ThumbsView.MyDeleteAllItems();
        EnableExit();
        return true;
    }
    return false;
}

bool CUploadDlg::OnHide()
{
    urlList_.clear();
    resultsWindow_->Clear();
    uploadManager_->removeSession(uploadSession_);
    uploadSession_.reset();
    for (auto it : files_) {
        delete it;
    }
    files_.clear();
    
    Settings.UseTxtTemplate = (SendDlgItemMessage(IDC_USETEMPLATE, BM_GETCHECK) == BST_CHECKED);
    Settings.CodeType = resultsWindow_->GetCodeType();
    Settings.CodeLang = resultsWindow_->GetPage();
    return true; 
}

void CUploadDlg::GenerateOutput()
{
    resultsWindow_->UpdateOutput();
}

void CUploadDlg::TotalUploadProgress(int CurPos, int Total, int FileProgress)
{
    uploadProgressBar_.SetPos(CurPos);
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

    if (Total) {
        CString buffer;
        buffer.Format(_T("%d %%"), static_cast<int>(100 * (static_cast<double>(CurPos) / Total)));
        SetDlgItemText(IDC_COMMONPERCENTS, buffer);
    }
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
        CString statusText = IuCoreUtils::Utf8ToWstring(progress->statusText).c_str();

        bool isThumb = task->role() == UploadTask::ThumbRole;
        int columnIndex = isThumb ? 2 : 1;
        ServiceLocator::instance()->taskDispatcher()->runInGuiThread([&] {
            uploadListView_.SetItemText(fps->tableRow, columnIndex, statusText);
        });
    }

    UrlShorteningTask* urlTask = dynamic_cast<UrlShorteningTask*>(task);
    if (urlTask ) {
        UploadTask* parentTask = urlTask->parentTask();
        
        if (urlTask->isFinished() && parentTask && parentTask->isFinished()) {
            CString statusText = U2W(parentTask->progress()->statusText);
            uploadListView_.SetItemText(fps->tableRow, 1, statusText);
        }
        
    }
}

void CUploadDlg::OnFolderUsed(UploadTask* task) {
    resultsWindow_->AddServer(task->serverProfile());
}

void CUploadDlg::onShortenUrlChanged(bool shortenUrl) {
    if ( !alreadyShortened_ && shortenUrl ) {
        GenerateOutput();
        
        uploadManager_->shortenLinksInSession(uploadSession_);
    } else {
        GenerateOutput();
    }
}

void CUploadDlg::createToolbar()
{
    CBitmap hBitmap;
    
    DWORD rtlStyle = Lang.isRTL() ? ILC_MIRROR | ILC_PERITEMMIRROR : 0;

    if (GuiTools::Is32BPP()) {
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_UPLOADTOOLBARBMP32BIT));
        toolbarImageList_.Create(16, 16, ILC_COLOR32 | rtlStyle, 0, 6);
        toolbarImageList_.Add(hBitmap, nullptr);
    }
    else {
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_UPLOADTOOLBARBMP16BIT));
        toolbarImageList_.Create(16, 16, ILC_COLOR32 | ILC_MASK | rtlStyle, 0, 6);
        toolbarImageList_.Add(hBitmap, RGB(255, 0, 255));
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
    toolbar_.SetImageList(toolbarImageList_);
    toolbar_.AddButton(IDC_UPLOADPROCESSTAB, BTNS_CHECK | BTNS_AUTOSIZE, TBSTATE_ENABLED | TBSTATE_PRESSED, 5, TR("Upload progress"), 0);
    toolbar_.AddButton(IDC_UPLOADRESULTSTAB, BTNS_CHECK | BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Links"), 0);
    toolbar_.AddButton(IDC_VIEWLOG, TBSTYLE_BUTTON | BTNS_AUTOSIZE, TBSTATE_ENABLED, 3, TR("Log"), 0);

    toolbar_.AutoSize();
    toolbar_.SetWindowLong(GWL_ID, IDC_RESULTSTOOLBAR);
    toolbar_.ShowWindow(SW_SHOW);
}

void CUploadDlg::updateTotalProgress() {
    if (!uploadSession_) {
        return;
    }
    int n = uploadSession_->taskCount();
    int totalPercent = 0;
    for (int i = 0; i < n; i++) {
        std::shared_ptr<UploadTask> task = uploadSession_->getTask(i);
        if (task->isRunning()) {
            int percent = 0;
            UploadProgress* progress = task->progress();
            if (progress->totalUpload) {
                percent = static_cast<int>(100 * ((float)progress->uploaded) / progress->totalUpload);
                totalPercent += percent;
            }
        }
    }
    TotalUploadProgress(uploadSession_->finishedTaskCount(UploadTask::StatusFinished), uploadSession_->taskCount(),
        static_cast<int>(((float)totalPercent) / 100 * 50));
}

LRESULT CUploadDlg::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    LogWindow.Show();
    return 0;
}

LRESULT CUploadDlg::OnUploadTableDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
    LPNMITEMACTIVATE lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
    int row = lpnmitem->iItem;
    if (row >= 0) {
        auto item = files_[row];
        if (IuCommonFunctions::IsImage(item->fileName)) {
            CImageViewItem imgViewItem;
            imgViewItem.index = 0;
            imgViewItem.fileName = item->fileName;
            HWND wizardDlg = WizardDlg->m_hWnd;
            imageViewWindow_.ViewImage(imgViewItem, wizardDlg);
        }
    }
    return 0;
}

LRESULT CUploadDlg::OnRetryUpload(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    int nCurItem = uploadListView_.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
    if (nCurItem >= 0) {
        auto task = uploadSession_->getTask(nCurItem);
        if (task) {
            UploadTask::Status status = task->status();
            if (status != UploadTask::StatusFinished && status != UploadTask::StatusRunning) {
                if (!uploadManager_->IsRunning()) {
                    SetDlgItemText(IDC_COMMONPROGRESS2, _T(""));
                    backgroundThreadStarted();
                    uploadManager_->retrySession(uploadSession_);
                }
            }
        }
    }

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
    resultsWindow_->ShowWindow(SW_SHOW);
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
    resultsWindow_->ShowWindow(SW_HIDE);
}

// This callback is being executed in worker thread
void CUploadDlg::onSessionFinished(UploadSession* session)
{
    ServiceLocator::instance()->taskDispatcher()->runInGuiThread([&] {
        onSessionFinished_UiThread(session);
    });
}

// This function is being executed in UI thread
void CUploadDlg::onSessionFinished_UiThread(UploadSession* session) {
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
        if (Settings.AutoCopyToClipboard) {
            resultsWindow_->copyResultsToClipboard();
        }
        showUploadResultsTab();
    }
}

// This callback is being executed in worker thread
void CUploadDlg::onTaskUploadProgress(UploadTask* task)
{
    FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(task->role() == UploadTask::DefaultRole ? task->userData() : task->parentTask()->userData());
    if (!fps)
    {
        return;
    }
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (fileTask) {
        
        bool isThumb = task->role() == UploadTask::ThumbRole;
        int percent = 0;
        UploadProgress* progress = task->progress();
        if (progress->totalUpload) {
            percent = static_cast<int>(100 * ((float)progress->uploaded) / progress->totalUpload);
        }
        CString uploadSpeed = Utf8ToWCstring(progress->speed);
        CString progressText;
        progressText.Format(TR("%s of %s (%d%%) %s"), (LPCTSTR)U2W(IuCoreUtils::fileSizeToString(progress->uploaded)),
            (LPCTSTR)U2W(IuCoreUtils::fileSizeToString(progress->totalUpload)), percent, (LPCTSTR)uploadSpeed);
       
        int columnIndex = isThumb ? 2 : 1;
        ServiceLocator::instance()->taskDispatcher()->runInGuiThread([&] {  
            uploadListView_.SetItemText(fps->tableRow, columnIndex, progressText);
        });
       
    }
}

// This callback is being executed in worker thread
void CUploadDlg::onTaskFinished(UploadTask* task, bool ok)
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (!fileTask)
    {
        return;
    }
    auto taskDispatcher = ServiceLocator::instance()->taskDispatcher();
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
        urlList_[fps->tableRow] = item;
        //uploadListView_.SetItemText(fps->tableRow, 1, _T("Готово"));
        taskDispatcher->runInGuiThread([this] { 
            updateTotalProgress(); 
        });
        
        //TotalUploadProgress(uploadSession_->finishedTaskCount(UploadTask::StatusFinished), uploadSession_->taskCount(), 0);
    }
    else if (fileTask->role() == UploadTask::ThumbRole) {
         FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(task->parentTask()->userData());
         if (!fps)
         {
             return;
         }
         taskDispatcher->runInGuiThread([&]{
             uploadListView_.SetItemText(fps->tableRow, 2, TR("Finished"));
         });
         
     }
        
    GenerateOutput();    
}

void CUploadDlg::onChildTaskAdded(UploadTask* child)
{
    auto dispatcher = ServiceLocator::instance()->taskDispatcher();
    if (!backgroundThreadStarted_)
    {
        dispatcher->runInGuiThread([&] {
            backgroundThreadStarted();
        });
    }
    if (child->role() == UploadTask::UrlShorteningRole)
    {
        FileProcessingStruct* fps = reinterpret_cast<FileProcessingStruct*>(child->parentTask()->userData());
        if (fps)
        {
            dispatcher->runInGuiThread([&] {
                uploadListView_.SetItemText(fps->tableRow, 1, _T("Сокращение ссылки..."));
            });
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