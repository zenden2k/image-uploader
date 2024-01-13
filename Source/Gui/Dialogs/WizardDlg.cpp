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
#include "WizardDlg.h"

#include <ShObjIdl.h>
#include <ComDef.h>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "Core/Images/ImageConverter.h"
#include "Core/ServiceLocator.h"
#include "Core/HistoryManager.h"
#include "WelcomeDlg.h"
#include "MainDlg.h"
#include "VideoGrabberPage.h"
#include "UploadSettings.h"
#include "UploadDlg.h"
#include "AboutDlg.h"
#include "FloatingWindow.h"
#include "ImageDownloaderDlg.h"
#include "LogWindow.h"
#include "Func/CmdLine.h"
#include "Gui/Dialogs/UpdateDlg.h"
#ifdef IU_ENABLE_MEDIAINFO
#include "Gui/Dialogs/MediaInfoDlg.h"
#include "Func/MediaInfoHelper.h"
#endif
#include "Gui/GuiTools.h"
#include "Gui/Dialogs/ImageReuploaderDlg.h"
#include "Gui/Dialogs/ShortenUrlDlg.h"
#include "Gui/Dialogs/WebViewWindow.h"
#include "Func/WinUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Gui/Dialogs/QuickSetupDlg.h"
#include "ImageEditor/Gui/ImageEditorWindow.h"
#include "Func/ImageEditorConfigurationProvider.h"
#include "Core/Logging.h"
#include "Core/Images/Utils.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/Scripting/ScriptsManager.h"
#include "Func/MyUtils.h"
#include "Core/Utils/DesktopUtils.h"
#include "Gui/Win7JumpList.h"
#include "Core/AppParams.h"
#include "Gui/Components/MyFileDialog.h"
#include "Core/ScreenCapture/MonitorEnumerator.h"
#include "Core/Network/NetworkClientFactory.h"
#include "Gui/Components/NewStyleFolderDialog.h"
#include "statusdlg.h"
#include "ServerListTool/ServersCheckerDlg.h"

using namespace Gdiplus;
namespace
{

struct TaskDispatcherMessageStruct {
    TaskRunnerTask callback;
    bool async;
    //Object* sender;
};

CString MakeTempFileName(const CString& FileName)
{
    CString FileNameBuf = AppParams::instance()->tempDirectoryW() + FileName;

    if (WinUtils::FileExists(FileNameBuf))
    {
        CString OnlyName = WinUtils::GetOnlyFileName(FileName);
        CString Ext = WinUtils::GetFileExt(FileName);
        FileNameBuf = AppParams::instance()->tempDirectoryW() + OnlyName + _T("_") + WinUtils::IntToStr(GetTickCount() ^ 33333) + (Ext ? _T(".") : _T("")) + Ext;
    }
    return FileNameBuf;
}

bool SaveFromHGlobal(HGLOBAL Data, const CString& FileName, LARGE_INTEGER* size, CString& OutName)
{
    if (!Data) return false;
    CString FileNameBuf = MakeTempFileName(FileName);

    size_t filesize = size ? size->QuadPart : GlobalSize(Data);

    if (!filesize) {
        return false;
    }

    PVOID LockedData = (PVOID)GlobalLock(Data);

    HANDLE hFile = CreateFile(FileNameBuf, GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        GlobalUnlock(Data);
        return false;
    }

    ULONG cbRead;

    WriteFile(hFile, LockedData, filesize, &cbRead, NULL);

    CloseHandle(hFile);
    GlobalUnlock(Data);
    OutName = FileNameBuf;
    return true;
}

bool SaveFromIStream(IStream *pStream, const CString& FileName, CString &OutName)
{
    if (!pStream) return false;
    CString FileNameBuf = MakeTempFileName(FileName);

    HANDLE hFile = CreateFile(FileNameBuf, GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    UCHAR bBuffer[65536];
    ULONG cbRead;

    while (SUCCEEDED(pStream->Read(bBuffer, sizeof(bBuffer), &cbRead)) && cbRead > 0)
    {
        WriteFile(hFile, bBuffer, cbRead, &cbRead, NULL);
    }

    CloseHandle(hFile);
    OutName = FileNameBuf;
    return true;
}

}

// CWizardDlg
CWizardDlg::CWizardDlg(std::shared_ptr<DefaultLogger> logger, CMyEngineList* enginelist, 
    UploadEngineManager* uploadEngineManager, UploadManager* uploadManager, 
    ScriptsManager* scriptsManager, WtlGuiSettings* settings):
    FolderAdd(this), 
    uploadManager_(uploadManager),
    uploadEngineManager_(uploadEngineManager), 
    scriptsManager_(scriptsManager),
    Settings(*settings),
    logger_(std::move(logger)),
    enginelist_(enginelist)
{ 
    mainThreadId_ = GetCurrentThreadId();
    screenshotIndex = 1;
    CurPage = -1;
    PrevPage = -1;
    NextPage = -1;
    ZeroMemory(Pages, sizeof(Pages));
    DragndropEnabled = true;
    QuickUploadMarker = false;
    m_bShowAfter = true;
    m_bHandleCmdLineFunc = false;
    m_bScreenshotFromTray = false;
    serversChanged_ = false;
    using namespace std::placeholders;
    settingsChangedConnection_ = Settings.onChange.connect(std::bind(&CWizardDlg::settingsChanged, this, _1)); 
}

void CWizardDlg::settingsChanged(BasicSettings* settingsBase) {
    auto* settings = dynamic_cast<CommonGuiSettings*>(settingsBase);
    if (settings) {
        if (!settings->imageServer.isEmpty()) {
            const std::string templateName = settings->imageServer.getByIndex(0).getImageUploadParamsRef().getThumbRef().TemplateName;
            if (sessionImageServer_.isEmpty()) {
                sessionImageServer_.getByIndex(0).getImageUploadParamsRef().getThumbRef().TemplateName = templateName;
            }
          
        }
    }

    if (!(m_hotkeys == Settings.Hotkeys)) {
        UnRegisterLocalHotkeys();
        RegisterLocalHotkeys();
    }
}

bool CWizardDlg::pasteFromClipboard() {
    if (IsClipboardFormatAvailable(CF_BITMAP)) {
        if (!OpenClipboard()) {
            return false;
        }
        HBITMAP bmp = static_cast<HBITMAP>(GetClipboardData(CF_BITMAP));

        if (!bmp) {
            CloseClipboard();
            return false;
        }

        PasteBitmap(bmp);
        CloseClipboard();
        return true;
    }

    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        CString text;
        WinUtils::GetClipboardText(text);
        CString outFileName;
        if (ImageUtils::SaveImageFromCliboardDataUriFormat(text, outFileName)) {
            CreatePage(wpMainPage);
            CMainDlg* MainDlg = getPage<CMainDlg>(wpMainPage);
            if (MainDlg) {
                MainDlg->AddToFileList(outFileName, L"", true, nullptr, true);
                return true;
            }
        } 
        if (CImageDownloaderDlg::LinksAvailableInText(text)) {
            CImageDownloaderDlg dlg(this, text);
            dlg.EmulateModal(m_hWnd);
            if (dlg.successfullDownloadsCount()) {
                return true;
            }
        }
    }
    return false;
}

CWizardDlg::~CWizardDlg()
{
    for (auto* page: Pages) {
        delete page;
    }

    for (auto& logWnd : logWindowsByFileName_) {
        logWnd.second->DestroyWindow();
    }
    settingsChangedConnection_.disconnect();
}

void CWizardDlg::setFloatWnd(std::shared_ptr<CFloatingWindow> floatWnd) {
    floatWnd_ = std::move(floatWnd);
}

LRESULT CWizardDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT clientRect;
    GetClientRect(&clientRect);
    auto* translator = ServiceLocator::instance()->translator();
    ATLASSERT(translator != nullptr);

    m_bShowWindow = true;

    win7JumpList_ = std::make_unique<Win7JumpList>();

    LPDWORD DlgCreationResult = reinterpret_cast<LPDWORD>(lParam);

    ATLASSERT(DlgCreationResult != NULL);

    // center the dialog on the screen
    CenterWindow();
    hIcon = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    SetIcon(hIcon, TRUE);

    hIconSmall = GuiTools::LoadSmallIcon(IDR_MAINFRAME);
    SetIcon(hIconSmall, FALSE);

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    ::RegisterDragDrop(m_hWnd, this);
#ifdef IU_ENABLE_MEDIAINFO
    MediaInfoHelper::FindMediaInfoDllPath();
