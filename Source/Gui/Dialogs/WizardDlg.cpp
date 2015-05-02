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
#include "WizardDlg.h"

#include <io.h>
#include "Core/Images/ImageConverter.h"
#include "Func/Base.h"
#include "Func/HistoryManager.h"

#include "welcomedlg.h"
#include "maindlg.h"
#include "VideoGrabberPage.h"
#include "uploadsettings.h"
#include "uploaddlg.h"
#include "aboutdlg.h"
#include "langselect.h"
#include "floatingwindow.h"
#include "TextViewDlg.h"
#include "ImageDownloaderDlg.h"
#include "LogWindow.h"
#include "Common/CmdLine.h"
#include "Gui/Dialogs/UpdateDlg.h"
#include "Func/Settings.h"
#include "Gui/Dialogs/MediaInfoDlg.h"
#include "Gui/GuiTools.h"
#include "Gui/Dialogs/ImageReuploaderDlg.h"
#include "Gui/Dialogs/ShortenUrlDlg.h"

#include "Gui/Dialogs/WebViewWindow.h"
#include "Core/Utils/CryptoUtils.h"
#include "Core/TextUtils.h"
#include "Func/WinUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Gui/Dialogs/QuickSetupDlg.h"
#include <ImageEditor/Gui/ImageEditorWindow.h>
#include "Func/ImageEditorConfigurationProvider.h"
#include "Core/Logging.h"
#include "Core/Images/Utils.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/Scripting/ScriptsManager.h"
#include "Func/myutils.h"


using namespace Gdiplus;
// CWizardDlg
CWizardDlg::CWizardDlg(): m_lRef(0), FolderAdd(this)
{ 
    screenshotIndex = 1;
    CurPage = -1;
    PrevPage = -1;
    NextPage = -1;
    m_StartingThreadId = 0;
    ZeroMemory(Pages, sizeof(Pages));
    DragndropEnabled = true;
    hLocalHotkeys = 0;
    QuickUploadMarker = false;
    m_bShowAfter = true;
    m_bHandleCmdLineFunc = false;
    updateDlg = 0;
    _EngineList = &m_EngineList;
    m_bScreenshotFromTray = false;
    serversChanged_ = false;
    scriptsManager_ = new ScriptsManager();
    uploadEngineManager_ = new UploadEngineManager(&m_EngineList);
    uploadManager_ = new UploadManager(uploadEngineManager_, scriptsManager_);
    floatWnd.setUploadManager(uploadManager_);
    floatWnd.setUploadEngineManager(uploadEngineManager_);
    Settings.addChangeCallback(CSettings::ChangeCallback(this, &CWizardDlg::settingsChanged));
}

void CWizardDlg::settingsChanged(CSettings* settings)
{
    CString templateName = settings->imageServer.getImageUploadParamsRef().getThumbRef().TemplateName;
    sessionImageServer_.getImageUploadParamsRef().getThumbRef().TemplateName = templateName;
}


CWizardDlg::~CWizardDlg()
{
    //Detach();
    if(updateDlg) delete updateDlg;
    for(int i=0; i<5; i++) 
    {
        CWizardPage *p = Pages[i];
        if(Pages[i]) delete p;
    }
    delete uploadManager_;
    delete uploadEngineManager_;
    delete scriptsManager_;

}

TCHAR MediaInfoDllPath[MAX_PATH] = _T("");
LRESULT CWizardDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYOUTRTL);  // test :))
    srand(unsigned int(time(0)));
    m_bShowWindow = true;
       
    LPDWORD DlgCreationResult = (LPDWORD) lParam; 
    
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
    OleInitialize(NULL);
    
    HRESULT res = ::RegisterDragDrop(m_hWnd, this);
    *MediaInfoDllPath=0;
    TCHAR Buffer[MAX_PATH];
    HKEY ExtKey;
    Buffer[0]=0;
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\KLCodecPack"), 0,/* REG_OPTION_NON_VOLATILE, */KEY_QUERY_VALUE,  &ExtKey/* NULL*/);
    TCHAR ClassName[MAX_PATH]=_T("\0");
    DWORD BufSize = sizeof(ClassName)/sizeof(TCHAR);
    DWORD Type = REG_SZ;
    RegQueryValueEx(ExtKey, _T("installdir"), 0, &Type, (LPBYTE)&ClassName, &BufSize);
    RegCloseKey(ExtKey);
 
    
    CString MediaDll = WinUtils::GetAppFolder() + _T("\\Modules\\MediaInfo.dll");
    if(WinUtils::FileExists( MediaDll)) lstrcpy(MediaInfoDllPath, MediaDll);
    else
    {
        CString MediaDll2 = CString(ClassName) + _T("\\Tools\\MediaInfo.dll");
        if(WinUtils::FileExists( MediaDll2)) lstrcpy(MediaInfoDllPath, MediaDll2);
    }
    SetWindowText(APPNAME);

   Lang.SetDirectory(WinUtils::GetAppFolder() + "Lang\\");
    bool isFirstRun =  Settings.Language.IsEmpty() || FALSE;
    for(size_t i=0; i<CmdLine.GetCount(); i++)
    {
        CString CurrentParam = CmdLine[i];
        if(CurrentParam .Left(10)==_T("/language="))
        {
            CString shortLanguageName = CurrentParam.Right(CurrentParam.GetLength()-10);
            CString foundName = Lang.getLanguageFileNameForLocale(shortLanguageName);
            if ( !foundName.IsEmpty() ) {
                Settings.Language = foundName;
            }
        }
    }
    if(isFirstRun)
    {
        CLangSelect LS;
        if(LS.DoModal(m_hWnd) == IDCANCEL)
        {
            *DlgCreationResult = 1;
            return 0;
        }
        Settings.Language = LS.Language;
        Lang.LoadLanguage(Settings.Language);
        
        /*if(MessageBox(TR("Добавить Image Uploader в контекстное меню проводника Windows?"),APPNAME, MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            Settings.ExplorerContextMenu = true;
            Settings.ExplorerContextMenu_changed = true;
            Settings.ExplorerVideoContextMenu = true;
        }    */
        Settings.SaveSettings();
    }
    else 
    {
        Lang.LoadLanguage(Settings.Language);
    }
    LogWindow.TranslateUI();

    CString ErrorStr;
    if(!LoadUploadEngines(IuCommonFunctions::GetDataFolder()+_T("servers.xml"), ErrorStr))  // Завершаем работу программы, если файл servers.lst отсутствует
    {
        CString ErrBuf;
        ErrBuf.Format(TR("Невозможно открыть файл со спиком серверов \"servers.xml\"!\n\nПричина:  %s\n\nПродолжить работу программы?"),(LPCTSTR)ErrorStr);
    
        if(MessageBox(ErrBuf, APPNAME, MB_ICONERROR|MB_YESNO)==IDNO)
        {
            *DlgCreationResult = 2;
            return 0;
        }
    }
    uploadEngineManager_->setScriptsDirectory(WCstringToUtf8(IuCommonFunctions::GetDataFolder() + _T("\\Scripts\\")));
    std::vector<CString> list;
    CString serversFolder = IuCommonFunctions::GetDataFolder() + _T("Servers\\");
    WinUtils::GetFolderFileList(list, serversFolder, _T("*.xml"));

    for(size_t i=0; i<list.size(); i++)
    {
        LoadUploadEngines(serversFolder+list[i], ErrorStr);
        //MessageBox(list[i]);
    }
    list.clear();

    CString userServersFolder = Utf8ToWCstring(Settings.SettingsFolder + "Servers\\");
    //MessageBox(userServersFolder);
    if ( userServersFolder != serversFolder) {
        
        WinUtils::GetFolderFileList(list, userServersFolder, _T("*.xml"));

        for(size_t i=0; i<list.size(); i++)
        {
            //MessageBox(list[i]);
            
            LoadUploadEngines(userServersFolder+list[i], ErrorStr);
        }
    }

    if ( Settings.urlShorteningServer.serverName().empty() ) {
        std::string defaultServerName = "is.gd";
        CUploadEngineData * uploadEngineData = static_cast<CUploadEngineList*>(&m_EngineList)->byName(defaultServerName);
        if ( uploadEngineData ) {
            Settings.urlShorteningServer.setServerName(defaultServerName);
        } else {
            uploadEngineData = m_EngineList.firstEngineOfType(CUploadEngineData::TypeUrlShorteningServer);
            if ( uploadEngineData ) {
                Settings.urlShorteningServer.setServerName(uploadEngineData->Name);
            }
        }
    }

    LoadUploadEngines(_T("userservers.xml"), ErrorStr);    

    if ( isFirstRun ) {
        CQuickSetupDlg quickSetupDialog;
        quickSetupDialog.DoModal(m_hWnd);
    }

    sessionImageServer_ = Settings.imageServer;
    sessionFileServer_ = Settings.fileServer;

    if(!*MediaInfoDllPath)
        WriteLog(logWarning, APPNAME, TR("Библиотека MediaInfo.dll не найдена. \nПолучение технических данных о файлах мультимедиа будет недоступно.")); 
    if(!CmdLine.IsOption(_T("tray")))
        TRC(IDCANCEL,"Выход");
    else 
        TRC(IDCANCEL,"Скрыть");
    TRC(IDC_UPDATESLABEL, "Проверить обновления");
    TRC(IDC_PREV,"< Назад");

    ACCEL accel;
    accel.cmd = ID_PASTE;
    accel.fVirt = FCONTROL|FVIRTKEY;
    accel.key =  VkKeyScan('v') ;
    hAccel = CreateAcceleratorTable(&accel, 1);

    RegisterLocalHotkeys();
    if(ParseCmdLine()) return 0;
 
    CreatePage(0); 
    ShowPage(0);
    ::SetFocus(Pages[0]->PageWnd);


    if(CmdLine.IsOption(_T("update")))
    {
        CreateUpdateDlg();
        updateDlg->ShowModal(m_hWnd);
    }
    else
    {
        if(time(0) - Settings.LastUpdateTime > 3600*24*3 /* 3 days */)
        {
            CreateUpdateDlg();
            updateDlg->Create(m_hWnd);
        }
    }
    return 0;  // Let the system set the focus
}

