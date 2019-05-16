// ServerListTool.cpp : main source file for ServerListTool.exe
//

#include "MainDlg.h"

#include <boost/locale/generator.hpp>
#include <boost/filesystem/path.hpp>

#include "atlheaders.h"
#include "3rdpart/GdiPlusH.h"
#include "Func/IuCommonFunctions.h"
#include "Core/Logging.h"
#include "Core/Logging/MyLogSink.h"
#include "Func/WinUtils.h"
#include "Func/DefaultLogger.h"
#include "Func/DefaultUploadErrorHandler.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Core/ServiceLocator.h"
#include "Func/WtlScriptDialogProvider.h"
#include "Core/AppParams.h"
#include "Core/Scripting/ScriptsManager.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/Network/NetworkClientFactory.h"
#include "Func/langclass.h"
#include "Core/Settings/CliSettings.h"

CAppModule _Module;

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

using namespace ServersListTool;
CliSettings Settings;
CLogWindow LogWindow;
CLang Lang;
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
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
    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = true;

    DefaultLogger defaultLogger;
    DefaultUploadErrorHandler uploadErrorHandler(&defaultLogger);

    google::InitGoogleLogging(W2U(WinUtils::GetAppFileName()).c_str());
    LogWindow.Create(0);
    ServiceLocator* serviceLocator = ServiceLocator::instance();
    serviceLocator->setLogWindow(&LogWindow);
    serviceLocator->setSettings(&Settings);
    serviceLocator->setUploadErrorHandler(&uploadErrorHandler);
    serviceLocator->setLogger(&defaultLogger);
    MyLogSink logSink(&defaultLogger);
    google::AddLogSink(&logSink);
    WtlScriptDialogProvider dialogProvider;
    serviceLocator->setDialogProvider(&dialogProvider);
    serviceLocator->setTranslator(&Lang);
    CMyEngineList engineList;
    serviceLocator->setMyEngineList(&engineList);
    serviceLocator->setEngineList(&engineList);
    CString serversFileName = WinUtils::GetAppFolder() + "Data/" + _T("servers.xml");
    if (!engineList.loadFromFile(serversFileName)) {
        MessageBox(0, _T("Cannot load server list!"), 0, 0);
        return false;
    }

    Settings.LoadSettings(W2U(WinUtils::GetAppFolder())+ "Data/", "settings.xml");
   
    Settings.setEngineList(&engineList);

    auto appParams = AppParams::instance();
    appParams->setIsGui(true);
    appParams->setDataDirectory(W2U(WinUtils::GetAppFolder() + "Data/"));
    auto networkClientFactory = std::make_shared<NetworkClientFactory>();
    ScriptsManager scriptsManager(networkClientFactory);
    UploadEngineManager uploadEngineManager(&engineList, &uploadErrorHandler, networkClientFactory);
    uploadEngineManager.setScriptsDirectory(WCstringToUtf8(IuCommonFunctions::GetDataFolder() + _T("\\Scripts\\")));
    UploadManager uploadManager(&uploadEngineManager, &engineList, &scriptsManager, &uploadErrorHandler, networkClientFactory);

    CString commonTempFolder, tempFolder;
    IuCommonFunctions::CreateTempFolder(commonTempFolder, tempFolder);
    appParams->setTempDirectory(W2U(tempFolder));
    HRESULT hRes = ::CoInitialize(NULL);
    // If you are running on NT 4.0 or higher you can use the following call instead to
    // make the EXE free threaded. This means that calls come in on a random RPC thread.
    //    HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
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
        CMainDlg dlgMain(&uploadEngineManager, &uploadManager, &engineList, networkClientFactory);
        nRet = dlgMain.DoModal(0);
    }

    _Module.Term();
    ::CoUninitialize();
    IuCommonFunctions::ClearTempFolder(tempFolder);
    google::ShutdownGoogleLogging();
    LogWindow.DestroyWindow();
    return nRet;
}
