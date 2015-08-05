/*
     Image Uploader - program for uploading images/files to the Internet

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

 
#include "atlheaders.h"
#include <shellapi.h> 
#include "resource.h"
#include "3rdpart/GdiplusH.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/wizarddlg.h"
#include "Gui/Dialogs/floatingwindow.h"
#include "Common/CmdLine.h"
#include "Core/Settings.h"
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

#ifndef NDEBUG
#include <vld.h>
#endif
CAppModule _Module;

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
        
    /*try {
        boost::filesystem::ofstream hello("test.txt");
    }
    catch (std::exception& ex)
    {
        LOG(ERROR) << ex.what();
    }*/
    IuCommonFunctions::CreateTempFolder();
    
    AppParams::instance()->setTempDirectory(W2U(IuCommonFunctions::IUTempFolder));
    std::vector<CString> fileList;
    WinUtils::GetFolderFileList( fileList, WinUtils::GetAppFolder() + _T("\\"), _T("*.old") );
    for ( size_t i=0; i<fileList.size(); i++ ) {
        DeleteFile( WinUtils::GetAppFolder() + fileList[i] );
    }

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

    CMessageLoop theLoop;
    _Module.AddMessageLoop( &theLoop );
    CWizardDlg  dlgMain;
    
    DWORD DlgCreationResult = 0;
    bool ShowMainWindow     = true;
    Settings.LoadSettings();

    if ( CmdLine.IsOption( _T("uninstall") ) ) {
        Settings.Uninstall();
        return 0;
    }

    bool BecomeTray = false;
    if ( Settings.ShowTrayIcon && !CmdLine.IsOption( _T("tray") ) ) {
        if ( !IsRunningFloatingWnd() ) {
            BecomeTray = true;
            CmdLine.AddParam( _T("/tray") );
        }    
    }

    bool bCreateFloatingWindow = false;

    if ( CmdLine.IsOption( _T("tray") ) || BecomeTray ) {
        if ( !IsRunningFloatingWnd() ) {
            bCreateFloatingWindow = true;
            ShowMainWindow        = BecomeTray;    
        } else {
            return 0;
        }
    }

    ServiceLocator::instance()->setProgramWindow(&dlgMain);
    floatWnd.setWizardDlg(&dlgMain);

    if ( dlgMain.Create( 0, reinterpret_cast<LPARAM>(&DlgCreationResult) ) == NULL ) {
            ATLTRACE( _T("Main dialog creation failed!  :( sorry\n") );
            dlgMain.m_hWnd = 0;
            return 0;
    }
    if(DlgCreationResult != 0) {
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

    Gdiplus::GdiplusShutdown( gdiplusToken );

    // Remove temporary files
    IuCommonFunctions::ClearTempFolder( IuCommonFunctions::IUTempFolder ); 
    std::vector<CString> folders;
    WinUtils::GetFolderFileList( folders, IuCommonFunctions::IUCommonTempFolder, "iu_temp_*" );
    for ( size_t i=0; i < folders.size(); i++ ) {
        // Extract Proccess ID from temp folder name
        CString pidStr = folders[i]; 
        pidStr.Replace( _T("iu_temp_"), _T("") );
        unsigned long pid =  wcstoul( pidStr, 0, 16 ) ^  0xa1234568;
        if ( pid && !WinUtils::IsProcessRunning( pid ) )
            IuCommonFunctions::ClearTempFolder( IuCommonFunctions::IUCommonTempFolder + _T("\\") + folders[i] );
    }
    
    // deletes empty temp directory
    RemoveDirectory( IuCommonFunctions::IUCommonTempFolder );
    LogWindow.DestroyWindow();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{    
    // Create and install global locale
    std::locale::global(boost::locale::generator().generate(""));
    // Make boost.filesystem use it
    boost::filesystem::path::imbue(std::locale());
    //BOOL res = SetProcessDefaultLayout(LAYOUT_RTL);

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
    //google::SetLogDestination(google::GLOG_INFO,"d:/" );
    DefaultLogger defaultLogger;
    DefaultUploadErrorHandler uploadErrorHandler(&defaultLogger);
   
    google::InitGoogleLogging(WCstringToUtf8(WinUtils::GetAppFileName()).c_str());
    LogWindow.Create(0);
    MyLogSink logSink(&defaultLogger);
    google::AddLogSink(&logSink);

    ServiceLocator* serviceLocator = ServiceLocator::instance();
    serviceLocator->setUploadErrorHandler(&uploadErrorHandler);
    serviceLocator->setLogger(&defaultLogger);

    WtlScriptDialogProvider dialogProvider;
    serviceLocator->setDialogProvider(&dialogProvider);
    serviceLocator->setTranslator(&Lang);

    OleInitialize(NULL);
    HRESULT hRes ;
    for( size_t i = 0; i < CmdLine.GetCount(); i++ ) {
        CString CurrentParam = CmdLine[i];
        if ( CurrentParam.Left(12) == _T("/waitforpid=") )    {
            CString pidStr = CurrentParam.Right( CurrentParam.GetLength() - 12 );
            DWORD pid = _ttoi( pidStr );
            HANDLE hProcess = OpenProcess( SYNCHRONIZE, false, pid ); 
            WaitForSingleObject( hProcess, 20000 );

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

    int nRet = Run( lpstrCmdLine, nCmdShow );
    _Module.Term();
    //iuPluginManager.UnloadPlugins();
    CScriptUploadEngine::DestroyScriptEngine();
    OleUninitialize();
    google::RemoveLogSink(&logSink);
    google::ShutdownGoogleLogging();
    
    return nRet;
}
