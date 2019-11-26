#include "Gui/mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QDebug>
//#include <3rdparty/qtdotnetstyle.h>
#include "Core/Logging.h"
#include <boost/filesystem/path.hpp>
#include <boost/locale.hpp>
#include "Core/CommonDefs.h"
#include "Core/ServiceLocator.h"
#include "Core/AppParams.h"

#include "QtUploadErrorHandler.h"
#include "QtDefaultLogger.h"
#include "QtScriptDialogProvider.h"
#include "Gui/LogWindow.h"

#include <Core/Settings/QtGuiSettings.h>
#include <QMessageBox>
#include "Core/i18n/Translator.h"
#include "versioninfo.h"

#ifdef _WIN32
CAppModule _Module;
QString dataFolder = "Data/";
#else
QString dataFolder = "/usr/share/imageuploader/";
#endif
QtGuiSettings Settings;

class Translator : public ITranslator {
public:
	std::string getCurrentLanguage() override {
		return "English";
	}
	std::string getCurrentLocale() override {
		return "en_US";
	}
	std::string translate(const char* str) override {
		return str;
	}
#ifdef _WIN32
	const wchar_t* translateW(const wchar_t* str) override {
		return str;
	}
#endif
};
Translator translator; // dummy translator

int main(int argc, char *argv[])
{
    ServiceLocator::instance()->setSettings(&Settings);
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
    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = true;
    // Enable memory dump from within VS.
#else
        FLAGS_logtostderr = true;
#endif

    google::InitGoogleLogging(argv[0]);
    AppParams::AppVersionInfo appVersion;
    appVersion.FullVersion = IU_APP_VER;
    appVersion.FullVersionClean = IU_APP_VER_CLEAN;
    appVersion.Build = std::stoi(IU_BUILD_NUMBER);
    appVersion.BuildDate = IU_BUILD_DATE;
    appVersion.CommitHash = IU_COMMIT_HASH;
    appVersion.CommitHashShort = IU_COMMIT_HASH_SHORT;
    appVersion.BranchName = IU_BRANCH_NAME;
    AppParams::instance()->setVersionInfo(appVersion);

	QApplication a(argc, argv);
    LogWindow logWindow;
	logWindow.show();
	auto logger = std::make_shared<QtDefaultLogger>(&logWindow);
	auto errorHandler = std::make_shared<QtUploadErrorHandler>(logger.get());
	QtScriptDialogProvider dlgProvider;
	ServiceLocator::instance()->setTranslator(&translator);
	ServiceLocator::instance()->setUploadErrorHandler(errorHandler);
	ServiceLocator::instance()->setLogger(logger);
	ServiceLocator::instance()->setDialogProvider(&dlgProvider);
    QString appDirectory = QCoreApplication::applicationDirPath();
    QString settingsFolder;
    setlocale(LC_ALL, "");

    if(QFileInfo::exists(appDirectory + "/Data/servers.xml")){
        dataFolder = appDirectory+"/Data/";
        settingsFolder = dataFolder;
    }
#ifndef _WIN32
   else {
dataFolder = "/usr/share/qimageuploader/";
   }

#ifndef __APPLE__
settingsFolder = getenv("HOME")+QString("/.config/qimageuploader/");
QDir settingsDir = QDir::root();
settingsDir.mkpath(settingsFolder);
#endif

#endif
    qDebug() << "Data directory:" << dataFolder;
    qDebug() << "Settings directory:" << settingsFolder;
    AppParams* params = AppParams::instance();
    params->setDataDirectory(Q2U(dataFolder));
    params->setSettingsDirectory(Q2U(settingsFolder));

    QTemporaryDir dir;
    if (dir.isValid()) {
        params->setTempDirectory(Q2U(dir.path()));
    } else {
        LOG(ERROR) << "Unable to create temp directory!";
    }

	Settings.LoadSettings(AppParams::instance()->settingsDirectory());
	auto engineList = std::make_unique<CUploadEngineList>();
	if (!engineList->loadFromFile(AppParams::instance()->dataDirectory() + "servers.xml", Settings.ServersSettings)) {
		QMessageBox::warning(nullptr, "Failure", "Unable to load servers.xml");
	}
    ServiceLocator::instance()->setEngineList(engineList.get());
    //google::AddLogSink(&logSink);
    //serviceLocator->setUploadErrorHandler(&uploadErrorHandler);
    //serviceLocator->setLogger(&defaultLogger);

	Settings.setEngineList(engineList.get());
	
    //QApplication::setStyle(new QtDotNetStyle);
    MainWindow w(engineList.get(), &logWindow);
    w.show();
    
    int res =  a.exec();

    //google::RemoveLogSink(&logSink);
	Settings.SaveSettings();
    google::ShutdownGoogleLogging();
    return res;
}
