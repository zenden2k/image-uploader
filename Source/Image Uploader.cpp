/*
     Image Uploader - program for uploading images/files to the Internet

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

#include "atlheaders.h" 
#include <boost/filesystem/path.hpp>
#include <boost/locale.hpp>
 
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Gui/Dialogs/FloatingWindow.h"
#include "Func/CmdLine.h"
#include "Func/WinUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Core/Logging.h"
#include "Core/Logging/MyLogSink.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Core/ServiceLocator.h"
#include "Func/DefaultUploadErrorHandler.h"
#include "Func/DefaultLogger.h"
#include "Func/WtlScriptDialogProvider.h"
#include "Core/AppParams.h"
#include "Func/LangClass.h"
#include "Func/GdiPlusInitializer.h"
#include "Gui/Dialogs/LangSelect.h"
#include "versioninfo.h"
#include "Core/Network/NetworkClientFactory.h"
#include "Core/Scripting/ScriptsManager.h"
#include "Core/Upload/Filters/UserFilter.h"
#include "Core/Upload/Filters/ImageConverterFilter.h"
#include "Core/Upload/Filters/SizeExceedFilter.h"
#include "Core/Upload/Filters/UrlShorteningFilter.h"
#include "Core/Upload/Filters/ImageSearchFilter.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/3rdpart/xdgmime/xdgmime.h"
#include "Gui/Helpers/LangHelper.h"

#ifndef NDEBUG
//#include <vld.h>
#endif

CAppModule _Module;

class Application {
    CLogWindow logWindow_;
    WtlGuiSettings settings_;
    CLang lang_;
    std::shared_ptr<CFloatingWindow> floatWnd_;
    std::unique_ptr<ScriptsManager> scriptsManager_;
    std::shared_ptr<DefaultLogger> logger_;
    std::unique_ptr<MyLogSink> myLogSink_;
    std::unique_ptr<UploadEngineManager> uploadEngineManager_;
    std::unique_ptr<UploadManager> uploadManager_;
    std::unique_ptr<CMyEngineList> engineList_;
    std::unique_ptr<ImageConverterFilter> imageConverterFilter_;
    std::unique_ptr<SizeExceedFilter> sizeExceedFilter_;
    std::shared_ptr<UrlShorteningFilter> urlShorteningFilter_;
    std::shared_ptr<ImageSearchFilter> imageSearchFilter_;
    std::unique_ptr<UserFilter> userFilter_;
    std::shared_ptr<WtlScriptDialogProvider> scriptDialogProvider_;
    std::unique_ptr<TaskDispatcher> taskDispatcher_;
    CString commonTempFolder_, tempFolder_;
public:
    Application() {
        srand(static_cast<unsigned>(time(nullptr)));
        setAppVersion();
        initBasicServices();
    }

    ~Application() {
        CScriptUploadEngine::DestroyScriptEngine();
        //ServiceLocator::instance()->setUploadManager(nullptr);
        logWindow_.DestroyWindow();
        // Remove temporary files
        IuCommonFunctions::ClearTempFolder(tempFolder_);
        std::vector<CString> folders;
        WinUtils::GetFolderFileList(folders, commonTempFolder_, "iu_temp_*");
        for (const auto& folder : folders) {
            // Extract process ID from temp folder name
            CString pidStr = folder;
            pidStr.Replace(_T("iu_temp_"), _T(""));
            unsigned long pid = wcstoul(pidStr, 0, 16) ^ 0xa1234568;
            if (pid && !WinUtils::IsProcessRunning(pid))
                IuCommonFunctions::ClearTempFolder(commonTempFolder_ + _T("\\") + folder);
        }

        // deletes empty temp directory
        RemoveDirectory(commonTempFolder_);
    }

    static void setAppVersion() {
        AppParams::AppVersionInfo appVersion;
        appVersion.FullVersion = IU_APP_VER;
        appVersion.FullVersionClean = IU_APP_VER_CLEAN;
        appVersion.Build = atoi(IU_BUILD_NUMBER);
        appVersion.BuildDate = IU_BUILD_DATE;
        appVersion.CommitHash = IU_COMMIT_HASH;
        appVersion.CommitHashShort = IU_COMMIT_HASH_SHORT;
        appVersion.BranchName = IU_BRANCH_NAME;
        AppParams::instance()->setVersionInfo(appVersion);
    }

    void initBasicServices() {
        AbstractImage::autoRegisterFactory<void>();
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
    }

    void initServices() {
        CString dataFolder = settings_.DataFolder;
        if (dataFolder.Right(1) == "\\") {
            dataFolder.Truncate(dataFolder.GetLength() - 1);
        }
        std::string dir = WinUtils::wstostr(dataFolder.GetString(), CP_ACP);
        char* cacheDir = strdup(dir.c_str());
        if (cacheDir) {
            const char* dirs[2]
                = { cacheDir, nullptr };
            xdg_mime_set_dirs(dirs);
            free(cacheDir);
        }

        ServiceLocator* serviceLocator = ServiceLocator::instance();
        taskDispatcher_ = std::make_unique<TaskDispatcher>(3);
        serviceLocator->setTaskDispatcher(taskDispatcher_.get());

        auto uploadErrorHandler = std::make_shared<DefaultUploadErrorHandler>(logger_);
        serviceLocator->setUploadErrorHandler(uploadErrorHandler);
        scriptDialogProvider_ = std::make_shared<WtlScriptDialogProvider>();
        serviceLocator->setDialogProvider(scriptDialogProvider_.get());
        serviceLocator->setTranslator(&lang_);
        scriptsManager_ = std::make_unique<ScriptsManager>(serviceLocator->networkClientFactory());
        engineList_ = std::make_unique<CMyEngineList>();
        serviceLocator->setEngineList(engineList_.get());
        serviceLocator->setMyEngineList(engineList_.get());
        settings_.setEngineList(engineList_.get());
        uploadEngineManager_ = std::make_unique<UploadEngineManager>(engineList_.get(), uploadErrorHandler, serviceLocator->networkClientFactory());
        uploadManager_ = std::make_unique<UploadManager>(uploadEngineManager_.get(), engineList_.get(), scriptsManager_.get(),
            uploadErrorHandler, serviceLocator->networkClientFactory(), settings_.MaxThreads);
        serviceLocator->setUploadManager(uploadManager_.get());

        imageConverterFilter_ = std::make_unique<ImageConverterFilter>();
        sizeExceedFilter_ = std::make_unique<SizeExceedFilter>(engineList_.get(), uploadEngineManager_.get());
        urlShorteningFilter_ = std::make_shared<UrlShorteningFilter>();
        imageSearchFilter_ = std::make_shared<ImageSearchFilter>();
        userFilter_ = std::make_unique<UserFilter>(scriptsManager_.get());
        
        uploadManager_->addUploadFilter(imageConverterFilter_.get());
        uploadManager_->addUploadFilter(userFilter_.get());
        uploadManager_->addUploadFilter(sizeExceedFilter_.get());
        uploadManager_->addUploadFilter(urlShorteningFilter_.get());
        uploadManager_->addUploadFilter(imageSearchFilter_.get());

        serviceLocator->setUrlShorteningFilter(urlShorteningFilter_);
    }

    int Run(LPTSTR lpstrCmdLine, int nCmdShow)
    {
        for (const auto& CurrentParam : CmdLine) {
            if (CurrentParam.Left(12) == _T("/waitforpid=")) {
                CString pidStr = CurrentParam.Right(CurrentParam.GetLength() - 12);
                DWORD pid = _ttoi(pidStr);
                HANDLE hProcess = OpenProcess(SYNCHRONIZE, false, pid);
                WaitForSingleObject(hProcess, 20000);
                CloseHandle(hProcess);

                // Workaround for version prior to 1.1.7
                if (!CmdLine.IsOption(_T("update")) && !CmdLine.IsOption(L"afterupdate")) {
                    settings_.FindDataFolder();
                    if (!WinUtils::IsDirectory(settings_.DataFolder + "Thumbnails\\") || !WinUtils::IsDirectory(WinUtils::GetAppFolder() + "Docs\\")) {
                        SimpleXml xml;
                        std::string updateFile = WCstringToUtf8(settings_.DataFolder + "Update\\iu_core.xml");
                        if (xml.LoadFromFile(updateFile)) {
                            SimpleXmlNode root = xml.getRoot("UpdateInfo", false);
                            if (!root.IsNull()) {
                                int64_t timestamp = root.AttributeInt64("TimeStamp");
                                if (timestamp >= 0) {
                                    root.SetAttribute("TimeStamp", timestamp - 1);
                                    xml.SaveToFile(updateFile);
                                    CmdLine.AddParam(L"/update");
                                }
                            }
                        }
                    }
                }
            }
            else if (CurrentParam == "/debuglog") {
                FLAGS_logtostderr = false;
                FLAGS_alsologtostderr = true;
                logWindow_.Show();
            }
        }

        // for Windows Vista and later versions
        if (CmdLine.IsOption(_T("integration"))) {
            settings_.LoadSettings("", "", false);
            lang_.LoadLanguage(settings_.Language);
            settings_.ApplyRegSettingsRightNow();
            CScriptUploadEngine::DestroyScriptEngine();
            return 0;
        }

        IuCommonFunctions::CreateTempFolder(commonTempFolder_, tempFolder_);

        AppParams::instance()->setTempDirectory(W2U(tempFolder_));
        std::vector<CString> fileList;
        WinUtils::GetFolderFileList(fileList, WinUtils::GetAppFolder() + _T("\\"), _T("*.old"));
        for (const auto& file : fileList) {
            DeleteFile(WinUtils::GetAppFolder() + file);
        }

        settings_.LoadSettings();

        GdiPlusInitializer gdiPlusInitializer;
        initServices();

        CMessageLoop theLoop;
        _Module.AddMessageLoop(&theLoop);
        
        CWizardDlg  dlgMain(logger_, engineList_.get(), uploadEngineManager_.get(), uploadManager_.get(), scriptsManager_.get(), &settings_);
        auto* serviceLocator = ServiceLocator::instance();
        serviceLocator->setProgramWindow(&dlgMain);
        serviceLocator->setTaskRunner(&dlgMain);

        floatWnd_ = std::make_shared<CFloatingWindow>(&dlgMain, uploadManager_.get(), uploadEngineManager_.get());
        dlgMain.setFloatWnd(floatWnd_);
        settings_.setFloatWnd(floatWnd_.get());


        DWORD DlgCreationResult = 0;
        bool ShowMainWindow = true;

        if (CmdLine.IsOption(_T("uninstall"))) {
            settings_.Uninstall();
            return 0;
        }

        bool BecomeTray = false;
        if (settings_.ShowTrayIcon && !CmdLine.IsOption(_T("tray"))) {
            if (!CFloatingWindow::IsRunningFloatingWnd()) {
                BecomeTray = true;
                CmdLine.AddParam(_T("/tray"));
            }
        }

        bool bCreateFloatingWindow = false;

        if (CmdLine.IsOption(_T("tray")) || BecomeTray) {
            if (!CFloatingWindow::IsRunningFloatingWnd()) {
                bCreateFloatingWindow = true;
                ShowMainWindow = BecomeTray;
            }
            else {
                return 0;
            }
        }

        lang_.SetDirectory(WinUtils::GetAppFolder() + "Lang\\");
        bool isFirstRun = settings_.Language.IsEmpty() || FALSE;
        for (size_t i = 0; i < CmdLine.GetCount(); i++) {
            CString CurrentParam = CmdLine[i];
            if (CurrentParam.Left(10) == _T("/language=")) {
                CString shortLanguageName = CurrentParam.Right(CurrentParam.GetLength() - 10);
                auto languageList{ LangHelper::instance()->getLanguageList((WinUtils::GetAppFolder() + "Lang").GetString()) };

                auto it = languageList.find(W2U(shortLanguageName));

                if (it != languageList.end()) {
                    settings_.Language = U2W(it->first);
                }
                
                
                /*CString foundName = lang_.getLanguageFileNameForLocale(shortLanguageName);
                if (!foundName.IsEmpty()) {
                    settings_.Language = foundName;
                }*/
            }
        }

        if (isFirstRun) {
            CLangSelect LS;
            if (LS.DoModal(nullptr) == IDCANCEL) {
                //*DlgCreationResult = 1;
                return 0;
            }
            settings_.Language = LS.getLanguage();
            
            lang_.LoadLanguage(settings_.Language);
            settings_.SaveSettings();
        }
        else {
            std::map<CString, CString> oldLangs = {
            {_T("Arabic"), _T("ar")},
            {_T("Farsi"), _T("fa")},
            {_T("Hrvatski"), _T("hr")},
            {_T("Hungarian"), _T("hu")},
            {_T("Romanian"), _T("ro")},
            {_T("Russian"), _T("ru")},
            {_T("Serbian"), _T("sr")},
            {_T("Swedish"), _T("sv")},
            {_T("Turkish"), _T("tr")},
            {_T("Ukrainian"), _T("uk")},
            };
            auto it = oldLangs.find(settings_.Language);
            if (it != oldLangs.end()) {
                settings_.Language = it->second;
            }
            lang_.LoadLanguage(settings_.Language);
        }

        //AppParams::instance()->setLanguageFile(W2U(lang_.getCurrentLanguageFile()));

        if (lang_.isRTL()) {
            SetProcessDefaultLayout(LAYOUT_RTL);
        }

        dlgMain.setIsFirstRun(isFirstRun);
        if (dlgMain.Create(0, reinterpret_cast<LPARAM>(&DlgCreationResult)) == NULL) {
            ATLTRACE(_T("Main dialog creation failed!  :( sorry\n"));
            dlgMain.m_hWnd = 0;
            return 0;
        }
        if (DlgCreationResult != 0) {
            dlgMain.DestroyWindow();
            dlgMain.m_hWnd = 0;
            return 0;
        }

        if (bCreateFloatingWindow) {
            floatWnd_->CreateTrayIcon();
        }
        if (/* ( CmdLine.GetCount() > 1 && CmdLine.IsOption( _T("quickshot") )*/
            CmdLine.IsOption(_T("mediainfo")) || !ShowMainWindow || !dlgMain.isShowWindowSet()) {
            dlgMain.ShowWindow(SW_HIDE);
        }
        else {
            dlgMain.ShowWindow(nCmdShow);
        }

        int nRet = theLoop.Run();
        if (dlgMain.m_hWnd) {
            dlgMain.DestroyWindow();
        }
        _Module.RemoveMessageLoop();

        return nRet;
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
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(WCstringToUtf8(WinUtils::GetAppFileName()).c_str());
    OleInitialize(NULL);

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc( NULL, 0, 0, 0L );

    AtlInitCommonControls( ICC_BAR_CLASSES | ICC_USEREX_CLASSES  );    // add flags to support other controls

    HRESULT hRes = _Module.Init( NULL, hInstance );
    int nRet;
    ATLASSERT( SUCCEEDED( hRes ) );
    {
        Application app;
        nRet = app.Run(lpstrCmdLine, nCmdShow);
    }
    _Module.Term();

    
    OleUninitialize();
    //google::RemoveLogSink(&logSink);
    google::ShutdownGoogleLogging();
    return nRet;
}