#endif
    SetWindowText(APPNAME);

    const int iconWidth = GetSystemMetrics(SM_CXSMICON);
    const int iconHeight = GetSystemMetrics(SM_CYSMICON);
    helpButtonIcon_.LoadIconWithScaleDown(MAKEINTRESOURCE(IDI_ICON_HELP_DROPDOWN), iconWidth, iconHeight);

    helpButton_.m_hWnd = GetDlgItem(IDC_HELPBUTTON);
    helpButton_.SetIcon(helpButtonIcon_);
    ServiceLocator::instance()->logWindow()->TranslateUI();
    aboutButtonToolTip_ = GuiTools::CreateToolTipForWindow(GetDlgItem(IDC_HELPBUTTON), TR("Help"));

    CString ErrorStr;
    if(!LoadUploadEngines(IuCommonFunctions::GetDataFolder()+_T("servers.xml"), ErrorStr))
    {
        CString ErrBuf;
        ErrBuf.Format(TR("Couldn't load servers list file \"servers.xml\"!\n\nThe reason is:  %s\n\nDo you wish to continue?"),(LPCTSTR)ErrorStr);
    
        if (LocalizedMessageBox(ErrBuf, APPNAME, MB_ICONERROR | MB_YESNO) == IDNO)
        {
            *DlgCreationResult = 2;
            return 0;
        }
    }
    uploadEngineManager_->setScriptsDirectory(WCstringToUtf8(IuCommonFunctions::GetDataFolder() + _T("\\Scripts\\")));
    std::vector<CString> list;
    CString serversFolder = IuCommonFunctions::GetDataFolder() + _T("Servers\\");
    boost::filesystem::path serversFolderPath(serversFolder);

    WinUtils::GetFolderFileList(list, serversFolder, _T("*.xml"));

    for(size_t i=0; i<list.size(); i++)
    {
        LoadUploadEngines(serversFolder+list[i], ErrorStr);
    }
    list.clear();

    CString userServersFolder = Utf8ToWCstring(Settings.SettingsFolder + "Servers\\");
    boost::filesystem::path userServersFolderPath(userServersFolder);
    
    try {
        if (boost::filesystem::exists(userServersFolderPath) && boost::filesystem::canonical(userServersFolderPath) != boost::filesystem::canonical(serversFolderPath)) {
            WinUtils::GetFolderFileList(list, userServersFolder, _T("*.xml"));

            for (size_t i = 0; i < list.size(); i++)
            {
                LoadUploadEngines(userServersFolder + list[i], ErrorStr);
            }
        }
    } catch ( std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
    
    LoadUploadEngines(_T("userservers.xml"), ErrorStr);    

	Settings.fixInvalidServers();
    if ( isFirstRun_ ) {
        CQuickSetupDlg quickSetupDialog;
		if (quickSetupDialog.DoModal(m_hWnd) != IDOK){
			*DlgCreationResult = 2;
			return 0;
		}
    }
    auto* historyManager = ServiceLocator::instance()->historyManager();
    historyManager->setHistoryDirectory(Settings.SettingsFolder + "\\History\\");
    historyManager->openDatabase();

    if (isFirstRun_) {
        Settings.HistorySettings.HistoryConverted = true;
    }
    
    if (!isFirstRun_ && !Settings.HistorySettings.HistoryConverted) {
        statusDlg_.reset(new CStatusDlg(false));
        statusDlg_->SetAppWindow(true);
        statusDlg_->SetInfo(TR("Converting history"), TR("Please wait while your history is being converted..."));
        
        std::thread t([&]() {
            historyManager->convertHistory();
            Settings.HistorySettings.HistoryConverted = true;
            ServiceLocator::instance()->taskRunner()->runInGuiThread([this]
            {
                EnableWindow(TRUE);
                statusDlg_->ProcessFinished();
            });
        });
        t.detach();
        statusDlg_->DoModal();
    }

    sessionImageServer_ = Settings.imageServer;
    sessionFileServer_ = Settings.fileServer;
#ifdef IU_ENABLE_MEDIAINFO
	if (!MediaInfoHelper::IsMediaInfoAvailable()) {
        ServiceLocator::instance()->logger()->write(ILogger::logWarning, APPNAME, TR("MediaInfo.dll Not found! \nGetting technical information of media files will not be accessible."));
	}
#endif
    if (!CmdLine.IsOption(_T("tray"))) {
        TRC(IDCANCEL, "Exit");
    } else {
        TRC(IDCANCEL, "Hide");
    }
    //TRC(IDC_UPDATESLABEL, "Check for Updates");
    TRC(IDC_PREV, "< Back");

    bool layeredChildAvailable = IsWindows8OrGreater();
    dragndropOverlay_.Create(m_hWnd, clientRect, 0, WS_CHILD | WS_CLIPSIBLINGS, layeredChildAvailable ? WS_EX_LAYERED : 0);
    if (layeredChildAvailable) {
        SetLayeredWindowAttributes(dragndropOverlay_, 0, 200, LWA_ALPHA);
    }
    dragndropOverlay_.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    // Set WS_CLIPSIBLINGS style for correct display of CDragndropOverlay on Windows 7
    HWND exitBtn = GetDlgItem(IDCANCEL);
    ::SetWindowLong(exitBtn, GWL_STYLE, ::GetWindowLong(exitBtn, GWL_STYLE) | WS_CLIPSIBLINGS);
    HWND nextBtn = GetDlgItem(IDC_NEXT);
    ::SetWindowLong(nextBtn, GWL_STYLE, ::GetWindowLong(nextBtn, GWL_STYLE) | WS_CLIPSIBLINGS);
    HWND prevBtn = GetDlgItem(IDC_PREV);
    ::SetWindowLong(prevBtn, GWL_STYLE, ::GetWindowLong(prevBtn, GWL_STYLE) | WS_CLIPSIBLINGS);

    helpButton_.SetWindowLong(GWL_STYLE, helpButton_.GetWindowLong(GWL_STYLE) | WS_CLIPSIBLINGS);

    SetTimer(kNewFilesTimer, 500);
    RegisterLocalHotkeys();
    if(ParseCmdLine()) return 0;
 
    CreatePage(wpWelcomePage); 
    ShowPage(wpWelcomePage);
    Pages[wpWelcomePage]->SetInitialFocus();

    if(CmdLine.IsOption(_T("update")))
    {
        CreateUpdateDlg();
        updateDlg->ShowModal(m_hWnd);
    }
    else
    {
        if(Settings.AutomaticallyCheckUpdates && (time(0) - Settings.LastUpdateTime > 3600*24*3 /* 3 days */))
        {
            CreateUpdateDlg();
            updateDlg->Create(m_hWnd);
        }
    }

    return 0;  
}

bool CWizardDlg::ParseCmdLine()
{
    size_t nIndex = 0;
    bool fromContextMenu = false;
#ifdef IU_ENABLE_MEDIAINFO
    if(CmdLine.IsOption(_T("mediainfo")))
    {
        size_t nIndex = 0;
        CString VideoFileName;
        if(CmdLine.GetNextFile(VideoFileName, nIndex))
        {
            CMediaInfoDlg dlg;
            dlg.ShowInfo(m_hWnd, VideoFileName);
            PostQuitMessage(0);
            return true;
        }
    }
#endif
    if(CmdLine.IsOption(_T("imageeditor")))
    {
        size_t nIndex = 0;
        CString imageFileName;
        if(CmdLine.GetNextFile(imageFileName, nIndex))
        {
            using namespace ImageEditor;
            ImageEditorConfigurationProvider configProvider;
            ImageEditor::ImageEditorWindow imageEditor(imageFileName, &configProvider);
            imageEditor.showUploadButton(false);
            m_bShowWindow=false;
            ImageEditorWindow::DialogResult dr = imageEditor.DoModal(m_hWnd, nullptr, ImageEditorWindow::wdmWindowed);
            if (dr == ImageEditorWindow::drCancel) {
                PostQuitMessage(0);    
            } else if (dr != ImageEditorWindow::drCopiedToClipboard){
                this->AddImage(imageFileName, WinUtils::myExtractFileName(imageFileName), true);
                //ShowPage(1);
                m_bShowAfter = true;
                m_bShowWindow = true;
                m_bHandleCmdLineFunc = true;
            }
            return true;
        }
    }

    if (CmdLine.IsOption(_T("serverschecker"))) {
        ServersListTool::CServersCheckerDlg dlg(&Settings, uploadEngineManager_, uploadManager_, enginelist_, std::make_shared<NetworkClientFactory>());
        dlg.DoModal(m_hWnd);
    }

    for(size_t i=0; i<CmdLine.GetCount(); i++)
    {
        CString CurrentParam = CmdLine[i];
        if ( CurrentParam == _T("/quickshot")  ) {
            m_bShowWindow=false;
            m_bHandleCmdLineFunc = true;
            if(!executeFunc(_T("regionscreenshot"), true))
                PostQuitMessage(0);
            return true;
        }
         else if(CurrentParam .Left(6)==_T("/func="))
        {
            m_bShowWindow=false;
            CString cmd = CurrentParam.Right(CurrentParam.GetLength()-6);
            m_bHandleCmdLineFunc = true;
            if(!executeFunc(cmd, true))
                PostQuitMessage(0);
            return true;
        } else if(CurrentParam .Left(15)==_T("/serverprofile=")) {
            CString serverProfileName = CurrentParam.Right(CurrentParam.GetLength()-15);
            
            if ( Settings.ServerProfiles.find(serverProfileName) == Settings.ServerProfiles.end()) {
                CString msg;
                msg.Format(TR("Profile \"%s\" not found.\nIt may be caused by a configuration error or usage of multiple versions of the application on the same computer."), 
                    serverProfileName.GetString());
                LocalizedMessageBox(msg, APPNAME, MB_ICONWARNING);
                CmdLine.RemoveOption(_T("quick"));
            } else {
                ServerProfile & sp = Settings.ServerProfiles[serverProfileName];
                CUploadEngineData *ued = sp.uploadEngineData();
                if ( ued ) {
                    if ( ued ->hasType(CUploadEngineData::TypeFileServer) ) {
                        sessionImageServer_ = sp;
                        sessionFileServer_ = sp;
                        serversChanged_ = true;
                        
                    } else if ( ued ->hasType(CUploadEngineData::TypeImageServer) ) {
                        sessionImageServer_ = sp;
                        serversChanged_ = true; 
                    }
                } else {
                    //MessageBox(_T("Server not found"));
                }
                
            }
            
        } else if (CurrentParam ==_T("/fromcontextmenu")) {
            sessionImageServer_ = Settings.contextMenuServer;
            fromContextMenu = true;
        }
    }

	CString FileName;
	
	if(CmdLine.GetNextFile(FileName, nIndex))
	{
		if(IsVideoFile(FileName) && !CmdLine.IsOption(_T("upload")) && !CmdLine.IsOption(_T("quick")))
		{
			ShowPage(wpVideoGrabberPage, CurPage, (Pages[wpMainPage]) ? wpMainPage : wpUploadSettingsPage);
            CVideoGrabberPage* dlg = getPage<CVideoGrabberPage>(wpVideoGrabberPage);
			dlg->SetFileName(FileName);			
			return true;
		}	
	}
	nIndex = 0;
	CStringList Paths;
	while(CmdLine.GetNextFile(FileName, nIndex))
	{
        if (WinUtils::FileExists(FileName) || WinUtils::IsDirectory(FileName)) {
            Paths.Add(WinUtils::ConvertRelativePathToAbsolute(FileName));
        }
	}
	if(!Paths.IsEmpty())
	{
		QuickUploadMarker = (fromContextMenu && Settings.QuickUpload && !CmdLine.IsOption(_T("noquick"))) || (CmdLine.IsOption(_T("quick")));	
		FolderAdd.Do(Paths, CmdLine.IsOption(_T("imagesonly")), true);
	}
    return false;
}

LRESULT CWizardDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(floatWnd_->m_hWnd)
    { 
        ShowWindow(SW_HIDE);
        if (Pages[wpMainPage] && CurPage == wpUploadPage) {
            getPage<CMainDlg>(wpMainPage)->ThumbsView.MyDeleteAllItems();
        }
        ShowPage(wpWelcomePage); 
    } else {
        CloseWizard();
    }
    return 0;
}