bool CWizardDlg::ParseCmdLine()
{ 
    int type = 0;
    int count = 0;

    int nIndex = 0;
    bool fromContextMenu = false;

    if(CmdLine.IsOption(_T("mediainfo")))
    {
        int nIndex = 0;
        CString VideoFileName;
        if(CmdLine.GetNextFile(VideoFileName, nIndex))
        {
            CMediaInfoDlg dlg;
            dlg.ShowInfo(VideoFileName);
            PostQuitMessage(0);
            return 0;
        }
    }

    if(CmdLine.IsOption(_T("imageeditor")))
    {
        int nIndex = 0;
        CString imageFileName;
        if(CmdLine.GetNextFile(imageFileName, nIndex))
        {
            using namespace ImageEditor;
            ImageEditorConfigurationProvider configProvider;
            ImageEditor::ImageEditorWindow imageEditor(imageFileName, &configProvider);
            imageEditor.showUploadButton(false);
            m_bShowWindow=false;
            ImageEditorWindow::DialogResult dr = imageEditor.DoModal(m_hWnd, ImageEditorWindow::wdmAuto);
            if ( dr == ImageEditorWindow::drCancel ) {
                PostQuitMessage(0);    
            } else {
                this->AddImage(imageFileName, myExtractFileName(imageFileName), true);
                //ShowPage(1);
                m_bShowAfter = true;
                m_bShowWindow = true;
                m_bHandleCmdLineFunc = true;
            }
            return 1;
        }
    }


    for(size_t i=0; i<CmdLine.GetCount(); i++)
    {
        CString CurrentParam = CmdLine[i];
        if ( CurrentParam == _T("/quickshot")  ) {
            m_bShowWindow=false;
            m_bHandleCmdLineFunc = true;
            if(!executeFunc(_T("regionscreenshot")))
                PostQuitMessage(0);
            return true;
        }
         else if(CurrentParam .Left(6)==_T("/func="))
        {
            m_bShowWindow=false;
            CString cmd = CurrentParam.Right(CurrentParam.GetLength()-6);
            m_bHandleCmdLineFunc = true;
            if(!executeFunc(cmd))
                PostQuitMessage(0);
            return true;
        } else if(CurrentParam .Left(15)==_T("/serverprofile=")) {
            CString serverProfileName = CurrentParam.Right(CurrentParam.GetLength()-15);
            
            if ( Settings.ServerProfiles.find(serverProfileName) == Settings.ServerProfiles.end()) {
                CString msg;
                msg.Format(TR("Профиль \"%s\" не найден."),TR("Ошибка"),MB_ICONWARNING);
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
        if(IsVideoFile(FileName) && !CmdLine.IsOption(_T("upload")))
        {
            ShowPage(1, CurPage, (Pages[2])?2:3);
            CVideoGrabberPage* dlg = (CVideoGrabberPage*) Pages[1];
            dlg->SetFileName(FileName);            
            return true;
        }    
    }
    nIndex = 0;
    CStringList Paths;
    while(CmdLine.GetNextFile(FileName, nIndex))
    {
        if(WinUtils::FileExists(FileName) || WinUtils::IsDirectory(FileName))
         Paths.Add(FileName);        
    }
    if(!Paths.IsEmpty())
    {
        QuickUploadMarker = (fromContextMenu && Settings.QuickUpload && !CmdLine.IsOption(_T("noquick"))) || (CmdLine.IsOption(_T("quick")));    
        FolderAdd.Do(Paths, CmdLine.IsOption(_T("imagesonly")), true);
    }
    return count!=0;
}

LRESULT CWizardDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(floatWnd.m_hWnd)
    { 
        ShowWindow(SW_HIDE);
        if(Pages[2] && CurPage == 4)
            ((CMainDlg*)Pages[2])->ThumbsView.MyDeleteAllItems();
        ShowPage(0); 
    }
    else
        CloseWizard();
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
            else if (Pages[0] && pMsg->hwnd==::GetDlgItem(Pages[0]->PageWnd,IDC_LISTBOX))
                return FALSE;
        }
        
        if(VK_BACK == pMsg->wParam)
        {
            if(Pages[CurPage] && VK_BACK == pMsg->wParam  && GetForegroundWindow() == m_hWnd && lstrcmpi(Buffer,_T("Edit") ))
            {
                if(pMsg->message==WM_KEYDOWN && ::IsWindowEnabled(GetDlgItem(IDC_PREV)))
                { 
                    OnPrevBnClicked(0,0,0); 
                    return TRUE;
                }
                else if (pMsg->message == WM_KEYUP) 
                    return TRUE;
            }
        }
    }

    if(hLocalHotkeys &&TranslateAccelerator(m_hWnd, hLocalHotkeys, pMsg)) 
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

