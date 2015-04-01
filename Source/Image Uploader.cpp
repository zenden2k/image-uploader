/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include <vld.h> 

#include "atlheaders.h"
#include <gdiplus.h>
#include <shellapi.h> 
#include "resource.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/wizarddlg.h"
#include "Gui/Dialogs/floatingwindow.h"
#include "Common/CmdLine.h"
#include "Func/Settings.h"
#include "Func/WinUtils.h"
#include <Func/IuCommonFunctions.h>
#include <Core/Logging.h>
#include <Func/MyLogSink.h>


CAppModule _Module;

bool IsProcessRunning(DWORD pid) {
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	if(!process) {
		return false;
	}
		
	CloseHandle(process);
	return true;
}

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	
	LogWindow.Create(0);
	IuCommonFunctions::CreateTempFolder();
	
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

	pWizardDlg = &dlgMain;
	if ( dlgMain.Create( 0, (LPARAM)&DlgCreationResult ) == NULL ) {
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

	// Удаляем временные файлы
	IuCommonFunctions::ClearTempFolder( IuCommonFunctions::IUTempFolder ); 
	std::vector<CString> folders;
	WinUtils::GetFolderFileList( folders, IuCommonFunctions::IUCommonTempFolder, "iu_temp_*" );
	for ( size_t i=0; i < folders.size(); i++ ) {
		// Extract Proccess ID from temp folder name
		CString pidStr = folders[i]; 
		pidStr.Replace( _T("iu_temp_"), _T("") );
		unsigned int pid =  wcstoul( pidStr, 0, 16 ) ^  0xa1234568;
		if ( pid && !IsProcessRunning( pid ) )
			IuCommonFunctions::ClearTempFolder( IuCommonFunctions::IUCommonTempFolder + _T("\\") + folders[i] );
	}
	
	// deletes empty temp directory
	RemoveDirectory( IuCommonFunctions::IUCommonTempFolder );
	LogWindow.DestroyWindow();
	return 0;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{	
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
	
	google::InitGoogleLogging(WCstringToUtf8(WinUtils::GetAppFileName()).c_str());
	MyLogSink logSink;
	google::AddLogSink(&logSink);

	OleInitialize(NULL);
	HRESULT hRes ;
	for( size_t i = 0; i < CmdLine.GetCount(); i++ ) {
		CString CurrentParam = CmdLine[i];
		if ( CurrentParam.Left(12) == _T("/waitforpid=") )	{
			CString pidStr = CurrentParam.Right( CurrentParam.GetLength() - 12 );
			DWORD pid = _ttoi( pidStr );
			HANDLE hProcess = OpenProcess( SYNCHRONIZE, false, pid ); 
			WaitForSingleObject( hProcess, 20000 );

			// Workaround for version prior to 1.1.7
			if (!CmdLine.IsOption(_T("update")) && !CmdLine.IsOption(L"afterupdate")) {
				Settings.FindDataFolder();
				if ( !WinUtils::IsDirectory( Settings.DataFolder + "Thumbnails\\") ) {
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

	AtlInitCommonControls( ICC_BAR_CLASSES | ICC_USEREX_CLASSES  );	// add flags to support other controls

	hRes = _Module.Init( NULL, hInstance );
	ATLASSERT( SUCCEEDED( hRes ) );

	int nRet = Run( lpstrCmdLine, nCmdShow );
	_Module.Term();
	CScriptUploadEngine::DestroyScriptEngine();
	OleUninitialize();

	google::ShutdownGoogleLogging();
	return 0;
}