BOOL CWizardDlg::PreTranslateMessage(MSG* pMsg)
{    
    if( pMsg->message == WM_KEYDOWN)
    {
        TCHAR Buffer[MAX_PATH];
        GetClassName(pMsg->hwnd, Buffer, sizeof(Buffer)/sizeof(TCHAR));
        if( pMsg->wParam == 'A' && !lstrcmpi(Buffer,_T("Edit") ) && GetKeyState(VK_CONTROL)<0)
        {
            ::SendMessage(pMsg->hwnd, EM_SETSEL, 0, -1);    
            return TRUE;
        }
        if( pMsg->wParam == 'V' && !lstrcmpi(Buffer,_T("Edit")) ) {
            return FALSE;
        }

        if(VK_RETURN == pMsg->wParam  && GetForegroundWindow()==m_hWnd  )
        {
            if( !lstrcmpi(Buffer,_T("Button"))){
                ::SendMessage(pMsg->hwnd, BM_CLICK, 0 ,0); return TRUE;}
            else if (Pages[wpWelcomePage] && pMsg->hwnd==::GetDlgItem(Pages[wpWelcomePage]->PageWnd,IDC_LISTBOX))
                return FALSE;
        }
        
        /*if (VK_BACK == pMsg->wParam && Pages[CurPage] && GetForegroundWindow() == m_hWnd && lstrcmpi(Buffer, _T("Edit")))
        {
            if (pMsg->message == WM_KEYDOWN && ::IsWindowEnabled(GetDlgItem(IDC_PREV))) {
                OnPrevBnClicked(0, 0, 0);
                return TRUE;
            }
            
        }*/
    }

    if(localHotkeys_ &&TranslateAccelerator(m_hWnd, localHotkeys_, pMsg)) 
    {
        return TRUE;
    }
    
    return CWindow::IsDialogMessage(pMsg);
}

BOOL CWizardDlg::OnIdle()
{
    return FALSE;
}

LRESULT CWizardDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

    bHandled = false;
    return 0;
}

void CWizardDlg::CloseDialog(int nVal)
{
    if(updateDlg)
        updateDlg->Abort();
    ShowWindow(SW_HIDE);
    if(CurPage >= 0)
        Pages[CurPage]->OnHide();
    
    Exit();
    DestroyWindow();
    ::PostQuitMessage(nVal);
}

bool CWizardDlg::ShowPage(WizardPageId idPage, int prev, int next)
{
    if(idPage == CurPage) return true;

    if (GetCurrentThreadId() != GetWindowThreadProcessId(m_hWnd, NULL)) {
        return SendMessage(WM_MY_SHOWPAGE, (WPARAM)(int)idPage) != FALSE;
    }

    int oldCurPage = CurPage;

    if (oldCurPage >= 0) {
        Pages[oldCurPage]->OnHide();
    }

    if (!CreatePage(idPage)) return false;

    SetDlgItemText(IDC_NEXT, TR("Next >"));

    HBITMAP bmp = Pages[idPage]->HeadBitmap;
    if (!bmp) ::ShowWindow(GetDlgItem(IDC_HEADBITMAP), SW_HIDE);
    else {
        ::ShowWindow(GetDlgItem(IDC_HEADBITMAP), SW_SHOW);
        SendDlgItemMessage(IDC_HEADBITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
    }

    PrevPage = prev;
    NextPage = next;
    CurPage = idPage;
    ::ShowWindow(Pages[idPage]->PageWnd, SW_SHOW);
    ::SetFocus(Pages[idPage]->PageWnd);
    Pages[idPage]->OnShow();

    //::ShowWindow(GetDlgItem(IDC_HELPBUTTON), idPage == wpWelcomePage);

    if (oldCurPage >= 0) {
        ::ShowWindow(Pages[oldCurPage]->PageWnd, SW_HIDE);
    }
    return false;
}

LRESULT CWizardDlg::OnPrevBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    if(PrevPage<0)
    {
        PrevPage = CurPage-1;
        if(PrevPage<0 || PrevPage==1)  PrevPage = 0;
    }    

    ShowPage(static_cast<WizardPageId>(PrevPage));
    PrevPage=-1;
    return 0;
}

LRESULT CWizardDlg::OnNextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    if(!::IsWindowVisible(hWndCtl)) return 0;
    if (CurPage < 0) {
        LOG(ERROR) << "Impossible situation";
        return 0;
    }
    if(!Pages[CurPage]->OnNext()) return 0;
    if(NextPage < 0)
    {
        NextPage = CurPage+1;
        if(NextPage>4 ) NextPage=0;
        if(NextPage==1) NextPage=2;
    }    
    ShowPage(static_cast<WizardPageId>(NextPage));
    NextPage = -1;
    return 0;
}

bool CWizardDlg::CreatePage(WizardPageId PageID)
{
    RECT rc = {3,3,636,500};
    RECT rc2 = {3,100,636,500};

    if (Pages[PageID] != nullptr) {
        return true;
    }

    CClientDC dc(m_hWnd);
    //float dpiScaleX_ = dc.GetDeviceCaps(LOGPIXELSX) / 96.0f;
    float dpiScaleY_ = dc.GetDeviceCaps(LOGPIXELSY) / 96.0f;
    int height = static_cast<int>(roundf(45 * dpiScaleY_));


    switch(PageID)
    {
        case wpWelcomePage:
            CWelcomeDlg *tmp;
            tmp = new CWelcomeDlg();
            Pages[PageID] = tmp;
            Pages[PageID]->WizardDlg=this;
            tmp->Create(m_hWnd,rc);
            break;

        case wpVideoGrabberPage:
            CVideoGrabberPage *tmp1;
            tmp1 = new CVideoGrabberPage(uploadEngineManager_);
            Pages[PageID]=tmp1;
            Pages[PageID]->WizardDlg=this;
            tmp1->Create(m_hWnd,rc);
            break;

        case wpMainPage:
            CMainDlg *tmp2;
            tmp2=new CMainDlg();
            Pages[PageID]=tmp2;
            Pages[PageID]->WizardDlg=this;
            tmp2->Create(m_hWnd,rc);
            break;

        case wpUploadSettingsPage:
            CUploadSettings *tmp3;
            tmp3 = new CUploadSettings(enginelist_, uploadEngineManager_);
            Pages[PageID]=tmp3;
            Pages[PageID]->WizardDlg=this;
            tmp3->Create(m_hWnd,rc2);
            tmp3->SetWindowPos(0,0, height,0,0,SWP_NOSIZE);
            break;
        case wpUploadPage:
            CUploadDlg *tmp4;
            tmp4=new CUploadDlg(this, uploadManager_);
            Pages[PageID]=tmp4;
            Pages[PageID]->WizardDlg=this;
            tmp4->Create(m_hWnd, rc);
            tmp4->SetWindowPos(0, 0, height, 0, 0,SWP_NOSIZE);
            break;
        default:
            return false;
    }
    Pages[PageID]->HeadBitmap = GenHeadBitmap(PageID);
    return true;
}

void CWizardDlg::setSessionImageServer(const ServerProfileGroup& server)
{
    sessionImageServer_ = server;
}

void CWizardDlg::setSessionFileServer(const ServerProfileGroup& server)
{
    sessionFileServer_ = server;
}

ServerProfileGroup CWizardDlg::getSessionImageServer() const
{
    return sessionImageServer_;
}

ServerProfileGroup CWizardDlg::getSessionFileServer() const
{
    return sessionFileServer_;
}

void CWizardDlg::setServersChanged(bool changed)
{
    serversChanged_ = changed;
}

bool CWizardDlg::serversChanged() const
{
    return serversChanged_;
}

WindowHandle CWizardDlg::getHandle() {
    return m_hWnd;
}

WindowNativeHandle CWizardDlg::getNativeHandle() {
    return m_hWnd;
}

void CWizardDlg::ShowUpdateMessage(const CString& msg) {
    if ((CurPage == wpMainPage || CurPage == wpWelcomePage) && !IsWindowVisible() && IsWindowEnabled() && floatWnd_->m_hWnd) {
        std::wstring title = str(boost::wformat(TR("%s - Updates available")) % APPNAME);
        floatWnd_->ShowBaloonTip(msg, title.c_str(), 8000, [&] {
            CreateUpdateDlg();
            if (!updateDlg->IsWindowVisible()) {
                updateDlg->ShowModal(m_hWnd);
            }
        });
    }
}

HBITMAP CWizardDlg::GenHeadBitmap(WizardPageId PageID) const
{
    if (PageID != wpUploadSettingsPage && PageID != wpUploadPage) {
        return nullptr;
    }
    RECT rc;
    GetClientRect(&rc);
    int width=rc.right-rc.left;
    CClientDC dc(m_hWnd);
    //float dpiScaleX_ = dc.GetDeviceCaps(LOGPIXELSX) / 96.0f;
    float dpiScaleY_ = dc.GetDeviceCaps(LOGPIXELSY) / 96.0f;
    int height = static_cast<int>(roundf(45 * dpiScaleY_));
    RectF bounds(0.0,0.0, float(width), height);
   
    Graphics g(m_hWnd,true);
    std::unique_ptr<Bitmap> BackBuffer = std::make_unique<Bitmap>(width, height, &g);
    Graphics gr(BackBuffer.get());
    
    LinearGradientBrush 
        brush(bounds, Color(255, 255, 255, 255), Color(255, 235,235,235), 
            LinearGradientModeVertical);
    gr.FillRectangle(&brush,bounds);

    LinearGradientBrush 
        br2(bounds, Color(130, 190, 190, 190), Color(255, 70, 70, 70), 
            LinearGradientModeBackwardDiagonal); 

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);
    Gdiplus::Font font(L"Arial", 12, FontStyleBold);

    if(PageID == 3)
        gr.DrawString(TR("Images settings and choosing server"), -1, &font, bounds, &format, &br2);
    else if(PageID==4)
        gr.DrawString(TR("Uploading file on server"), -1, &font, bounds, &format, &br2);

    HBITMAP bmp = nullptr;
    BackBuffer->GetHBITMAP(Color(255,255,255), &bmp);
    return bmp;
}

LRESULT CWizardDlg::OnBnClickedAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CAboutDlg dlg;
    dlg.DoModal();
    return 0;
}

void CWizardDlg::Exit()
{
    if (!Settings.SaveSettings()) {
        LocalizedMessageBox(TR("Could not save settings file. See error log for details."), APPNAME, MB_ICONERROR);
    }
}

