// ServerListTool.cpp : main source file for ServerListTool.exe
//

#include "MainDlg.h"

#include <boost/locale/generator.hpp>
#include <boost/filesystem/path.hpp>
#include <Core/Images/GdiPlusImage.h>


#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"
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
#include "Func/LangClass.h"
#include "Core/Settings/CliSettings.h"
#include "versioninfo.h"

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

class ServersCheckerApplication {
    CLogWindow logWindow_;
    CLang lang_;
    ServersCheckerSettings settings_;
    std::unique_ptr<MyLogSink> myLogSink_;
    std::unique_ptr<ScriptsManager> scriptsManager_;
    std::unique_ptr<UploadEngineManager> uploadEngineManager_;
    std::unique_ptr<UploadManager> uploadManager_;
    std::shared_ptr<DefaultLogger> logger_;
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
    std::unique_ptr<CMyEngineList> engineList_;
    std::shared_ptr<WtlScriptDialogProvider> scriptDialogProvider_;
    std::unique_ptr<TaskDispatcher> taskDispatcher_;

    CString commonTempFolder_, tempFolder_;
public:
    ServersCheckerApplication() {
        auto appParams = AppParams::instance();
        appParams->setIsGui(true);
        appParams->setDataDirectory(W2U(WinUtils::GetAppFolder() + "Data/"));

        IuCommonFunctions::CreateTempFolder(commonTempFolder_, tempFolder_);
        appParams->setTempDirectory(W2U(tempFolder_));

        setAppVersion();
        initBasicServices();
    }

    ~ServersCheckerApplication() {
        logWindow_.DestroyWindow();
        IuCommonFunctions::ClearTempFolder(tempFolder_);
    }

    void setAppVersion() const {
        AppParams::AppVersionInfo appVersion;
        appVersion.FullVersion = IU_APP_VER;
        appVersion.FullVersionClean = IU_APP_VER_CLEAN;
        appVersion.Build = std::stoi(IU_BUILD_NUMBER);
        appVersion.BuildDate = IU_BUILD_DATE;
        appVersion.CommitHash = IU_COMMIT_HASH;
        appVersion.CommitHashShort = IU_COMMIT_HASH_SHORT;
        appVersion.BranchName = IU_BRANCH_NAME;
        AppParams::instance()->setVersionInfo(appVersion);
    }

    void initBasicServices() {
        ServiceLocator* serviceLocator = ServiceLocator::instance();
        logger_ = std::make_shared<DefaultLogger>();
        myLogSink_ = std::make_unique<MyLogSink>(logger_.get());
        google::AddLogSink(myLogSink_.get());
        serviceLocator->setSettings(&settings_);
        serviceLocator->setNetworkClientFactory(std::make_shared<NetworkClientFactory>());
        logWindow_.Create(nullptr);
        logWindow_.setLogger(logger_.get());

        serviceLocator->setLogWindow(&logWindow_);
        serviceLocator->setLogger(logger_);
        serviceLocator->setTranslator(&lang_);
        AbstractImage::autoRegisterFactory<void>();
    }

    void initServices() {
        ServiceLocator* serviceLocator = ServiceLocator::instance();
        taskDispatcher_ = std::make_unique<TaskDispatcher>(3);
        serviceLocator->setTaskDispatcher(taskDispatcher_.get());

        auto uploadErrorHandler = std::make_shared<DefaultUploadErrorHandler>(logger_);
        serviceLocator->setUploadErrorHandler(uploadErrorHandler);
        networkClientFactory_ = std::make_shared<NetworkClientFactory>();
        scriptsManager_ = std::make_unique<ScriptsManager>(networkClientFactory_);
        engineList_ = std::make_unique<CMyEngineList>();
        uploadEngineManager_ = std::make_unique<UploadEngineManager>(engineList_.get(), uploadErrorHandler, networkClientFactory_);
        uploadEngineManager_->setScriptsDirectory(WCstringToUtf8(IuCommonFunctions::GetDataFolder() + _T("\\Scripts\\")));
        uploadManager_ = std::make_unique<UploadManager>(uploadEngineManager_.get(), engineList_.get(), scriptsManager_.get(), uploadErrorHandler, networkClientFactory_, 5);
        serviceLocator->setUploadManager(uploadManager_.get());
        scriptDialogProvider_ = std::make_shared<WtlScriptDialogProvider>();
        serviceLocator->setDialogProvider(scriptDialogProvider_.get());
        serviceLocator->setMyEngineList(engineList_.get());
        serviceLocator->setEngineList(engineList_.get());
    }

    int Run(LPTSTR lpstrCmdLine, int nCmdShow)
    {
        initServices();
        CString serversFileName = WinUtils::GetAppFolder() + "Data/" + _T("servers.xml");
        if (!engineList_->loadFromFile(serversFileName)) {
            MessageBox(0, _T("Cannot load server list!"), 0, 0);
            return false;
        }

        settings_.LoadSettings(W2U(WinUtils::GetAppFolder()), "ServersChecker.xml");

        settings_.setEngineList(engineList_.get());

        CMainDlg dlgMain(&settings_, uploadEngineManager_.get(), uploadManager_.get(), engineList_.get(), networkClientFactory_);
        int nret = dlgMain.DoModal(0);
        return nret;
    }
};
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
    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = true;

    google::InitGoogleLogging(W2U(WinUtils::GetAppFileName()).c_str());

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
        ServersCheckerApplication app;
        nRet = app.Run(lpstrCmdLine, nCmdShow); 
    }

    _Module.Term();
    ::CoUninitialize();
    
    google::ShutdownGoogleLogging();
    return nRet;
}