bool CWizardDlg::ShowPage(int idPage,int prev,int next)
{
    if(idPage == CurPage) return true;

    if(GetCurrentThreadId()!=GetWindowThreadProcessId(m_hWnd, NULL))
    {
        return SendMessage(WM_MY_SHOWPAGE, (WPARAM)idPage)!=FALSE;
    }
   
     if(!CreatePage(idPage)) return false;

    SetDlgItemText(IDC_NEXT,TR("Далее >"));

    HBITMAP bmp = Pages[idPage]->HeadBitmap;
    if(!bmp) ::ShowWindow(GetDlgItem(IDC_HEADBITMAP),SW_HIDE);
    else
    {
        ::ShowWindow(GetDlgItem(IDC_HEADBITMAP),SW_SHOW);
        SendDlgItemMessage(IDC_HEADBITMAP,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bmp);
    }

    ::ShowWindow(Pages[idPage]->PageWnd, SW_SHOW);
    ::SetFocus(Pages[idPage]->PageWnd);
    Pages[idPage]->OnShow();
    
        ::ShowWindow(GetDlgItem(IDC_UPDATESLABEL), idPage == 0);
    ::ShowWindow(GetDlgItem(IDC_HELPBUTTON), idPage == 0);
    if(CurPage >= 0)
    {
        
        ::ShowWindow(Pages[CurPage]->PageWnd, SW_HIDE);
        Pages[CurPage]->OnHide();
    }
    
    PrevPage = prev;
    NextPage = next;
    CurPage = idPage;
    return false;
}

LRESULT CWizardDlg::OnPrevBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    if(PrevPage<0)
    {
        PrevPage = CurPage-1;
        if(PrevPage<0 || PrevPage==1)  PrevPage = 0;
    }    

    ShowPage(PrevPage);
    PrevPage=-1;
    return 0;
}

LRESULT CWizardDlg::OnNextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    if(!::IsWindowVisible(hWndCtl)) return 0;
    if(!Pages[CurPage]->OnNext()) return 0;
    if(NextPage < 0)
    {
        NextPage = CurPage+1;
        if(NextPage>4 ) NextPage=0;
        if(NextPage==1) NextPage=2;
    }    
    ShowPage(NextPage);
    NextPage = -1;
    return 0;
}

bool CWizardDlg::CreatePage(int PageID)
{
    RECT rc = {3,3,636,500};
    RECT rc2 = {3,100,636,500};
    RECT rcc;
    GetClientRect(&rcc);
    int width = rcc.right - rcc.left;
    int height = 430;
    if(((PVOID)Pages[PageID])!=0) return true;;
    switch(PageID)
    {
        case 0:
            CWelcomeDlg *tmp;
            tmp = new CWelcomeDlg();
            Pages[PageID] = tmp;
            Pages[PageID]->WizardDlg=this;
            tmp->Create(m_hWnd,rc);
            break;

        case 1:
            CVideoGrabberPage *tmp1;
            tmp1 = new CVideoGrabberPage(uploadEngineManager_);
            Pages[PageID]=tmp1;
            Pages[PageID]->WizardDlg=this;
            tmp1->Create(m_hWnd,rc);
            break;

        case 2:
            CMainDlg *tmp2;
            tmp2=new CMainDlg();
            Pages[PageID]=tmp2;
            Pages[PageID]->WizardDlg=this;
            tmp2->Create(m_hWnd,rc);
            break;

        case 3:
            CUploadSettings *tmp3;
            tmp3 = new CUploadSettings(&m_EngineList, uploadEngineManager_);
            Pages[PageID]=tmp3;
            Pages[PageID]->WizardDlg=this;
            tmp3->Create(m_hWnd,rc2);
            tmp3->SetWindowPos(0,0,50,0,0,SWP_NOSIZE);
            break;
        case 4:
            CUploadDlg *tmp4;
            tmp4=new CUploadDlg(this, uploadManager_);
            Pages[PageID]=tmp4;
            Pages[PageID]->WizardDlg=this;
            tmp4->Create(m_hWnd, rc);
            tmp4->SetWindowPos(0, 0, 50, 0, 0,SWP_NOSIZE);
            break;
        default:
            return false;
    }
    Pages[PageID]->HeadBitmap = GenHeadBitmap(PageID);
    return true;
}

void CWizardDlg::setSessionImageServer(ServerProfile server)
{
    sessionImageServer_ = server;
}

void CWizardDlg::setSessionFileServer(ServerProfile server)
{
    sessionFileServer_ = server;
}

ServerProfile CWizardDlg::getSessionImageServer() const
{
    return sessionImageServer_;
}

ServerProfile CWizardDlg::getSessionFileServer() const
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

// Функция генерации заголовка страницы (если он нужен)
HBITMAP CWizardDlg::GenHeadBitmap(int PageID)
{
    if(PageID!=3 && PageID!=4) return 0;
    RECT rc;
    GetClientRect(&rc);
    int width=rc.right-rc.left;
    RectF bounds(0.0,0.0, float(width), float(50));
    Bitmap *BackBuffer;
    Graphics g(m_hWnd,true);
    
    BackBuffer = new Bitmap(width, 50, &g);
    Graphics gr(BackBuffer);
    COLORREF color=GetSysColor(COLOR_BTNFACE);
    
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

    
    LPTSTR Buffer = NULL;
    if(PageID == 3)
        gr.DrawString(TR("Параметры изображений и выбор сервера"), -1, &font, bounds, &format, &br2);
    else if(PageID==4)
        gr.DrawString(TR("Загрузка файлов на сервер"), -1, &font, bounds, &format, &br2);

    HBITMAP bmp=0;
    BackBuffer->GetHBITMAP(Color(255,255,255), &bmp);
    delete BackBuffer;
    return bmp;
}

LRESULT CWizardDlg::OnBnClickedAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // TODO: Add your control notification handler code here
    CAboutDlg dlg;
    dlg.DoModal();
    return 0;
}

void CWizardDlg::Exit()
{
    Settings.SaveSettings();
}

LRESULT CWizardDlg::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true;
    HDROP hDrop = (HDROP) wParam;
    TCHAR szBuffer[256] = _T("\0");
    if(CurPage > 2) return 0;

    int n = DragQueryFile(hDrop,    0xFFFFFFFF, 0, 0);

    CMainDlg* MainDlg = NULL;
    CStringList Paths;
    
    for (int i=0; i<n; i++)
    {
        
        DragQueryFile(hDrop,    i, szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
        if((IsVideoFile(szBuffer) && n==1) && !Settings.DropVideoFilesToTheList)
        {
            if(CurPage == 2)
            {
                if(Settings.DropVideoFilesToTheList || MessageBox(TR("Вы хотите извлечь кадры из этого видеофайла? \r\n(иначе файл будет просто добавлен в список)"),APPNAME,MB_YESNO)==IDNO)
                    goto filehost;
            }
            ShowPage(1, CurPage, (Pages[2])?2:3);
            CVideoGrabberPage* dlg = (CVideoGrabberPage*) Pages[1];
            dlg->SetFileName(szBuffer);
            
            break;
        }
        else if(CurPage == 0 || CurPage == 2)
        {
            filehost:
            if(WinUtils::FileExists(szBuffer) || WinUtils::IsDirectory(szBuffer))
                                     Paths.Add(szBuffer);            
        }
 
    }
    if(!Paths.IsEmpty())
    {
        CreatePage(2);
        FolderAdd.Do(Paths, false, true);
        ShowPage(2);
        if(MainDlg) MainDlg->ThumbsView.LoadThumbnails();
    }
    
    DragFinish(hDrop);
    return 0;
   
}

bool CWizardDlg::LoadUploadEngines(const CString &filename, CString &Error)
{
    m_EngineList.setNumOfRetries(Settings.FileRetryLimit, Settings.ActionRetryLimit);
    bool Result = m_EngineList.LoadFromFile(filename);
    Error = m_EngineList.ErrorStr();
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
STDMETHODIMP CWizardDlg::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    return S_OK;
}
    
