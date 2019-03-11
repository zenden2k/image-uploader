#include "Gui/mainwindow.h"
#include <QApplication>
//#include <3rdparty/qtdotnetstyle.h>
#include "Core/Logging.h"
#include <boost/filesystem/path.hpp>
#include <boost/locale.hpp>
#include "Core/ServiceLocator.h"
#include "Core/AppParams.h"

#include "QtUploadErrorHandler.h"
#include "QtDefaultLogger.h"
#include "Gui/LogWindow.h"
#include "Core/i18n/Translator.h"
#ifdef _WIN32
CAppModule _Module;
std::string dataFolder = "Data/";
#else
std::string dataFolder = "/usr/share/imageuploader/";
#endif

class Translator : public ITranslator {
public:
	virtual std::string getCurrentLanguage() override {
		return "English";
	}
	virtual std::string getCurrentLocale() override {
		return "en_US";
	}
	virtual std::string translate(const char* str) override {
		return str;
	}
#ifdef _WIN32
	virtual const wchar_t* translateW(const wchar_t* str) override {
		return str;
	}
#endif
};
Translator translator; // dummy translator


int main(int argc, char *argv[])
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
    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = true;
    // Enable memory dump from within VS.
#else
        FLAGS_logtostderr = true;
#endif

    google::InitGoogleLogging(argv[0]);

	QApplication a(argc, argv);
	LogWindow* logWindow = new LogWindow();
	logWindow->show();
	QtDefaultLogger logger(logWindow);
	QtUploadErrorHandler errorHandler(nullptr);
	ServiceLocator::instance()->setTranslator(&translator);
	ServiceLocator::instance()->setUploadErrorHandler(&errorHandler);
	ServiceLocator::instance()->setLogger(&logger);
    std::string appDirectory = IuCoreUtils::ExtractFilePath(argv[0]);
    std::string settingsFolder;
    setlocale(LC_ALL, "");

   if(IuCoreUtils::FileExists(appDirectory + "/Data/servers.xml"))
    {
        dataFolder = appDirectory+"/Data/";
        settingsFolder = dataFolder;
    }
#ifndef _WIN32
   else {
dataFolder = "/usr/share/imgupload/";
   }

#ifndef __APPLE__
settingsFolder = getenv("HOME")+std::string("/.config/imgupload/");
mkdir(settingsFolder.c_str(), 0700);
#endif

#endif
    AppParams* params = AppParams::instance();
    params->setDataDirectory(dataFolder);
    params->setSettingsDirectory(settingsFolder);

#ifdef _WIN32
    TCHAR ShortPath[1024];
    GetTempPath(ARRAY_SIZE(ShortPath), ShortPath);
    TCHAR TempPath[1024];
    if (!GetLongPathName(ShortPath,TempPath, ARRAY_SIZE(TempPath)) ) {
        lstrcpy(TempPath, ShortPath);
    }
    params->setTempDirectory(IuCoreUtils::WstringToUtf8(TempPath));
#else
    return  params->setTempDirectory("/var/tmp/");
#endif

    ServiceLocator* serviceLocator = ServiceLocator::instance();

    //google::AddLogSink(&logSink);
    //serviceLocator->setUploadErrorHandler(&uploadErrorHandler);
    //serviceLocator->setLogger(&defaultLogger);

	

    
	
    //QApplication::setStyle(new QtDotNetStyle);
    MainWindow w(logWindow);
    w.show();
    
    int res =  a.exec();

    //google::RemoveLogSink(&logSink);
    google::ShutdownGoogleLogging();
    return res;
}
