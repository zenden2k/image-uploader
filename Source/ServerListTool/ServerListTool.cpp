// ServerListTool.cpp : main source file for ServerListTool.exe
//

#include "resource.h"
#include "MainDlg.h"
#include <Gdiplus.h>
#include "Func/Common.h"
#include <GdiPlus.h>
#include <Func/IuCommonFunctions.h>
#include <Core/Logging.h>
#include <Func/MyLogSink.h>
#include <Func/WinUtils.h>
CAppModule _Module;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	FLAGS_logtostderr = true;
	//google::SetLogDestination(google::GLOG_INFO,"d:/" );

	google::InitGoogleLogging(WCstringToUtf8(WinUtils::GetAppFileName()).c_str());
	MyLogSink logSink;
	google::AddLogSink(&logSink);

	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);   // add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	int nRet = 0;
	// BLOCK: Run application
	{
		CMainDlg dlgMain;
		nRet = dlgMain.DoModal();
	}

	_Module.Term();
	SquirrelVM::Shutdown();
	::CoUninitialize();
	IuCommonFunctions::ClearTempFolder(IuCommonFunctions::IUTempFolder);
	return nRet;
}