LRESULT CWizardDlg::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true;
    HDROP hDrop = reinterpret_cast<HDROP>(wParam);
    TCHAR szBuffer[256] = _T("\0");
    if(CurPage > 2) return 0;

    int n = DragQueryFile(hDrop,    0xFFFFFFFF, 0, 0);

    CMainDlg* MainDlg = nullptr;
    CStringList Paths;
    
    for (int i=0; i<n; i++)
    {
        
        DragQueryFile(hDrop,    i, szBuffer, sizeof(szBuffer)/sizeof(TCHAR));

        if((IsVideoFile(szBuffer) && n==1) && !Settings.DropVideoFilesToTheList)
        {
            if (enableDragndropOverlay_ && dragndropOverlaySelectedItem_ == CDragndropOverlay::ItemId::kAddToTheList) {
                goto filehost;
            }

            ShowPage(wpVideoGrabberPage, CurPage, (Pages[wpMainPage]) ? wpMainPage : wpUploadSettingsPage);
            CVideoGrabberPage* dlg = getPage<CVideoGrabberPage>(wpVideoGrabberPage);
            dlg->SetFileName(szBuffer);
            
            break;
        }
        else if(CurPage == wpWelcomePage || CurPage == wpMainPage)
        {
            filehost:
            if(WinUtils::FileExists(szBuffer) || WinUtils::IsDirectory(szBuffer))
                                     Paths.Add(szBuffer);            
        }
 
    }
    if(!Paths.IsEmpty())
    {
        CreatePage(wpMainPage);
        FolderAdd.Do(Paths, false, true);
        ShowPage(wpMainPage);
        MainDlg = getPage<CMainDlg>(wpMainPage);
        if (MainDlg) {
//            MainDlg->ThumbsView.LoadThumbnails();
        }
    }
    
    DragFinish(hDrop);
    return 0;  
}

bool CWizardDlg::LoadUploadEngines(const CString &filename, CString &Error)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    enginelist_->setNumOfRetries(settings->FileRetryLimit, settings->ActionRetryLimit);
    bool Result = enginelist_->loadFromFile(filename);
    Error = enginelist_->errorStr();
    return Result;
}

STDMETHODIMP_(ULONG) CWizardDlg::AddRef()
{
    return InterlockedIncrement( &m_lRef );
}

STDMETHODIMP_(ULONG) CWizardDlg::Release()
{
    if ( InterlockedDecrement( &m_lRef ) == 0 )
   {
        //    delete this;
        return 0;
   }
    return m_lRef;
}

STDMETHODIMP CWizardDlg::QueryInterface( REFIID riid, void** ppv )
{
    *ppv = NULL;

    if ( riid == IID_IUnknown || riid == IID_IDropTarget )
        *ppv = this;

    if ( *ppv )
    {
        AddRef();
        return( S_OK );
    }
    return (E_NOINTERFACE);
}

//    IDropTarget methods
STDMETHODIMP CWizardDlg::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
    if (!acceptsDragnDrop()) {
        *pdwEffect = DROPEFFECT_NONE;
        return S_FALSE;
    }

    enableDragndropOverlay_ = false;

  
    FORMATETC formatHdrop = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    FORMATETC formatFileDescriptor = { static_cast<WORD>(RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR)), nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    FORMATETC formatBitmap = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    if (pDataObj->QueryGetData(&formatHdrop) != S_OK &&
        pDataObj->QueryGetData(&formatFileDescriptor) != S_OK &&
        pDataObj->QueryGetData(&formatBitmap) != S_OK
        )
    {
        *pdwEffect = DROPEFFECT_NONE;
        return DRAGDROP_S_CANCEL;

    }

    if (!Settings.DropVideoFilesToTheList && (CurPage == wpMainPage || CurPage == wpWelcomePage)) {
        FORMATETC tc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        if (pDataObj->QueryGetData(&tc) == S_OK)
        {
            STGMEDIUM ddd;
            if (pDataObj->GetData(&tc, &ddd) == S_OK)
            {
                PVOID pDrop = GlobalLock(ddd.hGlobal);
                HDROP hDrop = static_cast<HDROP>(pDrop);
                int n = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);

                if (n == 1) {
                    int len = DragQueryFile(hDrop, 0, nullptr, 0) + 1;
                    CString buffer;

                    DragQueryFile(hDrop, 0, buffer.GetBuffer(len), len);
                    buffer.ReleaseBuffer();
                    if (IsVideoFile(buffer)) {
                        enableDragndropOverlay_ = true;
                    }
                }
                //DragFinish(hDrop);
                GlobalUnlock(pDrop);
                ReleaseStgMedium(&ddd);
            }
        }
    }

    if (!enableDragndropOverlay_) {
        queryDropFiledescriptors(pDataObj, &enableDragndropOverlay_);
    }

    if (enableDragndropOverlay_) {
        dragndropOverlay_.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        dragndropOverlay_.ShowWindow(SW_SHOW);
    }

    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}
    
STDMETHODIMP CWizardDlg::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
    if(!acceptsDragnDrop()) {
        *pdwEffect = DROPEFFECT_NONE;
        return S_FALSE;
    }
    *pdwEffect = DROPEFFECT_COPY;

    if (enableDragndropOverlay_ && !dragndropOverlay_.IsWindowVisible()) {
        dragndropOverlay_.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        dragndropOverlay_.ShowWindow(SW_SHOW);
    }
    POINT clientPt{ pt.x, pt.y };
    dragndropOverlay_.ScreenToClient(&clientPt);
    dragndropOverlay_.dragMove(clientPt.x, clientPt.y);
    return S_OK;
}
    
STDMETHODIMP CWizardDlg::DragLeave( void)
{
    dragndropOverlay_.ShowWindow(SW_HIDE);
    return S_OK;
}

bool CWizardDlg::queryDropFiledescriptors(IDataObject* pDataObj, bool* enableOverlay) {
    FORMATETC tc2 = { static_cast<CLIPFORMAT>(RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR)), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    if (pDataObj->QueryGetData(&tc2) == S_OK)
    {
        STGMEDIUM ddd;

        if (pDataObj->GetData(&tc2, &ddd) == S_OK) {

            PVOID hdrop = (PVOID)GlobalLock(ddd.hGlobal);
            FILEGROUPDESCRIPTOR* fgd = (FILEGROUPDESCRIPTOR*)hdrop;

            if (fgd->cItems == 1 && enableOverlay && IsVideoFile(fgd->fgd[0].cFileName)) {
                *enableOverlay = true;
            }
            ReleaseStgMedium(&ddd);
            return true;
        }
    }
    return false;
}

bool CWizardDlg::HandleDropFiledescriptors(IDataObject *pDataObj)
{
    FORMATETC tc2 = { static_cast<CLIPFORMAT>(RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR)), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    if(pDataObj->QueryGetData(&tc2)==S_OK )
    {
        STGMEDIUM ddd;

        if(pDataObj->GetData(&tc2, &ddd) == S_OK ){

            PVOID hdrop = (PVOID) GlobalLock ( ddd.hGlobal );
            FILEGROUPDESCRIPTOR *fgd = (FILEGROUPDESCRIPTOR*) hdrop;
            
            CStringList Paths;
            for(size_t i=0; i<fgd->cItems; i++)
            {
                FILEDESCRIPTOR& desc = fgd->fgd[i];
                FORMATETC tc3 = { static_cast<CLIPFORMAT>(RegisterClipboardFormat(CFSTR_FILECONTENTS)), 0, DVASPECT_CONTENT, static_cast<LONG>(i), TYMED_HGLOBAL | TYMED_ISTREAM};
                HRESULT res = pDataObj->QueryGetData(&tc3);
                if(res == S_OK)
                {
                    STGMEDIUM ddd2{};
                    ddd2.tymed = TYMED_HGLOBAL   | TYMED_ISTREAM ;
                    if(pDataObj->GetData(&tc3, &ddd2) == S_OK )
                    {
                        CString OutFileName;
                        bool FileWasSaved = false;
                        
                        if(ddd2.tymed == TYMED_HGLOBAL)
                        {
                            LARGE_INTEGER size{};
                            size.LowPart = desc.nFileSizeLow;
                            size.HighPart = desc.nFileSizeHigh;
                            FileWasSaved = SaveFromHGlobal(ddd2.hGlobal, fgd->fgd[i].cFileName, desc.dwFlags & FD_FILESIZE ? &size: nullptr, OutFileName);
                        }

                        if(ddd2.tymed == TYMED_ISTREAM)
                        {    
                            FileWasSaved = SaveFromIStream(ddd2.pstm, fgd->fgd[i].cFileName, OutFileName); 
                        }

                        if(FileWasSaved) // Additing received file to program
                        {
                            if(IsVideoFile(OutFileName) && !(enableDragndropOverlay_ 
                                && dragndropOverlaySelectedItem_ == CDragndropOverlay::ItemId::kAddToTheList))
                            {

                                ShowPage(wpVideoGrabberPage, CurPage, (Pages[2])? 2 : 3);
                                CVideoGrabberPage* dlg = getPage<CVideoGrabberPage>(wpVideoGrabberPage);
                                dlg->SetFileName(OutFileName);
                                ReleaseStgMedium(&ddd2);
                                break;
                            }
                            else if((CurPage==0||CurPage==2))
                            {
                                
                                if(WinUtils::FileExists(OutFileName) || WinUtils::IsDirectory(OutFileName))
                                     Paths.Add(OutFileName);        
                            }
                        }
                        ReleaseStgMedium(&ddd2);
                    }
                } /*else {
                    LOG(WARNING) << _com_error(res).ErrorMessage();
                }*/

               
            }
            GlobalUnlock(hdrop);
            ReleaseStgMedium(&ddd);

            if (!Paths.IsEmpty())
            {
                CreatePage(wpMainPage);
                //QuickUploadMarker = (Settings.QuickUpload && !CmdLine.IsOption(_T("noquick"))) || (CmdLine.IsOption(_T("quick")));
                FolderAdd.Do(Paths, /*CmdLine.IsOption(_T("imagesonly"))*/false, true);
                ShowPage(wpMainPage);
                return true;
            }
        }
    }
    return false;
}

bool CWizardDlg::HandleDropHDROP(IDataObject *pDataObj)
{
    FORMATETC tc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    if(pDataObj->QueryGetData(&tc) == S_OK ) 
    {
        STGMEDIUM ddd;
        if(pDataObj->GetData(&tc, &ddd) == S_OK)
        {
            PVOID hdrop = (PVOID) GlobalLock ( ddd.hGlobal );
            BOOL b;
            OnDropFiles(0,(WPARAM) hdrop, 0 ,b);
            GlobalUnlock ( hdrop );
            return true;
        }
    }
    return false;
}

bool CWizardDlg::HandleDropBitmap(IDataObject *pDataObj)
{
    FORMATETC FtcBitmap;
    FtcBitmap.cfFormat = CF_BITMAP;
    FtcBitmap.ptd = 0;
    FtcBitmap.dwAspect = 1;
    FtcBitmap.lindex = DVASPECT_CONTENT;
    FtcBitmap.tymed =  TYMED_HGLOBAL;

    if(pDataObj->QueryGetData(&FtcBitmap) == S_OK ) 
    {
        STGMEDIUM ddd;
        if(pDataObj->GetData(&FtcBitmap, &ddd) == S_OK)
        {
            PasteBitmap(ddd.hBitmap);
            return true;
        }
    }
    return false;
}