STDMETHODIMP CWizardDlg::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    bool AcceptFile = true;
    if(!IsWindowEnabled() || !DragndropEnabled ) {
        AcceptFile = false;
    }

    if(CurPage != 0 && CurPage!=2 && CurPage!=1) 
        AcceptFile = false;

    if(!AcceptFile) 
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_FALSE;
    }
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}
    
STDMETHODIMP CWizardDlg::DragLeave( void)
{
    return S_OK;
}

CString MakeTempFileName(const CString& FileName)
{
    CString FileNameBuf;
    FileNameBuf = IuCommonFunctions::IUTempFolder + FileName;

   if(WinUtils::FileExists(FileNameBuf))
    {
        CString OnlyName;
        OnlyName = WinUtils::GetOnlyFileName(FileName);
        CString Ext = WinUtils::GetFileExt(FileName);
        FileNameBuf = IuCommonFunctions::IUTempFolder + OnlyName + _T("_")+ WinUtils::IntToStr(GetTickCount()^33333) + (Ext? _T("."):_T("")) + Ext;
    }
    return FileNameBuf;
}

bool SaveFromHGlobal(HGLOBAL Data, const CString& FileName, CString& OutName)
{
    if(!Data) return false;
    CString FileNameBuf = MakeTempFileName(FileName);  

    DWORD filesize = GlobalSize(Data);
    if(!filesize) 
        return false;
    PVOID LockedData =(PVOID) GlobalLock (Data);

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
    if(!pStream) return false;
    CString FileNameBuf = MakeTempFileName(FileName);  

    HANDLE hFile = CreateFile(FileNameBuf, GENERIC_WRITE, 
                          0, NULL, CREATE_ALWAYS, 
                          FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;
                
    UCHAR bBuffer[4096];
    ULONG cbRead;
            
    while (SUCCEEDED(pStream->Read(bBuffer, sizeof(bBuffer), &cbRead)) && cbRead > 0)
    {
        WriteFile(hFile, bBuffer, cbRead, &cbRead, NULL);
    }

    CloseHandle(hFile);
    OutName = FileNameBuf;
    return true;
}

bool CWizardDlg::HandleDropFiledescriptors(IDataObject *pDataObj)
{
    FORMATETC tc2 = { RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    if(pDataObj->QueryGetData(&tc2)==S_OK )
    {
        STGMEDIUM ddd;

        if(pDataObj->GetData(&tc2, &ddd) == S_OK ){

            PVOID hdrop = (PVOID) GlobalLock ( ddd.hGlobal );
            FILEGROUPDESCRIPTOR *fgd = (FILEGROUPDESCRIPTOR*) hdrop;
            CStringList Paths;
            for(size_t i=0; i<fgd->cItems; i++)
            {
                FORMATETC tc3 = { RegisterClipboardFormat(CFSTR_FILECONTENTS), 0, DVASPECT_CONTENT, i, TYMED_HGLOBAL };
                if(pDataObj->QueryGetData(&tc3) == S_OK )
                {
                    STGMEDIUM ddd2;
                    ddd2.tymed = TYMED_HGLOBAL;
                    if(pDataObj->GetData(&tc3, &ddd2) == S_OK )
                    {
                        CString OutFileName;
                        bool FileWasSaved = false;
                        
                        if(ddd2.tymed == TYMED_HGLOBAL)
                        {
                            FileWasSaved = SaveFromHGlobal(ddd2.hGlobal, fgd->fgd[i].cFileName, OutFileName);
                        }

                        if(ddd2.tymed == TYMED_ISTREAM)
                        {    
                            FileWasSaved = SaveFromIStream(ddd2.pstm, fgd->fgd[i].cFileName, OutFileName); 
                        }

                        if(FileWasSaved) // Additing received file to program
                        {
                            if(IsVideoFile(OutFileName))
                            {
                                ShowPage(1, CurPage, (Pages[2])? 2 : 3);
                                CVideoGrabberPage* dlg = (CVideoGrabberPage*) Pages[1];
                                dlg->SetFileName(OutFileName);
                                break;
                            }
                            else if((CurPage==0||CurPage==2))
                            {
                                
                                if(WinUtils::FileExists(OutFileName) || WinUtils::IsDirectory(OutFileName))
                                     Paths.Add(OutFileName);        
                            }
                        }
                    }
                }

                GlobalUnlock ( hdrop );
                
                if(!Paths.IsEmpty())
                {
                    CreatePage(2);
                    //QuickUploadMarker = (Settings.QuickUpload && !CmdLine.IsOption(_T("noquick"))) || (CmdLine.IsOption(_T("quick")));
                    FolderAdd.Do(Paths, /*CmdLine.IsOption(_T("imagesonly"))*/false, true);
                    ShowPage(2);
                    return true;
                }
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

STDMETHODIMP CWizardDlg::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if(!IsWindowEnabled() || !DragndropEnabled) 
    {
        *pdwEffect = DROPEFFECT_NONE; 
        return S_FALSE;
    }
    

    // This should be called first 
    // otherwise dragndrop from Firefox will not work
    if(HandleDropFiledescriptors(pDataObj))
        return S_OK;

    if(HandleDropHDROP(pDataObj))
        return S_OK;

    if(HandleDropBitmap(pDataObj))
        return S_OK;

    return S_OK;
}

LRESULT CWizardDlg::OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(IsClipboardFormatAvailable(CF_BITMAP)) 
    {
        if(!OpenClipboard()) return 0;
        HBITMAP bmp = (HBITMAP) GetClipboardData(CF_BITMAP);

        if(!bmp) return CloseClipboard();

        PasteBitmap(bmp);
        CloseClipboard();
    }

    if(IsClipboardFormatAvailable(CF_UNICODETEXT)) 
    {
        CString text;
        IU_GetClipboardText(text);
        if(CImageDownloaderDlg::LinksAvailableInText(text))
        {
            CImageDownloaderDlg dlg(this,CString(text));
            dlg.DoModal(m_hWnd);
        }
    }    
    return 0;
}

LRESULT CWizardDlg::OnDocumentation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ShellExecute(0,L"open",WinUtils::GetAppFolder()+"Docs\\index.html",0,WinUtils::GetAppFolder()+"Docs\\",SW_SHOWNORMAL);
    return 0;
}

LRESULT CWizardDlg::OnShowLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
        LogWindow.Show();
        return 0;
}

void CWizardDlg::PasteBitmap(HBITMAP Bmp)
{
    if(CurPage!=0 && CurPage!=2) return;

    CString buf2;
    SIZE dim;
    GetBitmapDimensionEx(Bmp, &dim);
    Bitmap bm(Bmp,0);
    if(bm.GetLastStatus()==Ok)
    {
        MySaveImage(&bm,_T("clipboard"),buf2,1,100);

        CreatePage(2);
        CMainDlg* MainDlg = (CMainDlg*) Pages[2];
        MainDlg->AddToFileList( buf2);
        MainDlg->ThumbsView.LoadThumbnails();
        ShowPage(2);
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

CFolderAdd::CFolderAdd(CWizardDlg *WizardDlg): m_pWizardDlg(WizardDlg)
{
    m_bSubDirs = true; 
}

void CFolderAdd::Do(CStringList &Paths, bool ImagesOnly, bool SubDirs)
{
    count = 0;
    m_bImagesOnly = ImagesOnly;
    RECT rc={0,0,0,0};
    m_bSubDirs = SubDirs;
   if(!dlg.m_hWnd)
    dlg.Create(m_pWizardDlg->m_hWnd, rc);
    dlg.SetWindowTitle(CString(ImagesOnly?TR("Поиск графических файлов..."):TR("Сбор файлов...")));
    m_Paths.RemoveAll();
    m_Paths.Copy(Paths);
    findfile = 0;
    ZeroMemory(&wfd, sizeof(wfd));

    Start(THREAD_PRIORITY_BELOW_NORMAL);
}

int CFolderAdd::ProcessDir( CString currentDir, bool bRecursive /* = true  */ )
{
    CString strWildcard;
    
     dlg.SetInfo(CString(TR("Обрабатывается каталог:")), currentDir);
    strWildcard = currentDir + "\\*";

    _tfinddata_t s_Dir;
    intptr_t hDir;
  
    if( (hDir = _tfindfirst( strWildcard, &s_Dir )) == -1L )
        
         return 1;

    do
    {
         if(dlg.NeedStop()) { _findclose( hDir ); return 0;}

        if( s_Dir.name[ 0 ] != '.'  && ( s_Dir.attrib & _A_SUBDIR ) && bRecursive == true )
        {
            ProcessDir( currentDir + '\\' + s_Dir.name, bRecursive );
        }
        else if ( s_Dir.name[ 0 ] != '.' )
          {
                if(!m_bImagesOnly || IsImage(s_Dir.name))
                {
                    CWizardDlg::AddImageStruct ais;
                    ais.show = !m_pWizardDlg->QuickUploadMarker;
                    CString name = CString(currentDir) + CString(_T("\\"))+ CString( s_Dir.name);
                    ais.RealFileName = name;
                    if(SendMessage(m_pWizardDlg->m_hWnd, WM_MY_ADDIMAGE,(WPARAM) &ais,0 ))
                        count++;
                }
          }
     } while( _tfindnext( hDir, &s_Dir ) == 0 );

    _findclose( hDir );

    return 0;
}

DWORD CFolderAdd::Run()
{
    EnableWindow(m_pWizardDlg->m_hWnd, false);
    for(size_t i=0; i<m_Paths.GetCount(); i++)
    {
        CString CurPath = m_Paths[i];
        if(WinUtils::IsDirectory(CurPath))
            ProcessDir(CurPath, m_bSubDirs);
        else 
            if(!m_bImagesOnly || IsImage(CurPath))
            {
                CWizardDlg::AddImageStruct ais;
                ais.show = !m_pWizardDlg->QuickUploadMarker;
                CString name = CurPath;
                ais.RealFileName = CurPath;
                if(SendMessage(m_pWizardDlg->m_hWnd, WM_MY_ADDIMAGE,(WPARAM) &ais,0 ))
        
                count++;
            }
        if(dlg.NeedStop()) break;
    }

    dlg.Hide();
    EnableWindow(m_pWizardDlg->m_hWnd, true);

    if(!count) 
        MessageBox(m_pWizardDlg->m_hWnd, m_bImagesOnly?TR("Не найдено ни одного файла изображений."):TR("Не найдено ни одного файла."), APPNAME, MB_ICONINFORMATION);
    else     
    {
        if( m_pWizardDlg->QuickUploadMarker)
        {

            m_pWizardDlg->ShowPage(4);
        }
        else
        {

            m_pWizardDlg->ShowPage(2);

            ((CMainDlg*) m_pWizardDlg->Pages[2])->ThumbsView.LoadThumbnails();
        }
    }
    dlg.DestroyWindow();
    dlg.m_hWnd = NULL;
    return 0;
}

int CFolderAdd::GetNextImgFile(LPTSTR szBuffer, int nLength)
{
    TCHAR szBuffer2[MAX_PATH], TempPath[256];
    
    GetTempPath(256, TempPath);
    wsprintf(szBuffer2,_T("%s*.*"), (LPCTSTR)m_szPath);
    
    if(!findfile)
    {
        findfile=FindFirstFile(szBuffer2, &wfd);
        if(!findfile) goto error;
    }
    else 
    {
        if(!FindNextFile(findfile, &wfd))
            goto error;

    }
    if(lstrlen(wfd.cFileName) < 1) goto error;
    lstrcpyn(szBuffer, wfd.cFileName, nLength);

    return TRUE;

    error:
    if(findfile) FindClose(findfile);
    return FALSE;
}

bool CWizardDlg::AddImage(const CString &FileName, const CString &VirtualFileName, bool Show)
{
    CreatePage(2);
    CMainDlg* MainDlg = (CMainDlg*) Pages[2];
    if(!MainDlg) return false;
    MainDlg->AddToFileList(FileName, VirtualFileName);
    if(Show){
        MainDlg->ThumbsView.LoadThumbnails();
        ShowPage(2);
    }
    return true;
}

LRESULT CWizardDlg::OnAddImages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{    
    AddImageStruct* ais = reinterpret_cast<AddImageStruct*>(wParam);
    if(!ais) return 0;
    return  AddImage(ais->RealFileName, ais->VirtualFileName, ais->show);
}

CMyFolderDialog::CMyFolderDialog(HWND hWnd):
                CFolderDialogImpl(hWnd, TR("Выбор папки"), BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON|BIF_VALIDATE )
{
    OleInitialize(NULL);
}
void CMyFolderDialog::OnInitialized()
{
    HWND wnd = CreateWindowEx(0, _T("button"), TR("Включая поддиректории"), WS_VISIBLE|BS_CHECKBOX|WS_CHILD|BS_AUTOCHECKBOX, 15,30, 200,24, m_hWnd, 0,0, 0);
    SendMessage(wnd, WM_SETFONT, (WPARAM)SendMessage(m_hWnd, WM_GETFONT, 0,0),  MAKELPARAM(false, 0));
    SendMessage(wnd, BM_SETCHECK, (WPARAM)(m_bSubdirs?BST_CHECKED    :BST_UNCHECKED),0);
    SetProp(m_hWnd, PROP_OBJECT_PTR, (HANDLE) this);
    OldProc  = (DLGPROC) SetWindowLongPtr(m_hWnd, DWLP_DLGPROC, (LONG_PTR)DialogProc);    
    SubdirsCheckbox = wnd;
    m_bSubdirs = true;
}

//  Overloaded WinProc function for BrowseForFolders dialog
BOOL CALLBACK CMyFolderDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMyFolderDialog *th = (CMyFolderDialog *) GetProp(hwndDlg, PROP_OBJECT_PTR);
    if(!th) return FALSE;

    if(uMsg == WM_COMMAND && HIWORD(wParam)== BN_CLICKED && ((HWND) lParam)== th->SubdirsCheckbox)
        th->m_bSubdirs = SendMessage(th->SubdirsCheckbox, BM_GETCHECK,0,0) == BST_CHECKED;
    
    return th->OldProc(hwndDlg, uMsg, wParam, lParam);
}

 LRESULT CWizardDlg::OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true; 
    return 1;
}
    
LRESULT     CWizardDlg::OnWmShowPage(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    int PageIndex = wParam;
    ShowPage(PageIndex);
    return 0;
}
    
bool CWizardDlg::funcAddImages(bool AnyFiles)
{
    TCHAR Buf[MAX_PATH * 4];
    if(AnyFiles)
        GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR), 1, TR("Любые файлы"),
                                      _T("*.*"));
    else
    GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR), 2, 
                                   CString(TR("Изображения")) + _T(" (jpeg, bmp, png, gif ...)"),
                                   _T("*.jpg;*.jpeg;*.gif;*.png;*.bmp;*.tiff"), TR("Любые файлы"),
                                   _T("*.*"));

    int nCount = 0;  
    CMultiFileDialog fd(0, 0, OFN_HIDEREADONLY, Buf, m_hWnd);
    
    TCHAR Buffer[1000];
    fd.m_ofn.lpstrInitialDir = Settings.ImagesFolder;
    
    if(fd.DoModal(m_hWnd) != IDOK) return 0;
    
    LPCTSTR FileName = 0;
    fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

    CreatePage(2);
    do
    {
        
        FileName = (FileName) ? fd.GetNextFileName() : fd.GetFirstFileName();
        if (!FileName) break;
        fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

        if(Buffer[lstrlen(Buffer)-1] != '\\')
        lstrcat(Buffer, _T("\\"));
        
        if (FileName)
        {
            lstrcat(Buffer, FileName);
            if (((CMainDlg*)Pages[2])->AddToFileList(Buffer))
                nCount++;
        }
    } while (FileName);
     
    
    fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));
    Settings.ImagesFolder = Buffer;
    if(nCount)
        ShowPage(2, 0, 3);

    if(CurPage == 2)
        ((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
    ShowWindow(SW_SHOW);
    m_bShowWindow = true;
    return true;
}

bool CWizardDlg::executeFunc(CString funcBody)
{
    bool LaunchCopy = false;

    if(CurPage == 4) LaunchCopy = true;
    if(CurPage == 1) LaunchCopy = true;
    if(CurPage == 3) ShowPage(2);

    if(!IsWindowEnabled())LaunchCopy= true; 

    CString funcName = StringSection(funcBody,  _T(','), 0 );
    CString funcParam1 = StringSection(funcBody, _T(','), 1);

    if(!funcParam1.IsEmpty())
    {
            m_bScreenshotFromTray = _ttoi(funcParam1);
    }
    if(LaunchCopy)
    {
        if(Settings.TrayIconSettings.DontLaunchCopy)
        {    
             if(IsWindowVisible() && IsWindowEnabled())
                SetForegroundWindow(m_hWnd);
            else if(!IsWindowEnabled()) SetActiveWindow();
            FlashWindow(true);
        }
        else
            IULaunchCopy(_T("/func=")+funcBody,CAtlArray<CString>());
        return false;
    }
    if(funcName == _T("addimages"))
        return funcAddImages();
    /*else if(funcName == _T("addfiles"))
        return funcAddImages(true);*/
    if(funcName == _T("addfiles"))
        return funcAddFiles();
    else if(funcName == _T("importvideo"))
        return funcImportVideo();
    else if(funcName == _T("screenshotdlg"))
        return funcScreenshotDlg();
    else if(funcName == _T("regionscreenshot"))
        return funcRegionScreenshot();
    else if(funcName == _T("regionscreenshot_dontshow"))
        return funcRegionScreenshot(false);
    else if(funcName == _T("fullscreenshot"))
        return funcFullScreenshot();
    else if(funcName == _T("windowhandlescreenshot"))
        return funcWindowHandleScreenshot();
    else if(funcName == _T("freeformscreenshot"))
            return funcFreeformScreenshot();
    else if(funcName == _T("downloadimages"))
        return funcDownloadImages();
    else if(funcName == _T("windowscreenshot"))
        return funcWindowScreenshot();
    else if(funcName == _T("windowscreenshot_delayed"))
        return funcWindowScreenshot(true);
    else if(funcName == _T("addfolder"))
        return funcAddFolder();
    else if(funcName == _T("paste"))
        return funcPaste();
    else if(funcName == _T("settings"))
        return funcSettings();
    else if(funcName == _T("reuploadimages"))
        return funcReuploadImages();
    else if(funcName == _T("shortenurl"))
        return funcShortenUrl();
    else if(funcName == _T("mediainfo"))
        return funcMediaInfo();

    return false;
}

bool CWizardDlg::funcImportVideo()
{
    TCHAR Buf[MAX_PATH*4];
    GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
            CString(TR("Видео файлы"))+ _T(" (avi, mpg, vob, wmv ...)"),
            Settings.prepareVideoDialogFilters(),
        TR("Все файлы"),
        _T("*.*"));

    CFileDialog fd(true,0,0,4|2,Buf,m_hWnd);
    
    TCHAR Buffer[1000];
    fd.m_ofn.lpstrInitialDir = Settings.VideoFolder;
    if(fd.DoModal()!=IDOK || !fd.m_szFileName[0]) return 0;
    WinUtils::ExtractFilePath(fd.m_szFileName, Buffer); 
    Settings.VideoFolder = Buffer;
    CreatePage(1);
    LastVideoFile = fd.m_szFileName;
    ((CVideoGrabberPage*)Pages[1])->SetFileName(fd.m_szFileName); // C-style conversion .. 
    ShowPage(1,0,(Pages[2])?2:3);
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
    CommonScreenshot(cmRectangles);
    return true;
}

