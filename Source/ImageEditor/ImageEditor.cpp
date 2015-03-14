// ImageEditor.cpp : main source file for ImageEditor.exe
//

#include "ImageEditor/Gui/ImageEditorWindow.h"
#include "ImageEditor/resource.h"
#include "ImageEditor/Gui/ImageEditorView.h"
#include <Core/Logging.h>

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT) {
	FLAGS_logtostderr = false;
	FLAGS_alsologtostderr = true;

	google::InitGoogleLogging("ImageEditor.exe");
	CMessageLoop theLoop;
	_Module.AddMessageLoop( &theLoop );

	ImageEditorWindow wndMain;
	RECT rc = {100,100,1280,800};

	if ( wndMain.Create(0, rc, _T("ImageEditor"), WS_OVERLAPPED | WS_POPUP | WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
		WS_MINIMIZEBOX) == NULL ) {
		ATLTRACE( _T("Main window creation failed!\n") );
		return 0;
	}

	wndMain.ShowWindow( nCmdShow );

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	wndMain.DestroyWindow();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT( SUCCEEDED(hRes) );
	// in WinMain
	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icce);

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc( NULL, 0, 0, 0L );

	AtlInitCommonControls( ICC_BAR_CLASSES );	// add flags to support other controls

	hRes = _Module.Init( NULL, hInstance );
	ATLASSERT( SUCCEEDED(hRes) );

	int nRet = Run( lpstrCmdLine, nCmdShow );

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