void CWizardDlg::setIsFirstRun(bool isFirstRun) {
    isFirstRun_ = isFirstRun;
}

STDMETHODIMP CWizardDlg::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if(!IsWindowEnabled() || !DragndropEnabled) 
    {
        *pdwEffect = DROPEFFECT_NONE; 
        return S_FALSE;
    }

    if (enableDragndropOverlay_) {
        POINT clientPt{ pt.x, pt.y };
        dragndropOverlay_.ScreenToClient(&clientPt);

        dragndropOverlaySelectedItem_ = dragndropOverlay_.itemAtPos(clientPt.x, clientPt.y);
    }

    dragndropOverlay_.ShowWindow(SW_HIDE);
    bool hasBitmap = false;
    FORMATETC FtcBitmap;
    FtcBitmap.cfFormat = CF_BITMAP;
    FtcBitmap.ptd = 0;
    FtcBitmap.dwAspect = 1;
    FtcBitmap.lindex = DVASPECT_CONTENT;
    FtcBitmap.tymed = TYMED_HGLOBAL;

    if (pDataObj->QueryGetData(&FtcBitmap) == S_OK)  {
        hasBitmap = true;
    }else {
        FtcBitmap.cfFormat = CF_DIB;
        if (pDataObj->QueryGetData(&FtcBitmap) == S_OK) {
            hasBitmap = true;
        }
    }

    *pdwEffect = DROPEFFECT_COPY;
    if (!hasBitmap && HandleDropHDROP(pDataObj))
        return S_OK;

    if (HandleDropFiledescriptors(pDataObj))
        return S_OK;

    if(HandleDropBitmap(pDataObj))
        return S_OK;

    *pdwEffect = DROPEFFECT_NONE;
    return S_OK;
}

LRESULT CWizardDlg::OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    funcFromClipboard(false);
    return 0;
}

LRESULT CWizardDlg::OnDocumentation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SHELLEXECUTEINFO ShInfo;
    CString fileName = WinUtils::GetAppFolder() + "Docs\\index.html";
    CString directory = WinUtils::GetAppFolder() + "Docs\\";
    ZeroMemory(&ShInfo, sizeof(SHELLEXECUTEINFO));
    ShInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShInfo.nShow = SW_SHOWNORMAL;
    ShInfo.fMask = SEE_MASK_DEFAULT;
    ShInfo.hwnd = m_hWnd;
    ShInfo.lpVerb = TEXT("open");
    ShInfo.lpFile = fileName;
    ShInfo.lpDirectory = directory;

    if (ShellExecuteEx(&ShInfo)==FALSE) {
        DWORD error = GetLastError();
        if (error != ERROR_CANCELLED) {
            LOG(ERROR) << "ShellExecute failed. " << WinUtils::FormatWindowsErrorMessage(error);
        }
        return 0;
    }
    return 0;
}

LRESULT CWizardDlg::OnShowLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ServiceLocator::instance()->logWindow()->Show();
    return 0;
}

LRESULT CWizardDlg::OnOpenScreenshotFolderClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    funcOpenScreenshotFolder();
    return 0;
}

LRESULT CWizardDlg::OnEnableDropTarget(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    // This message is sent from ThumbsView when dragging of an item starts.
    // We need this to avoid dragging files from wizard to wizard itself.
    // Disable temporary drag-n-drop to wizard's window
    DragndropEnabled = !!wParam;
    return 0;
}

void CWizardDlg::PasteBitmap(HBITMAP Bmp)
{
    if (CurPage != wpWelcomePage && CurPage != wpMainPage && CurPage != -1) {
        return;
    }
   
    CString fileNameBuffer;
    Bitmap bm(Bmp, nullptr);
    if (bm.GetLastStatus() == Ok) {
        try {
            if (ImageUtils::MySaveImage(&bm, _T("clipboard"), fileNameBuffer, ImageUtils::sifPNG, 100)) {
                CreatePage(wpMainPage);
                CMainDlg* MainDlg = getPage<CMainDlg>(wpMainPage);
                MainDlg->AddToFileList(fileNameBuffer, L"", true, nullptr, true);
                ShowPage(wpMainPage);
            }
        } catch (const std::exception& ex) {
            LOG(ERROR) << "Failed to save image: " << ex.what();
        }
    }
}

void CWizardDlg::AddFolder(LPCTSTR szFolder, bool SubDirs )
{
   CString Folder = szFolder;
    if(Folder[Folder.GetLength()-1]==_T('\\'))
        Folder.Delete(Folder.GetLength()-1);

    CStringList Paths;
    Paths.Add(Folder );
    FolderAdd.Do(Paths, true, SubDirs);
}

bool CWizardDlg::AddImage(const CString &FileName, const CString &VirtualFileName, bool Show)
{
    CreatePage(wpMainPage);
    CMainDlg* MainDlg = getPage<CMainDlg>(wpMainPage);
    if (!MainDlg) {
        return false;
    }
    MainDlg->AddToFileList(FileName, VirtualFileName);
    if(Show){
        ShowPage(wpMainPage);
    }
    return true;
}

bool CWizardDlg::AddImageAsync(const CString &FileName, const CString &VirtualFileName, bool show) {
    std::lock_guard<std::mutex> lk(newImagesMutex_);
    newImages_.push_back({ FileName, VirtualFileName, show });
    return true;
}

LRESULT CWizardDlg::OnAddImages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{    
    auto* ais = reinterpret_cast<AddImageStruct*>(wParam);
    if(!ais) return 0;
    return  AddImage(ais->RealFileName, ais->VirtualFileName, ais->show);
}
    
LRESULT CWizardDlg::OnWmShowPage(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    int PageIndex = wParam;
    ShowPage(static_cast<WizardPageId>(PageIndex));
    return 0;
}

LRESULT CWizardDlg::OnTaskDispatcherMsg(UINT, WPARAM wParam, LPARAM, BOOL&) {
    if (wParam) {
        auto msg = reinterpret_cast<TaskDispatcherMessageStruct*>(wParam);
        msg->callback(); 
    } else {
        // Async task
        std::vector<TaskRunnerTask> tasks;
        {
            std::lock_guard<std::mutex> lk(scheduledTasksMutex_);
            std::swap(tasks, scheduledTasks_);
        }
        for(const auto& task: tasks) {
            try {
                task();
            } catch (std::exception& ex) {
                LOG(ERROR) << ex.what();
            }
        }
    }
    return 0;
}

bool CWizardDlg::funcAddImages(bool AnyFiles)
{
    int nCount = 0;

    CComPtr<IFileOpenDialog> pDlg;

    CString filterBuffer = CString(TR("Images")) + _T(" (jpeg, bmp, png, gif ...)");
    CString imageFilter = IuCommonFunctions::PrepareFileDialogImageFilter();
    COMDLG_FILTERSPEC aFileTypes[] = {
        {filterBuffer, imageFilter},
        {TR("Any file"), _T("*.*")}
    };
    DWORD dwFlags = 0;

    // Create the file-save dialog COM object.
    HRESULT hr = pDlg.CoCreateInstance(CLSID_FileOpenDialog);

    if (FAILED(hr))
        return false;

    pDlg->SetFileTypes(_countof(aFileTypes), aFileTypes);

    CComPtr<IShellItem> psiFolder;
    LPWSTR wszPath = NULL;

    if (Settings.ImagesFolder.IsEmpty()) {

        hr = SHGetKnownFolderPath(FOLDERID_Pictures, KF_FLAG_CREATE,
                                  NULL, &wszPath);

        if (SUCCEEDED(hr)) {
            hr = SHCreateItemFromParsingName(wszPath, NULL, IID_PPV_ARGS(&psiFolder));

            if (SUCCEEDED(hr))
                pDlg->SetDefaultFolder(psiFolder);

            CoTaskMemFree(wszPath);
        }

    } else {

        hr = SHCreateItemFromParsingName(Settings.ImagesFolder, NULL, IID_PPV_ARGS(&psiFolder));

        if (SUCCEEDED(hr)) {
            pDlg->SetDefaultFolder(psiFolder);
        }
    }

    //pDlg->SetTitle(L"A File-Save Dialog");
    //pDlg->SetOkButtonLabel(L"D&o It!");
    //pDlg->SetFileName(L"mystuff.txt");
    //pDlg->SetDefaultExtension(L"txt");

    pDlg->GetOptions(&dwFlags);
    pDlg->SetOptions(dwFlags | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
    // Create the file-open dialog COM object.
    hr = pDlg->Show(m_hWnd);

    // If the user chose any files, loop thru the array of files.
    if (SUCCEEDED(hr)) {
        CComPtr<IShellItemArray> pItemArray;

        hr = pDlg->GetResults(&pItemArray);

        if (SUCCEEDED(hr)) {
            DWORD cSelItems;

            // Get the number of selected files.
            hr = pItemArray->GetCount(&cSelItems);

            if (SUCCEEDED(hr)) {
                if (!cSelItems) {
                    return 0;
                }
                for (DWORD j = 0; j < cSelItems; j++) {
                    CComPtr<IShellItem> pItem;

                    // Get an IShellItem interface on the next file.
                    hr = pItemArray->GetItemAt(j, &pItem);

                    if (SUCCEEDED(hr)) {
                        LPOLESTR pwsz = NULL;

                        // Get its file system path.
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);

                        if (SUCCEEDED(hr)) {
                            CreatePage(wpMainPage);
                            if (getPage<CMainDlg>(wpMainPage)->AddToFileList(pwsz)) {
                                nCount++;
                            }
                            CoTaskMemFree(pwsz);
                        }
                    }
                }
            }
        } else {
            return 0;
        }
        CComPtr<IShellItem> pFolderItem;
        hr = pDlg->GetFolder(&pFolderItem);
        if (SUCCEEDED(hr)) {
            LPOLESTR pwsz = NULL;

            // Get its file system path.
            hr = pFolderItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);

            if (SUCCEEDED(hr)) {
                Settings.ImagesFolder = pwsz;
                CoTaskMemFree(pwsz);
            }
        }
    } else {
        return false;
    }
    
    
    if (nCount) {
        ShowPage(wpMainPage, wpWelcomePage, wpUploadSettingsPage);
        CMainDlg* mainDlg = getPage<CMainDlg>(wpMainPage);
        mainDlg->UpdateStatusLabel();

        if (CurPage == wpMainPage) {
//            mainDlg->ThumbsView.LoadThumbnails();
        }
        ShowWindow(SW_SHOW);
        m_bShowWindow = true;
    }
    return true;
}

