/*
     Image Uploader - program for uploading images/files to the Internet

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

#include "atlheaders.h" 
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/wizarddlg.h"
#include "Gui/Dialogs/floatingwindow.h"
#include "Func/CmdLine.h"
#include "Func/WinUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Core/Logging.h"
#include "Core/Logging/MyLogSink.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include <boost/filesystem/path.hpp>
#include <boost/locale.hpp>
#include "Core/ServiceLocator.h"
#include "Func/DefaultUploadErrorHandler.h"
#include "Func/DefaultLogger.h"
#include "Func/WtlScriptDialogProvider.h"
#include "Core/AppParams.h"
#include "Func/LangClass.h"
#include "Func/GdiPlusInitializer.h"
#include "Gui/Dialogs/LangSelect.h"
#include "versioninfo.h"

#ifndef NDEBUG
//#include <vld.h>
#endif

CAppModule _Module;
CLogWindow LogWindow;
WtlGuiSettings Settings;
CLang Lang;

int Run(LPTSTR lpstrCmdLine, int nCmdShow, DefaultLogger* defaultLogger)
{
    CString commonTempFolder, tempFolder;
    IuCommonFunctions::CreateTempFolder(commonTempFolder, tempFolder);
    
    AppParams::instance()->setTempDirectory(W2U(tempFolder));
    std::vector<CString> fileList;
    WinUtils::GetFolderFileList( fileList, WinUtils::GetAppFolder() + _T("\\"), _T("*.old") );
    for ( const auto& file : fileList ) {
        DeleteFile(WinUtils::GetAppFolder() + file);
    }

    GdiPlusInitializer gdiPlusInitializer;

    CMessageLoop theLoop;
    _Module.AddMessageLoop( &theLoop );
    CWizardDlg  dlgMain(defaultLogger);
    
    DWORD DlgCreationResult = 0;
    bool ShowMainWindow     = true;
    
    Settings.LoadSettings();

    if ( CmdLine.IsOption( _T("uninstall") ) ) {
        Settings.Uninstall();
        return 0;
    }

    bool BecomeTray = false;
    if ( Settings.ShowTrayIcon && !CmdLine.IsOption( _T("tray") ) ) {
        if (!CFloatingWindow::IsRunningFloatingWnd()) {
            BecomeTray = true;
            CmdLine.AddParam( _T("/tray") );
        }    
    }

    bool bCreateFloatingWindow = false;

    if ( CmdLine.IsOption( _T("tray") ) || BecomeTray ) {
        if (!CFloatingWindow::IsRunningFloatingWnd()) {
            bCreateFloatingWindow = true;
            ShowMainWindow        = BecomeTray;    
        } else {
            return 0;
        }
    }

    ServiceLocator::instance()->setProgramWindow(&dlgMain);
    floatWnd.setWizardDlg(&dlgMain);

    Lang.SetDirectory(WinUtils::GetAppFolder() + "Lang\\");
    bool isFirstRun = Settings.Language.IsEmpty() || FALSE;
    for (size_t i = 0; i<CmdLine.GetCount(); i++) {
        CString CurrentParam = CmdLine[i];
        if (CurrentParam.Left(10) == _T("/language=")) {
            CString shortLanguageName = CurrentParam.Right(CurrentParam.GetLength() - 10);
            CString foundName = Lang.getLanguageFileNameForLocale(shortLanguageName);
            if (!foundName.IsEmpty()) {
                Settings.Language = foundName;
            }
        }
    }
    if (isFirstRun) {
        CLangSelect LS;
        if (LS.DoModal(nullptr) == IDCANCEL) {
            //*DlgCreationResult = 1;
            return 0;
        }
        Settings.Language = LS.Language;
        Lang.LoadLanguage(Settings.Language);
        Settings.SaveSettings();
    } else {
        Lang.LoadLanguage(Settings.Language);
    }
    AppParams::instance()->setLanguageFile(W2U(Lang.getCurrentLanguageFile()));

    if (Lang.isRTL()) {
        SetProcessDefaultLayout(LAYOUT_RTL);
    }

    dlgMain.setIsFirstRun(isFirstRun);
    if ( dlgMain.Create( 0, reinterpret_cast<LPARAM>(&DlgCreationResult) ) == NULL ) {
            ATLTRACE( _T("Main dialog creation failed!  :( sorry\n") );
            dlgMain.m_hWnd = 0;
            return 0;
    }
    if(DlgCreationResult != 0) {
        dlgMain.DestroyWindow();
        dlgMain.m_hWnd = 0;
        return 0;
    }
    
    if(bCreateFloatingWindow) {
        floatWnd.CreateTrayIcon();
    }    
    if (/* ( CmdLine.GetCount() > 1 && CmdLine.IsOption( _T("quickshot") )*/  
            CmdLine.IsOption( _T("mediainfo") ) || !ShowMainWindow || !dlgMain.m_bShowWindow ) {
        dlgMain.ShowWindow(SW_HIDE);
    } else {
        dlgMain.ShowWindow(nCmdShow);
    }
     
    int nRet = theLoop.Run();
    if ( dlgMain.m_hWnd ) {
        dlgMain.DestroyWindow();
    }
    _Module.RemoveMessageLoop();

    // Remove temporary files
    IuCommonFunctions::ClearTempFolder( tempFolder ); 
    std::vector<CString> folders;
    WinUtils::GetFolderFileList( folders, commonTempFolder, "iu_temp_*" );
    for ( const auto& folder : folders) {
        // Extract Proccess ID from temp folder name
        CString pidStr = folder;
        pidStr.Replace( _T("iu_temp_"), _T("") );
        unsigned long pid =  wcstoul( pidStr, 0, 16 ) ^  0xa1234568;
        if ( pid && !WinUtils::IsProcessRunning( pid ) )
            IuCommonFunctions::ClearTempFolder(commonTempFolder + _T("\\") + folder);
    }
    
    // deletes empty temp directory
    RemoveDirectory( commonTempFolder );
    
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{    
    // Create and install global locale
    std::locale::global(boost::locale::generator().generate(""));
    // Make boost.filesystem use it
    boost::filesystem::path::imbue(std::locale());

#if defined(_WIN32) && !defined(NDEBUG)
    // These global strings in GLOG are initially reserved with a small
    // amount of storage space (16 bytes). Resizing the string larger than its
    // initial size, after the _CrtMemCheckpoint call, can be reported as
    // a memory leak.
    // So for 'debug builds', where memory leak checking is performed,
    // reserve a large enough space so the string will not be resized later.
    // For these variables, _MAX_PATH should be fine.
    FLAGS_log_dir.reserve(_MAX_PATH);  // comment out this line to trigger false memory leak
    FLAGS_log_link.reserve(_MAX_PATH);

    // Enable memory dump from within VS.

#endif
    FLAGS_logtostderr = true;

    AppParams::AppVersionInfo appVersion;
    appVersion.FullVersion = IU_APP_VER;
    appVersion.FullVersionClean = IU_APP_VER_CLEAN;
    appVersion.Build = std::stoi(IU_BUILD_NUMBER);
    appVersion.BuildDate = IU_BUILD_DATE;
    appVersion.CommitHash = IU_COMMIT_HASH;
    appVersion.CommitHashShort = IU_COMMIT_HASH_SHORT;
    appVersion.BranchName = IU_BRANCH_NAME;
    AppParams::instance()->setVersionInfo(appVersion);

    ServiceLocator* serviceLocator = ServiceLocator::instance();
    ServiceLocator::instance()->setSettings(&Settings);
    LogWindow.Create(nullptr);
    serviceLocator->setLogWindow(&LogWindow);
    DefaultLogger defaultLogger;
    LogWindow.setLogger(&defaultLogger);
    DefaultUploadErrorHandler uploadErrorHandler(&defaultLogger);
   
    google::InitGoogleLogging(WCstringToUtf8(WinUtils::GetAppFileName()).c_str());
    
    MyLogSink logSink(&defaultLogger);
    google::AddLogSink(&logSink);

    
    serviceLocator->setUploadErrorHandler(&uploadErrorHandler);
    serviceLocator->setLogger(&defaultLogger);

    WtlScriptDialogProvider dialogProvider;
    serviceLocator->setDialogProvider(&dialogProvider);
    serviceLocator->setTranslator(&Lang);

    OleInitialize(NULL);
    HRESULT hRes ;
    for (const auto& CurrentParam: CmdLine) {
        if ( CurrentParam.Left(12) == _T("/waitforpid=") )    {
            CString pidStr = CurrentParam.Right( CurrentParam.GetLength() - 12 );
            DWORD pid = _ttoi( pidStr );
            HANDLE hProcess = OpenProcess( SYNCHRONIZE, false, pid ); 
            WaitForSingleObject( hProcess, 20000 );
            CloseHandle(hProcess);

            // Workaround for version prior to 1.1.7
            if (!CmdLine.IsOption(_T("update")) && !CmdLine.IsOption(L"afterupdate")) {
                Settings.FindDataFolder();
                if ( !WinUtils::IsDirectory( Settings.DataFolder + "Thumbnails\\") || !WinUtils::IsDirectory( WinUtils::GetAppFolder() + "Docs\\") ) {
                    SimpleXml xml;
                    std::string updateFile = WCstringToUtf8(Settings.DataFolder + "Update\\iu_core.xml");
                    if ( xml.LoadFromFile(updateFile) ) {
                        SimpleXmlNode root = xml.getRoot("UpdateInfo", false);
                        if ( !root.IsNull() ) {
                            int64_t timestamp = root.AttributeInt64("TimeStamp");
                            if ( timestamp >= 0  ) {
                                root.SetAttribute("TimeStamp", timestamp-1);
                                xml.SaveToFile(updateFile);
                                CmdLine.AddParam(L"/update");
                            }
                        }
                    }
                }
            }
        } else if ( CurrentParam == "/debuglog") {
            FLAGS_logtostderr = false;
            FLAGS_alsologtostderr = true;
            LogWindow.Show();
        }
    }

    // for Windows Vista and later versions
    if ( CmdLine.IsOption( _T("integration") ) )  {
        Settings.LoadSettings("","",false);        
        Settings.ApplyRegSettingsRightNow();
        CScriptUploadEngine::DestroyScriptEngine();
        return 0;
    }

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc( NULL, 0, 0, 0L );

    AtlInitCommonControls( ICC_BAR_CLASSES | ICC_USEREX_CLASSES  );    // add flags to support other controls

    hRes = _Module.Init( NULL, hInstance );
    ATLASSERT( SUCCEEDED( hRes ) );

    int nRet = Run( lpstrCmdLine, nCmdShow, &defaultLogger);
    _Module.Term();
    //iuPluginManager.UnloadPlugins();
    CScriptUploadEngine::DestroyScriptEngine();
    OleUninitialize();
    google::RemoveLogSink(&logSink);
    google::ShutdownGoogleLogging();
    LogWindow.DestroyWindow();
    return nRet;
}