void CWizardDlg::OnScreenshotFinished(int Result)
{
    EnableWindow();

    if(m_bShowAfter || (Result && !floatWnd.m_hWnd))
    {
        m_bShowWindow = true;
        ShowWindow(SW_SHOWNORMAL);
        SetForegroundWindow(m_hWnd);
    }

    if(Result )
    {
        if((CMainDlg*)Pages[2])
        {
            ((CMainDlg*)Pages[2])->ThumbsView.SetFocus();
            ((CMainDlg*)Pages[2])->ThumbsView.SelectLastItem();
        }
    }
    else if (!Result && m_bHandleCmdLineFunc)
    {
        
        PostQuitMessage(0);
    }
    m_bHandleCmdLineFunc = false;

}

void CWizardDlg::OnScreenshotSaving(LPTSTR FileName, Bitmap* Bm)
{
    if(FileName && lstrlen(FileName))
    {
        CreatePage(2);
        ((CMainDlg*)Pages[2])->AddToFileList(FileName);
        if(CurPage == 2)
        ((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
        ShowPage(2,0,3);
    }
}

bool CWizardDlg::funcFullScreenshot()
{
    CommonScreenshot(cmFullScreen);    
    return true;
}

bool CWizardDlg::funcWindowScreenshot(bool Delay)
{
    CommonScreenshot(cmActiveWindow);
    return true;
}

bool CWizardDlg::funcAddFolder()
{
    CMyFolderDialog fd(m_hWnd);
    fd.m_bSubdirs = Settings.ParseSubDirs;
    if(fd.DoModal(m_hWnd) == IDOK)
    {
        Settings.ParseSubDirs = fd.m_bSubdirs;
        ShowWindow(SW_SHOW);
        m_bShowWindow = true;
        AddFolder(fd.GetFolderPath(),fd.m_bSubdirs);
        
        return true;
    }
    else return false;
}
LRESULT CWizardDlg::OnEnable(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if(!floatWnd.m_hWnd)
      TRC(IDCANCEL, "Выход");
    else 
        TRC(IDCANCEL, "Скрыть");

    if(!(m_hotkeys == Settings.Hotkeys))
    {
        UnRegisterLocalHotkeys();
        RegisterLocalHotkeys();
    }
    return 0;
}

LRESULT CWizardDlg::OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CString webViewClass(_T("CWebViewWindow"));
    CString fileDialogClass(_T("FileDialogSubclassWindow"));
    CString dialogClass(_T("#32770"));
    HWND browserWindow = CWebViewWindow::window;
    if ( !browserWindow || ::IsWindowVisible(browserWindow) ) {
        return 0;
    }
    if ( wParam == WA_INACTIVE ) {
        HWND wnd = (HWND)lParam;
        if ( wnd == 0 ) {
            //LOG(INFO) << "wnd=0.  SetActiveWindow();";
            SetActiveWindow();
            bHandled = true;
            return 0;
        }
        TCHAR Buffer[MAX_PATH] = _T("");
        GetClassName(wnd, Buffer, sizeof(Buffer)/sizeof(TCHAR));
        //LOG(INFO) << "CWizardDlg::OnActivate0 class="<< Buffer << " wnd="<<wnd << " title = "<< GuiTools::GetWindowText(wnd);
        if ( Buffer[0] == 0 ) {
            //LOG(INFO) << "Buffer=0.  SetActiveWindow();";
            SetActiveWindow();
            bHandled = true;
            return 0;
        }
        if ( (Buffer == dialogClass || Buffer == fileDialogClass) ) {
            //LOG(INFO) << "CWizardDlg::OnActivate1 "<< Buffer;
            HWND parent = ::GetParent(wnd);
            if ( parent ) {
                GetClassName(parent, Buffer, sizeof(Buffer)/sizeof(TCHAR));
                //LOG(INFO) << "CWizardDlg::OnActivate2 "<< Buffer;
                if ( (Buffer == dialogClass || Buffer == fileDialogClass) ) {
                     parent = ::GetParent(parent);
                    if ( parent ) {
                        //LOG(INFO) << "CWizardDlg::OnActivate3 "<< Buffer;
                        GetClassName(parent, Buffer, sizeof(Buffer)/sizeof(TCHAR));
                        if ( Buffer == webViewClass && !::IsWindowVisible(parent) ){
                            SetActiveWindow();
                            bHandled = true;
                            return 0;
                        }
                    } else {
                        //LOG(INFO) << "CWizardDlg::OnActivate3 parent is null";
                    

                            SetActiveWindow();
                            bHandled = true;
                            return 0;
                    
                        
                    }
                } else if ( Buffer ==webViewClass && !::IsWindowVisible(parent) ){
                    SetActiveWindow();
                    bHandled = true;
                    return 0;
                }
                
            }
            
        }
        
    }
    return 0;
}

void CWizardDlg::CloseWizard()
{
    if(CurPage!=0 && CurPage!=4 && Settings.ConfirmOnExit)
        if(MessageBox(TR("Вы уверены что хотите выйти из программы?"),APPNAME, MB_YESNO|MB_ICONQUESTION) != IDYES) return ;
    
    CloseDialog(0);
}


bool CWizardDlg::RegisterLocalHotkeys()
{
    ACCEL *Accels;
    m_hotkeys = Settings.Hotkeys;
    int n=m_hotkeys.size();
    Accels = new ACCEL [n];
    int j =0;
    for(int i =0; i<n; i++)
    {
        if(!m_hotkeys[i].localKey.keyCode) continue;
        Accels[j]= m_hotkeys[i].localKey.toAccel();
        Accels[j].cmd = 10000+i;
        j++;
            
    }

    hLocalHotkeys = CreateAcceleratorTable(Accels,j);
    delete[] Accels;
    return true;
}

LRESULT CWizardDlg::OnLocalHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(CurPage==3) ShowPage(2);
    if(!IsWindowEnabled() || (CurPage!=0 && CurPage!=2))
        return 0;
    int hotkeyId = wID-ID_HOTKEY_BASE;
    executeFunc(m_hotkeys[hotkeyId].func);
    return 0;
}