bool CWizardDlg::executeFunc(CString funcBody, bool fromCmdLine)
{
    bool LaunchCopy = false;

    if (CurPage == wpUploadPage || CurPage == wpVideoGrabberPage) {
        LaunchCopy = true;
    }
    if (CurPage == 3) ShowPage(wpMainPage);

    if (!IsWindowEnabled()) {
        LaunchCopy = true;
    }

    CString funcName = WinUtils::StringSection(funcBody, _T(','), 0);
    CString funcParam1 = WinUtils::StringSection(funcBody, _T(','), 1);

    if (!funcParam1.IsEmpty()) {
        m_bScreenshotFromTray = _ttoi(funcParam1);
    }
    if (LaunchCopy) {
        if (Settings.TrayIconSettings.DontLaunchCopy) {
            if (IsWindowVisible() && IsWindowEnabled())
                SetForegroundWindow(m_hWnd);
            else if (!IsWindowEnabled()) SetActiveWindow();
            FlashWindow(true);
        } else
            IULaunchCopy(_T("/func=") + funcBody, CAtlArray<CString>());
        return false;
    }
    if (funcName == _T("addimages"))
        return funcAddImages();
    /*else if(funcName == _T("addfiles"))
        return funcAddImages(true);*/
    if (funcName == _T("addfiles"))
        return funcAddFiles();
    else if (funcName == _T("importvideo"))
        return funcImportVideo();
    else if (funcName == _T("screenshotdlg"))
        return funcScreenshotDlg();
    else if (funcName == _T("regionscreenshot"))
        return funcRegionScreenshot();
    else if (funcName == _T("regionscreenshot_dontshow"))
        return funcRegionScreenshot(false);
    else if (funcName == _T("fullscreenshot"))
        return funcFullScreenshot();
    else if (funcName == _T("windowhandlescreenshot"))
        return funcWindowHandleScreenshot();
    else if (funcName == _T("freeformscreenshot"))
        return funcFreeformScreenshot();
    else if (funcName == _T("lastregionscreenshot"))
        return funcLastRegionScreenshot();
    else if (funcName == _T("downloadimages"))
        return funcDownloadImages();
    else if (funcName == _T("windowscreenshot"))
        return funcWindowScreenshot();
    else if (funcName == _T("windowscreenshot_delayed"))
        return funcWindowScreenshot(true);
    else if (funcName == _T("addfolder"))
        return funcAddFolder();
    else if (funcName == _T("fromclipboard") || funcName == _T("paste"))
        return funcFromClipboard(fromCmdLine);
    else if (funcName == _T("settings"))
        return funcSettings();
    else if (funcName == _T("reuploadimages"))
        return funcReuploadImages();
    else if (funcName == _T("shortenurl"))
        return funcShortenUrl();
#ifdef IU_ENABLE_MEDIAINFO
    else if (funcName == _T("mediainfo"))
        return funcMediaInfo();
#endif
    else if (funcName == _T("open_screenshot_folder"))
        return funcOpenScreenshotFolder();
    else if (funcName == _T("exit"))
        return funcExit();
    return false;
}

bool CWizardDlg::importVideoFile(const CString& fileName, int prevPage) {
    CreatePage(wpVideoGrabberPage);
    LastVideoFile = fileName;
    getPage<CVideoGrabberPage>(wpVideoGrabberPage)->SetFileName(fileName);
    ShowPage(wpVideoGrabberPage, prevPage, Pages[wpMainPage] ? wpMainPage : wpUploadSettingsPage);
    return true;
}

bool CWizardDlg::funcImportVideo()
{
    IMyFileDialog::FileFilterArray filters = {
        {CString(TR("Video files")) + _T(" (avi, mpg, vob, wmv ...)"), PrepareVideoDialogFilters(),},
        {TR("All files"), _T("*.*")}
    };

    auto dlg = MyFileDialogFactory::createFileDialog(m_hWnd, Settings.VideoFolder, TR("Choose video file"), filters, false);
    if (dlg->DoModal(m_hWnd) != IDOK) {
        return false;
    }
    CString fileName = dlg->getFile();
    Settings.VideoFolder = dlg->getFolderPath();

    if (!WinUtils::FileExists(fileName)) {
        return false;
    }
	importVideoFile(fileName);
    ShowWindow(SW_SHOW);
    m_bShowWindow = true;
    return true;
}

bool CWizardDlg::funcScreenshotDlg()
{
    CScreenshotDlg dlg;
    if(dlg.DoModal(m_hWnd) != IDOK) return false;
    
    CommonScreenshot(dlg.captureMode()); 
    m_bShowWindow = true;
    return true;
}

bool CWizardDlg::funcRegionScreenshot(bool ShowAfter)
{
    m_bShowAfter = ShowAfter;
    CommonScreenshot(ScreenCapture::cmRectangles);
    return true;
}

void CWizardDlg::OnScreenshotFinished(int Result)
{
    EnableWindow();

    if(m_bShowAfter || (Result && !floatWnd_->m_hWnd))
    {
        m_bShowWindow = true;
        ShowWindow(SW_SHOWNORMAL);
        SetForegroundWindow(m_hWnd);
    }

    if(Result )
    {
        CMainDlg* mainDlg = getPage<CMainDlg>(wpMainPage);
        if (mainDlg)
        {
            mainDlg->ThumbsView.SetFocus();
            mainDlg->ThumbsView.SelectLastItem();
        }
    }
    else if (m_bHandleCmdLineFunc)
    {
        
        PostQuitMessage(0);
    }
    m_bHandleCmdLineFunc = false;

}

void CWizardDlg::OnScreenshotSaving(LPTSTR FileName, Bitmap* Bm)
{
    if(FileName && lstrlen(FileName))
    {
        CreatePage(wpMainPage);
        CMainDlg* mainDlg = getPage<CMainDlg>(wpMainPage);
        if (mainDlg) {
            mainDlg->AddToFileList(FileName);
            if (CurPage == wpMainPage) {
//                mainDlg->ThumbsView.LoadThumbnails();
            }
            ShowPage(wpMainPage, wpWelcomePage, wpUploadSettingsPage);
        }
    }
}

bool CWizardDlg::funcFullScreenshot()
{
    CommonScreenshot(ScreenCapture::cmFullScreen);
    return true;
}

bool CWizardDlg::funcWindowScreenshot(bool Delay)
{
    CommonScreenshot(ScreenCapture::cmActiveWindow);
    return true;
}

bool CWizardDlg::funcLastRegionScreenshot() {
    return CommonScreenshot(ScreenCapture::cmLastRegion);
}

bool CWizardDlg::funcAddFolder() {

    constexpr DWORD kCheckboxId = 2000;
    CNewStyleFolderDialog dlg(m_hWnd, CString(), TR("Choose folder"), true);
    dlg.AddCheckbox(kCheckboxId, TR("Including subdirectories"), Settings.ParseSubDirs);
    if (dlg.DoModal(m_hWnd) == IDOK) {
        Settings.ParseSubDirs = dlg.IsCheckboxChecked(kCheckboxId);
        m_bShowWindow = true;
        AddFolder(dlg.GetFolderPath(), Settings.ParseSubDirs);
    }
    return false;

}
LRESULT CWizardDlg::OnEnable(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if(!floatWnd_->m_hWnd)
      TRC(IDCANCEL, "Exit");
    else 
        TRC(IDCANCEL, "Hide");

    return 0;
}

void CWizardDlg::CloseWizard()
{
    if(CurPage!= wpWelcomePage && CurPage!= wpUploadPage && Settings.ConfirmOnExit)
        if(LocalizedMessageBox(TR("Are you sure to quit?"),APPNAME, MB_YESNO|MB_ICONQUESTION) != IDYES) return ;
    
    CloseDialog(0);
}

bool CWizardDlg::RegisterLocalHotkeys() {
    m_hotkeys = Settings.Hotkeys;
    int n = m_hotkeys.size();
    constexpr auto PREDEFINED_ACCEL_COUNT = 2;
    auto accels = std::make_unique<ACCEL[]>(n + PREDEFINED_ACCEL_COUNT);
    accels[0] = {FVIRTKEY, VK_F1, IDC_DOCUMENTATION};
    accels[1] = {FVIRTKEY | FCONTROL | FSHIFT, 'L', IDC_SHOWLOG};

    int j = PREDEFINED_ACCEL_COUNT;
    for (int i = 0; i < n; i++) {
        if (!m_hotkeys[i].localKey.keyCode) {
            continue;
        }
        accels[j] = m_hotkeys[i].localKey.toAccel();
        accels[j].cmd = static_cast<WORD>(10000 + i);
        j++;
    }

    localHotkeys_.DestroyObject();
    localHotkeys_.CreateAcceleratorTable(accels.get(), j);
    return true;
}

LRESULT CWizardDlg::OnLocalHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(CurPage== wpUploadSettingsPage) ShowPage(wpMainPage);
    if(!IsWindowEnabled() || (CurPage!= wpWelcomePage && CurPage!= wpMainPage))
        return 0;
    int hotkeyId = wID-ID_HOTKEY_BASE;
    executeFunc(m_hotkeys[hotkeyId].func);
    return 0;
}

bool CWizardDlg::UnRegisterLocalHotkeys()
{
    localHotkeys_.DestroyObject();

    m_hotkeys.clear();
    return true;
}

bool CWizardDlg::funcSettings()
{
    CSettingsDlg dlg(CSettingsDlg::spGeneral, uploadEngineManager_);
    //dlg.DoModal(m_hWnd);
    if(!IsWindowVisible())
        dlg.DoModal(0);
    else
        dlg.DoModal(m_hWnd);
    sessionImageServer_ = Settings.imageServer;
    sessionFileServer_ = Settings.fileServer;
    return true;
}

