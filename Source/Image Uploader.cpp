/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "atlheaders.h"
#include <gdiplus.h>
#include <shellapi.h> 
#include "resource.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/wizarddlg.h"
#include "Gui/Dialogs/floatingwindow.h"
#include "Common/CmdLine.h"
#include "Func/Settings.h"

CAppModule _Module;

bool IsProcessRunning(DWORD pid)
{
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	if(!process) {
		return false;
	}
		
	CloseHandle(process);
	return true;
}

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CreateTempFolder();
	
	std::vector<CString> fileList;
	GetFolderFileList( fileList, GetAppFolder() + _T("\\"), _T("*.old") );
	for ( size_t i=0; i<fileList.size(); i++ ) {
		DeleteFile( GetAppFolder() + fileList[i] );
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
	LogWindow.Create(0);
	if(bCreateFloatingWindow) {
		floatWnd.CreateTrayIcon();
	}	
	if ( ( CmdLine.GetCount() > 1 && CmdLine.IsOption( _T("quickshot") ) ) 
		    || CmdLine.IsOption( _T("mediainfo") ) || !ShowMainWindow || !dlgMain.m_bShowWindow ) {
		dlgMain.ShowWindow(SW_HIDE);
	} else {
		dlgMain.ShowWindow(nCmdShow);
	}
	
	int nRet = theLoop.Run();
	_Module.RemoveMessageLoop();

	Gdiplus::GdiplusShutdown( gdiplusToken );

	// Удаляем временные файлы
	ClearTempFolder( IUTempFolder ); 
	std::vector<CString> folders;
	GetFolderFileList( folders, IUCommonTempFolder, "iu_temp_*" );
	for ( size_t i=0; i < folders.size(); i++ ) {
		// Extract Proccess ID from temp folder name
		CString pidStr = folders[i]; 
		pidStr.Replace( _T("iu_temp_"), _T("") );
		unsigned int pid =  wcstoul( pidStr, 0, 16 ) ^  0xa1234568;
		if ( pid && !IsProcessRunning( pid ) )
			ClearTempFolder( IUCommonTempFolder + _T("\\") + folders[i] );
	}
	
	// deletes empty temp directory
	RemoveDirectory( IUCommonTempFolder );
	return 0;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{	
	OleInitialize(NULL);
	HRESULT hRes ;

	for( size_t i = 0; i < CmdLine.GetCount(); i++ ) {
		CString CurrentParam = CmdLine[i];
		if ( CurrentParam.Left(12) == _T("/waitforpid=") )	{
			CString pidStr = CurrentParam.Right( CurrentParam.GetLength() - 12 );
			DWORD pid = _ttoi( pidStr );
			HANDLE hProcess = OpenProcess( SYNCHRONIZE, false, pid ); 
			WaitForSingleObject( hProcess, 20000 );
		}
	}

	// for Windows Vista and later versions
	if ( CmdLine.IsOption( _T("integration") ) )  {
		Settings.LoadSettings( 0, false );		
		Settings.ApplyRegSettingsRightNow();
		CScriptUploadEngine::DestroyScriptEngine();
		return 0;
	}

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc( NULL, 0, 0, 0L );

	AtlInitCommonControls( ICC_BAR_CLASSES );	// add flags to support other controls

	hRes = _Module.Init( NULL, hInstance );
	ATLASSERT( SUCCEEDED( hRes ) );

	int nRet = Run( lpstrCmdLine, nCmdShow );
	_Module.Term();
	CScriptUploadEngine::DestroyScriptEngine();
	OleUninitialize();
	return 0;
}