bool CWizardDlg::UnRegisterLocalHotkeys()
{
    if ( hLocalHotkeys ) {
        DestroyAcceleratorTable(hLocalHotkeys);
    }
    //LOG(INFO) << "m_hotkeys="<<m_hotkeys.GetCount();
    m_hotkeys.clear();
    hLocalHotkeys = 0;
    return true;
}

bool CWizardDlg::funcPaste()
{
    BOOL b;
    OnPaste(0,0,0,b);
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
    dlg.DoModal(m_hWnd);
    return true;
}

bool CWizardDlg::funcMediaInfo()
{
    TCHAR Buf[MAX_PATH*4]; //String buffer which will contain filter for CFileDialog
    GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),3, 
            CString(TR("Видео файлы"))+ _T(" (avi, mpg, vob, wmv ...)"),
        /*_T("*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;*.rm;")*/
        Settings.prepareVideoDialogFilters(),
        CString(TR("Аудио файлы"))+ _T(" (mp3, wma, wav ...)"),
        _T("*.mp3;*.wav;*.wma;*.mid;*.asx"),
        
        TR("Все файлы"),
        _T("*.*"));

    CFileDialog fd(true,0,0,4|2,Buf,m_hWnd);
    fd.m_ofn.lpstrInitialDir = Settings.VideoFolder;

    if(fd.DoModal()!=IDOK || !fd.m_szFileName[0]) return 0;
    TCHAR Buffer[512];
    WinUtils::ExtractFilePath(fd.m_szFileName, Buffer);
    Settings.VideoFolder = Buffer;
    CMediaInfoDlg dlg;
    LastVideoFile = fd.m_szFileName;
    dlg.ShowInfo(fd.m_szFileName);
    return true;
}