bool CWizardDlg::funcDownloadImages()
{
    CImageDownloaderDlg dlg(this,CString());
    dlg.EmulateModal(m_hWnd);
    return true;
}
#ifdef IU_ENABLE_MEDIAINFO
bool CWizardDlg::funcMediaInfo()
{
    IMyFileDialog::FileFilterArray filters = {
        { CString(TR("Video files")) + _T(" (avi, mpg, vob, wmv ...)"), PrepareVideoDialogFilters(), },
        { CString(TR("Audio files")) + _T(" (mp3, wma, wav ...)"), _T("*.mp3;*.wav;*.wma;*.mid;*.asx") },
        { TR("All files"), _T("*.*") }
    };

    auto fileDlg = MyFileDialogFactory::createFileDialog(m_hWnd, Settings.VideoFolder, TR("Choose media file"), filters, false);
    
    if (fileDlg->DoModal(m_hWnd) != IDOK) {
        return false;
    }

    CString fileName = fileDlg->getFile();

    if (fileName.IsEmpty() || !WinUtils::FileExists(fileName)) {
        return false;
    }
    TCHAR Buffer[512];
    WinUtils::ExtractFilePath(fileName, Buffer, ARRAY_SIZE(Buffer));
    Settings.VideoFolder = Buffer;
    CMediaInfoDlg dlg;
    LastVideoFile = fileName;
    dlg.ShowInfo(m_hWnd, fileName);
    return true;
}
#endif
bool CWizardDlg::funcAddFiles()
{
    IMyFileDialog::FileFilterArray filters = { 
        { CString(TR("Images")) + _T(" (jpeg, bmp, png, gif ...)"), IuCommonFunctions::PrepareFileDialogImageFilter() },
        { CString(TR("Video files")) + _T(" (avi, mpg, vob, wmv ...)"), PrepareVideoDialogFilters(), },
        { TR("Any file"), _T("*.*") }
    };
    auto fileDialog(MyFileDialogFactory::createFileDialog(m_hWnd, Settings.ImagesFolder, TR("Choose files"), filters, true));
    
    fileDialog->setFileTypeIndex(3);

    if (fileDialog->DoModal(m_hWnd) != IDOK) {
        return 0;
    }
    std::vector<CString> files;
    fileDialog->getFiles(files);

    if (!files.empty()) {
        CreatePage(wpMainPage);
        CMainDlg* mainDlg = getPage<CMainDlg>(wpMainPage);
        int nCount = 0;
        for (const auto& fileName : files) {
            if (mainDlg->AddToFileList(fileName)) {
                nCount++;
            }
        }

        Settings.ImagesFolder = fileDialog->getFolderPath();
        if (nCount) {
            ShowPage(wpMainPage, wpWelcomePage, wpUploadSettingsPage);
        }
        mainDlg->UpdateStatusLabel();

        ShowWindow(SW_SHOW);
        m_bShowWindow = true;
    }
    
    return true;
}

LRESULT CWizardDlg::OnWmMyExit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == kWmMyExitParam )
    {
        CloseDialog(0);
    }
    return 0;
}

bool CWizardDlg::CanShowWindow()
{
    return (CurPage == wpMainPage || CurPage == wpWelcomePage) && IsWindowVisible() && IsWindowEnabled();
}

bool CWizardDlg::hasLastScreenshotRegion() const {
    return !!lastScreenshotRegion_;
}

void CWizardDlg::setLastScreenshotRegion(std::shared_ptr<ScreenCapture::CScreenshotRegion> region, HMONITOR monitor) {
    lastScreenshotRegion_ = region;
    lastScreenshotMonitor_ = monitor;
    bool available = !!lastScreenshotRegion_;
    for(const auto& cb: lastRegionAvailabilityChangeCallbacks_) {
        if (cb) {
            cb(available);
        }
    }
}

void CWizardDlg::addLastRegionAvailabilityChangeCallback(std::function<void(bool)> cb) {
    lastRegionAvailabilityChangeCallbacks_.push_back(cb);
}

bool CWizardDlg::getQuickUploadMarker() const {
    return QuickUploadMarker;
}

void CWizardDlg::setQuickUploadMarker(bool val) {
    QuickUploadMarker = val;
}

CString CWizardDlg::getLastVideoFile() const {
    return LastVideoFile;
}

void CWizardDlg::setLastVideoFile(CString fileName) {
    LastVideoFile = fileName;
}

bool CWizardDlg::isShowWindowSet() const {
    return m_bShowWindow;
}

void CWizardDlg::UpdateAvailabilityChanged(bool Available)
{
}
    
LRESULT CWizardDlg::OnUpdateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    CreateUpdateDlg();
    updateDlg->ShowModal(m_hWnd, true);
    return 0;
}

void CWizardDlg::CreateUpdateDlg()
{
    if(!updateDlg)
    {
        updateDlg.reset(new CUpdateDlg());
        updateDlg->setUpdateCallback(this);
    }
}

bool CWizardDlg::CommonScreenshot(ScreenCapture::CaptureMode mode)
{
    using namespace ScreenCapture;
    // TODO: this method is too complicated and long. 
    bool needToShow = IsWindowVisible()!=FALSE;
    if(m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_UPLOAD   && !floatWnd_->m_hWnd)
    {
        m_bScreenshotFromTray = false;
        //return false;
    }
    bool CanceledByUser = false;
    bool Result = false;
    if(needToShow)
        ShowWindow(SW_HIDE);
    EnableWindow(false);
    CScreenCaptureEngine engine;
    
    CString buf; // file name buffer
    std::shared_ptr<Gdiplus::Bitmap> result;
    CWindowHandlesRegion::WindowCapturingFlags wcfFlags;
    wcfFlags.AddShadow = Settings.ScreenshotSettings.AddShadow;
    wcfFlags.RemoveBackground =     Settings.ScreenshotSettings.RemoveBackground;
    wcfFlags.RemoveCorners = Settings.ScreenshotSettings.RemoveCorners;
    int WindowHidingDelay = (needToShow||m_bScreenshotFromTray==2)? Settings.ScreenshotSettings.WindowHidingDelay: 0;
    
    engine.setDelay(WindowHidingDelay);
    MonitorMode monitorMode = static_cast<MonitorMode>(Settings.ScreenshotSettings.MonitorMode);
    HMONITOR monitor = nullptr;
    if (mode == cmLastRegion) {
        monitor = lastScreenshotMonitor_;
    } else if (monitorMode == MonitorMode::kCurrentMonitor) {
        monitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    } else if (monitorMode >= 0) {
        MonitorEnumerator enumerator;
        if (!enumerator.enumDisplayMonitors(nullptr, nullptr)) {
            return false;
        }
        MonitorEnumerator::MonitorInfo* monitorInfo = enumerator.getByIndex(monitorMode);
        if (!monitorInfo) {
            LOG(WARNING) << "Unable to find monitor #" << monitorMode;
        } else {
            monitor = monitorInfo->monitor;
        }
    }
    engine.setMonitorMode(monitorMode, monitor);
    if(mode == cmFullScreen)
    {
        engine.setDelay(WindowHidingDelay + Settings.ScreenshotSettings.Delay*1000);
        engine.captureScreen();
        result = engine.capturedBitmap();
    }
    else if (mode == cmActiveWindow)
    {
        int Delay = Settings.ScreenshotSettings.Delay;
        if(Delay <1) Delay = 1;
        engine.setDelay(WindowHidingDelay + Delay*1000);
        CActiveWindowRegion winRegion;
        winRegion.setWindowCapturingFlags(wcfFlags);
        winRegion.SetWindowHidingDelay(Settings.ScreenshotSettings.WindowHidingDelay);
        engine.captureRegion(&winRegion);
        result = engine.capturedBitmap();
    } else if (mode == cmLastRegion) {
        if (!lastScreenshotRegion_) {
            LOG(ERROR) << "Last region is empty!";
        }
        else {
            engine.setDelay(WindowHidingDelay + Settings.ScreenshotSettings.Delay * 1000);
            engine.captureRegion(lastScreenshotRegion_.get());
            result = engine.capturedBitmap();
        }
    }
    else if(engine.captureScreen())
    {
        if ( mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod ) {
            result = engine.capturedBitmap();
        } else {
            // Show old window for selecting screen region
            RegionSelect.Parent = m_hWnd;
            SelectionMode selMode = SelectionMode::smRectangles;
            if(mode == cmFreeform)
                selMode = SelectionMode::smFreeform;
            if(mode == cmRectangles)
                selMode = SelectionMode::smRectangles;
            if(mode == cmWindowHandles)
                selMode = SelectionMode::smWindowHandles;

            RegionSelect.m_SelectionMode = selMode;
            std::shared_ptr<Gdiplus::Bitmap> res(engine.capturedBitmap());
            if(res)
            {
                HBITMAP gdiBitmap=0;
                res->GetHBITMAP(Color(255,255,255), &gdiBitmap);
                if(RegionSelect.Execute(gdiBitmap, res->GetWidth(), res->GetHeight(), monitor))
                {
                    if(RegionSelect.wasImageEdited() || (mode!=cmWindowHandles /*|| !Settings.ScreenshotSettings.ShowForeground*/) )
                    engine.setSource(gdiBitmap);
                    
                    else{
                        engine.setSource(0);
                    }
                    
                    engine.setDelay(0);
                    auto rgn = RegionSelect.region();
                    if(rgn)
                    {
                        auto* whr =  dynamic_cast<CWindowHandlesRegion*>(rgn.get());
                        if(whr)
                        {
                            whr->SetWindowHidingDelay(static_cast<int>(Settings.ScreenshotSettings.WindowHidingDelay * 1.2));
                            whr->setWindowCapturingFlags(wcfFlags);
                        }
                        engine.captureRegion(rgn.get());    
                        result = engine.capturedBitmap();
                        DeleteObject(gdiBitmap);
                    }
                    setLastScreenshotRegion(rgn, monitor);
                }
                else CanceledByUser = true;
            }
        }
    }
    using namespace ImageEditor;
    ImageEditorWindow::DialogResult dialogResult = ImageEditorWindow::drCancel;
    CString suggestingFileName;
    if ( result ) {
        suggestingFileName = IuCommonFunctions::GenerateFileName(Settings.ScreenshotSettings.FilenameTemplate, screenshotIndex,CPoint(result->GetWidth(),result->GetHeight()));
    }

    if(result && ( (mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod) || (!m_bScreenshotFromTray && Settings.ScreenshotSettings.OpenInEditor ) || (m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_OPENINEDITOR) ))
    {
        ImageEditorConfigurationProvider configProvider;
        ImageEditor::ImageEditorWindow imageEditor(result, mode == cmFreeform ||   mode == cmActiveWindow, &configProvider);
        imageEditor.setInitialDrawingTool((mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod) ? ImageEditor::DrawingToolType::dtCrop : ImageEditor::DrawingToolType::dtBrush);
        imageEditor.showUploadButton(m_bScreenshotFromTray!=0);
        if ( m_bScreenshotFromTray ) {
            imageEditor.setServerName(Utf8ToWCstring(Settings.quickScreenshotServer.getByIndex(0).serverName()));
        }
        imageEditor.setSuggestedFileName(suggestingFileName);
        dialogResult = imageEditor.DoModal(m_hWnd, monitor, ((mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod) || mode == cmFullScreen) ? ImageEditorWindow::wdmFullscreen : ImageEditorWindow::wdmAuto);
        if (dialogResult != ImageEditorWindow::drCancel && mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod) {
            Gdiplus::Rect lastCrop = imageEditor.lastAppliedCrop();
            if (!lastCrop.IsEmptyArea()) {
                setLastScreenshotRegion(std::make_shared<CRectRegion>(lastCrop.X, lastCrop.Y, lastCrop.Width, lastCrop.Height), monitor);
            }
        }
        if ( dialogResult == ImageEditorWindow::drAddToWizard || dialogResult == ImageEditorWindow::drUpload ) {
            result = imageEditor.getResultingBitmap();
        } else {
            if (dialogResult == ImageEditorWindow::drCopiedToClipboard && floatWnd_->m_hWnd) {
                floatWnd_->ShowScreenshotCopiedToClipboardMessage();
            }
            CanceledByUser = true;
        }
    } 

    if(!CanceledByUser)
    {
        if(result)
        {
            Result = true;
            bool CopyToClipboard = false;
            if((m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_CLIPBOARD) || Settings.ScreenshotSettings.CopyToClipboard)
            {

                CopyToClipboard = true;
            }
            ImageUtils::SaveImageFormat savingFormat = static_cast<ImageUtils::SaveImageFormat>(Settings.ScreenshotSettings.Format);
            if (savingFormat == ImageUtils::sifJPEG) {
                ImageUtils::Gdip_RemoveAlpha(*result, Color(255, 255, 255, 255));
            }

            CString saveFolder = IuCommonFunctions::GenerateFileName(Settings.ScreenshotSettings.Folder, screenshotIndex,CPoint(result->GetWidth(),result->GetHeight()));
            try {
                ImageUtils::MySaveImage(result.get(),suggestingFileName,buf,savingFormat, Settings.ScreenshotSettings.Quality,(Settings.ScreenshotSettings.Folder.IsEmpty())?0:(LPCTSTR)saveFolder);
            } catch (const std::exception& ex) {
                LOG(ERROR) << ex.what();
            }
                screenshotIndex++;
            if ( CopyToClipboard )
            {
                CClientDC dc(m_hWnd);
                if (ImageUtils::CopyBitmapToClipboard(m_hWnd, dc, result.get()) ) {
                    if (m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_CLIPBOARD 
                        && dialogResult == ImageEditorWindow::drCancel) {
                        floatWnd_->ShowScreenshotCopiedToClipboardMessage();
                        Result = false;
                    }
                }
            }
            if(!m_bScreenshotFromTray || dialogResult == ImageEditorWindow::drAddToWizard || (Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_ADDTOWIZARD || Settings.TrayIconSettings.TrayScreenshotAction== TRAY_SCREENSHOT_SHOWWIZARD))
            {
                CreatePage(wpMainPage);
                CMainDlg* mainDlg = getPage<CMainDlg>(wpMainPage);
                mainDlg->AddToFileList(buf);
                mainDlg->ThumbsView.EnsureVisible(mainDlg->ThumbsView.GetItemCount() - 1, true);
//                mainDlg->ThumbsView.LoadThumbnails();
                mainDlg->ThumbsView.SetFocus();
                ShowPage(wpMainPage, wpWelcomePage, wpUploadSettingsPage);
            }
            else if(m_bScreenshotFromTray && (Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_UPLOAD || dialogResult == ImageEditorWindow::drUpload))
            {
                Result = false;
                CString displayFileName = WinUtils::myExtractFileName(buf);
                floatWnd_->UploadScreenshot(buf, displayFileName);
            }
        }
        else
        {
            LocalizedMessageBox(TR("Unable to make screenshot!"));
        }
    }

    m_bShowAfter  = false;
    if(Result || needToShow )
    {
        if(needToShow || (!m_bScreenshotFromTray ||Settings.TrayIconSettings.TrayScreenshotAction!= TRAY_SCREENSHOT_ADDTOWIZARD))
        {
            m_bShowAfter = true;
        }
    } 
    else m_bShowAfter = false;
    m_bScreenshotFromTray = false;
    OnScreenshotFinished(Result);

    return Result;
}

bool CWizardDlg::funcWindowHandleScreenshot()
{
    return CommonScreenshot(ScreenCapture::cmWindowHandles);
}

bool CWizardDlg::funcFreeformScreenshot()
{
    return CommonScreenshot(ScreenCapture::cmFreeform);
}

bool CWizardDlg::IsClipboardDataAvailable()
{
    bool IsClipboard = IsClipboardFormatAvailable(CF_BITMAP)!=FALSE;

    if(!IsClipboard)
    {
        if(IsClipboardFormatAvailable(CF_UNICODETEXT)) 
        {
            CString text;
            WinUtils::GetClipboardText(text);
            if (text.Left(5) == _T("data:")) {
                IsClipboard = true;
            }
            else if(CImageDownloaderDlg::LinksAvailableInText(text))
            {
                IsClipboard = true;
            }
        }
    }
    return IsClipboard;
}

bool CWizardDlg::funcReuploadImages() {
    CImageReuploaderDlg dlg(this, ServiceLocator::instance()->myEngineList(), uploadManager_, uploadEngineManager_, CString());
    dlg.DoModal(m_hWnd);
    return false;
}

bool CWizardDlg::funcShortenUrl() {
    CShortenUrlDlg dlg(this, uploadManager_, uploadEngineManager_, CString());
    dlg.DoModal();
    return false;
}

bool CWizardDlg::funcOpenScreenshotFolder() {
    CString screenshotFolder = Settings.ScreenshotSettings.Folder;

    if (screenshotFolder.IsEmpty()) {
        screenshotFolder = AppParams::instance()->tempDirectoryW();
    }

    if (!screenshotFolder.IsEmpty()) {
        DesktopUtils::ShellOpenUrl(W2U(screenshotFolder));
    }

    return false;
}

bool CWizardDlg::funcFromClipboard(bool fromCmdLine) {
     if (pasteFromClipboard()) {
        ShowWindow(SW_SHOW);
        if (fromCmdLine && CmdLine.IsOption(_T("quick"))) {
            ShowPage(wpUploadPage, wpMainPage);
        } else {
            ShowPage(wpMainPage, wpWelcomePage, wpUploadSettingsPage);
        }
        m_bShowWindow = true;
        return true;
    }
    return false;
}

bool CWizardDlg::funcExit() {
    CloseWizard();
    return true;
}

LRESULT CWizardDlg::OnBnClickedHelpbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    RECT rc;
    ::GetWindowRect(hWndCtl, &rc );
    POINT menuOrigin = {rc.left,rc.bottom};

    CMenu popupMenu;
    popupMenu.CreatePopupMenu();
    popupMenu.AppendMenu(MF_STRING, IDC_ABOUT, TR("About..."));
    popupMenu.AppendMenu(MF_STRING, IDC_DOCUMENTATION, TR("Documentation") + CString(_T("\tF1")));
    popupMenu.AppendMenu(MF_STRING, IDC_UPDATESLABEL, TR("Check for Updates"));
    popupMenu.AppendMenu(MF_SEPARATOR, 99998,_T(""));
    popupMenu.AppendMenu(MF_STRING, IDM_OPENSCREENSHOTS_FOLDER, TR("Open screenshots folder"));
    popupMenu.AppendMenu(MF_SEPARATOR, 99999, _T(""));
#ifndef NDEBUG
    popupMenu.AppendMenu(MF_STRING, IDM_OPENSERVERSCHECKER, _T("Servers Checker"));
#endif
    popupMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, TR("Show Error Log") + CString(_T("\tCtrl+Shift+L")));

    TPMPARAMS excludeArea;
    ZeroMemory(&excludeArea, sizeof(excludeArea));
    excludeArea.cbSize = sizeof(excludeArea);
    excludeArea.rcExclude = rc;
    popupMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON, menuOrigin.x, menuOrigin.y, m_hWnd, &excludeArea);

    return 0;
}

void CWizardDlg::runInGuiThread(TaskRunnerTask&& task, bool async) {
    if (async) {
        std::lock_guard<std::mutex> lk(scheduledTasksMutex_);
        scheduledTasks_.push_back(std::move(task));
        PostMessage(WM_TASKDISPATCHERMSG, 0, 0);
    } else {
        if (GetCurrentThreadId() == mainThreadId_) {
            task();
        } else {
            TaskDispatcherMessageStruct msg;
            msg.callback = std::move(task);
            msg.async = false;
            SendMessage(WM_TASKDISPATCHERMSG, reinterpret_cast<WPARAM>(&msg), 0);
        }
    }
}

void CWizardDlg::showLogWindowForFileName(CString fileName) {
    if (fileName.IsEmpty()) {
        return;
    }

    auto it = logWindowsByFileName_.find(fileName);
    if (it != logWindowsByFileName_.end()) {
        it->second->Show();
        return;
    }

    auto wnd = std::make_unique<CLogWindow>();
    wnd->Create(nullptr);
    wnd->setFileNameFilter(fileName);
    wnd->setLogger(logger_.get());
    wnd->TranslateUI();
    
    wnd->reloadList();
    wnd->Show();
    logWindowsByFileName_[fileName] = std::move(wnd);
}

LRESULT CWizardDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (wParam == kNewFilesTimer) {
        std::lock_guard<std::mutex> lk(newImagesMutex_);
        for (const auto& item : newImages_) {
            AddImage(item.RealFileName, item.VirtualFileName, item.show);
        }
        newImages_.clear();
    }
    return 0;
}

LRESULT CWizardDlg::OnServersCheckerClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    ServersListTool::CServersCheckerDlg dlg(&Settings, uploadEngineManager_, uploadManager_, enginelist_, std::make_shared<NetworkClientFactory>());
    dlg.DoModal(m_hWnd);
    return 0;
}

bool CWizardDlg::acceptsDragnDrop() const {
    if (!IsWindowEnabled() || !DragndropEnabled) {
        return false;
    }

    if (CurPage != wpWelcomePage && CurPage != wpMainPage && CurPage != wpVideoGrabberPage) {
        return false;
    }
    return true;
}

void CWizardDlg::beginAddFiles() {
    CreatePage(wpMainPage);
    CMainDlg* MainDlg = getPage<CMainDlg>(wpMainPage);
    if (!MainDlg) {
        return;
    }

    MainDlg->ThumbsView.beginAdd();
}

void  CWizardDlg::endAddFiles() {
    CMainDlg* MainDlg = getPage<CMainDlg>(wpMainPage);
    if (!MainDlg) {
        return;
    }
    MainDlg->ThumbsView.endAdd();
}