bool CWizardDlg::funcAddFiles()
{
    TCHAR Buf[MAX_PATH*4];
    GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),1, TR("Любые файлы"), _T("*.*"));

    int nCount=0;
    CMultiFileDialog fd(0, 0, OFN_HIDEREADONLY, Buf, m_hWnd);
    
    TCHAR Buffer[1000];
    fd.m_ofn.lpstrInitialDir = Settings.ImagesFolder;

    if(fd.DoModal(m_hWnd) != IDOK) return 0;
    LPCTSTR FileName = 0;
    fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

    CreatePage(2);
    do
    {
        
        FileName = (FileName) ? fd.GetNextFileName() : fd.GetFirstFileName();
        if(!FileName) break;
        fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

        if(Buffer[lstrlen(Buffer)-1] != '\\')
        lstrcat(Buffer, _T("\\"));
        
        if(FileName)
        {
            lstrcat(Buffer, FileName);
            if(((CMainDlg*)Pages[2])->AddToFileList(Buffer))
                nCount++;
        
        }
    } while (FileName);
     
    
    fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));
    Settings.ImagesFolder = Buffer;
    if(nCount)
        ShowPage(2, 0, 3);

    if(CurPage == 2)
        ((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
    ShowWindow(SW_SHOW);
    m_bShowWindow = true;
    return true;
}

LRESULT CWizardDlg::OnWmMyExit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(wParam  == 5)
    {
        CloseDialog(0);
    }
    return 0;
}

bool CWizardDlg::CanShowWindow()
{
    return (CurPage == 2 || CurPage == 0) && IsWindowVisible() && IsWindowEnabled();
}

void CWizardDlg::UpdateAvailabilityChanged(bool Available)
{
    if(Available)
    {
        TRC(IDC_UPDATESLABEL, "Доступны обновления");
    }
}
    
LRESULT CWizardDlg::OnUpdateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    CreateUpdateDlg();
    updateDlg->ShowModal(m_hWnd);
    return 0;
}

void CWizardDlg::CreateUpdateDlg()
{
    if(!updateDlg)
    {
        updateDlg = new CUpdateDlg();
        updateDlg->setUpdateCallback(this);
    }
}
  



bool CWizardDlg::CommonScreenshot(CaptureMode mode)
{
    bool needToShow = IsWindowVisible()!=FALSE;
    if(m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_UPLOAD   && !floatWnd.m_hWnd)
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
    if(mode == cmFullScreen)
    {
        engine.setDelay(WindowHidingDelay + Settings.ScreenshotSettings.Delay*1000);
        engine.captureScreen();
        result = std::shared_ptr<Gdiplus::Bitmap>(engine.capturedBitmap());
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
        result = std::shared_ptr<Gdiplus::Bitmap>(engine.capturedBitmap());
    }
    else if(engine.captureScreen())
    {
        if ( mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod ) {
            result = std::shared_ptr<Gdiplus::Bitmap>(engine.capturedBitmap());
        } else {
            RegionSelect.Parent = m_hWnd;
            SelectionMode selMode;
            if(mode == cmFreeform)
                selMode = smFreeform;
            if(mode == cmRectangles)
                selMode = smRectangles;
            if(mode == cmWindowHandles)
                selMode = smWindowHandles;

            RegionSelect.m_SelectionMode = selMode;
            std::shared_ptr<Gdiplus::Bitmap> res(engine.capturedBitmap());
            if(res)
            {
                HBITMAP gdiBitmap=0;
                res->GetHBITMAP(Color(255,255,255), &gdiBitmap);
                if(RegionSelect.Execute(gdiBitmap, res->GetWidth(), res->GetHeight()))
                {
                    if(RegionSelect.wasImageEdited() || (mode!=cmWindowHandles /*|| !Settings.ScreenshotSettings.ShowForeground*/) )
                    engine.setSource(gdiBitmap);
                    
                    else{
                        engine.setSource(0);
                    }
                    
                    engine.setDelay(0);
                    CScreenshotRegion* rgn = RegionSelect.region();
                    if(rgn)
                    {
                        CWindowHandlesRegion *whr =  dynamic_cast<CWindowHandlesRegion*>(rgn);
                        if(whr)
                        {
                            whr->SetWindowHidingDelay(int(Settings.ScreenshotSettings.WindowHidingDelay*1.2));
                            whr->setWindowCapturingFlags(wcfFlags);
                        }
                        engine.captureRegion(rgn);    
                        result = std::shared_ptr<Gdiplus::Bitmap>(engine.capturedBitmap());
                        DeleteObject(gdiBitmap);
                    }
                }
                else CanceledByUser = true;
            }
        }
    }
    using namespace ImageEditor;
    ImageEditorWindow::DialogResult dr = ImageEditorWindow::drCancel;
    CString suggestingFileName;
    if ( result ) {
        suggestingFileName = IuCommonFunctions::GenerateFileName(Settings.ScreenshotSettings.FilenameTemplate, screenshotIndex,CPoint(result->GetWidth(),result->GetHeight()));
    }

    if(result && ( (mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod) || (!m_bScreenshotFromTray && Settings.ScreenshotSettings.OpenInEditor ) || (m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_OPENINEDITOR) ))
    {
        ImageEditorConfigurationProvider configProvider;
        ImageEditor::ImageEditorWindow imageEditor(result, mode == cmFreeform ||   mode == cmActiveWindow, &configProvider);
        imageEditor.setInitialDrawingTool((mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod) ? ImageEditor::Canvas::dtCrop : ImageEditor::Canvas::dtBrush);
        imageEditor.showUploadButton(m_bScreenshotFromTray);
        if ( m_bScreenshotFromTray ) {
            imageEditor.setServerName(Utf8ToWCstring(Settings.quickScreenshotServer.serverName()));
        }
        imageEditor.setSuggestedFileName(suggestingFileName);
        dr = imageEditor.DoModal(m_hWnd, ((mode == cmRectangles && !Settings.ScreenshotSettings.UseOldRegionScreenshotMethod) || mode == cmFullScreen ) ? ImageEditorWindow::wdmFullscreen : ImageEditorWindow::wdmAuto);
        if ( dr == ImageEditorWindow::drAddToWizard || dr == ImageEditorWindow::drUpload ) {
            result = imageEditor.getResultingBitmap();
        }else {
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
            int savingFormat = Settings.ScreenshotSettings.Format;
            if(savingFormat == 0) // jpeg
                Gdip_RemoveAlpha(*result,Color(255,255,255,255));

            CString saveFolder = IuCommonFunctions::GenerateFileName(Settings.ScreenshotSettings.Folder, screenshotIndex,CPoint(result->GetWidth(),result->GetHeight()));
            MySaveImage(result.get(),suggestingFileName,buf,savingFormat, Settings.ScreenshotSettings.Quality,(Settings.ScreenshotSettings.Folder.IsEmpty())?0:(LPCTSTR)saveFolder);
            screenshotIndex++;
            if ( CopyToClipboard )
            {
                CDC dc = GetDC();
                if ( CopyBitmapToClipboard(m_hWnd, dc, result.get()) ) { // remove alpha if saving format is JPEG
                    if(m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_CLIPBOARD) {
                        floatWnd.ShowBaloonTip(TR("Снимок сохранен в буфере обмена"),_T("Image Uploader"));
                        Result = false;
                    }
                }
            }
            if(!m_bScreenshotFromTray || dr == ImageEditorWindow::drAddToWizard || (Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_ADDTOWIZARD || Settings.TrayIconSettings.TrayScreenshotAction== TRAY_SCREENSHOT_SHOWWIZARD))
            {
                CreatePage(2); 
                ((CMainDlg*)Pages[2])->AddToFileList(buf);
                ((CMainDlg*)Pages[2])->ThumbsView.EnsureVisible(((CMainDlg*)Pages[2])->ThumbsView.GetItemCount()-1,true);
                ((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
                ((CMainDlg*)Pages[2])->ThumbsView.SetFocus();
                ShowPage(2,0,3);
            }
            else if(m_bScreenshotFromTray && (Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_UPLOAD || dr == ImageEditorWindow::drUpload))
            {
                Result = false;
                floatWnd.UploadScreenshot(buf,buf);
            }

        }
        else
        {
            MessageBox(_T("Невозможно сделать снимок экрана!"));
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
    return CommonScreenshot(cmWindowHandles);
}

bool CWizardDlg::funcFreeformScreenshot()
{
    return CommonScreenshot(cmFreeform);
}

bool CWizardDlg::IsClipboardDataAvailable()
{
    bool IsClipboard = IsClipboardFormatAvailable(CF_BITMAP)!=FALSE;

    if(!IsClipboard)
    {
        if(IsClipboardFormatAvailable(CF_UNICODETEXT)) 
        {
            CString text;
            IU_GetClipboardText(text);
            if(CImageDownloaderDlg::LinksAvailableInText(text))
            {
                IsClipboard = true;
            }
        }
    }
    return IsClipboard;
}

bool CWizardDlg::funcReuploadImages() {
    CImageReuploaderDlg dlg(this, _EngineList, uploadManager_, uploadEngineManager_, CString());
    dlg.DoModal(m_hWnd);
    return false;
}

bool CWizardDlg::funcShortenUrl() {
    CShortenUrlDlg dlg(this,_EngineList, uploadManager_, CString());
    dlg.DoModal();
    return false;
}

CWizardDlg * pWizardDlg;
LRESULT CWizardDlg::OnBnClickedHelpbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    // TODO: Add your control notification handler code here
    RECT rc;
    ::GetWindowRect(hWndCtl, &rc );
    POINT menuOrigin = {rc.left,rc.bottom};

    CMenu popupMenu;
    popupMenu.CreatePopupMenu();
    popupMenu.AppendMenu(MF_STRING, IDC_ABOUT, TR("О программе..."));
    popupMenu.AppendMenu(MF_STRING, IDC_DOCUMENTATION, TR("Документация"));
    popupMenu.AppendMenu(MF_STRING, IDC_UPDATESLABEL, TR("Проверить обновления"));
    popupMenu.AppendMenu(MF_SEPARATOR, 99999,_T(""));
    popupMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, TR("Показать лог"));


    popupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, menuOrigin.x, menuOrigin.y, m_hWnd);

    return 0;
    return 0;
}
