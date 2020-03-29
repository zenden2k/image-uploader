
#include <vector>
#include <boost/program_options.hpp>
#include <gtest/gtest.h>
#ifdef _MSC_VER
   // #include <vld.h> //Check for memory leaks
#endif
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include "sqtest.h"
#include "TestHelpers.h"
#include "Core/Scripting/Squirrelnc.h"
#include "Core/Scripting/API/ScriptAPI.h"
#include "Core/Logging.h"
#include "Core/Logging/ConsoleLogger.h"
#include "Core/Upload/ConsoleUploadErrorHandler.h"
#include "Core/Logging/MyLogSink.h"
#include "Core/ServiceLocator.h"
#include "Core/i18n/Translator.h"

#ifdef _WIN32
#include "atlheaders.h"
#endif

void printHandler(HSQUIRRELVM vm, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    vfprintf(stdout, s, vl);
    va_end(vl);

    (void)vm;
}

void errorHandler(HSQUIRRELVM vm, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    vfprintf(stderr, s, vl);
    va_end(vl);

    (void)vm;
}

class Translator : public ITranslator {
public:
    virtual std::string getCurrentLanguage() override {
        return "English";
    }
    virtual std::string getCurrentLocale() override {
        return "en_US";
    }
    virtual std::string translate(const char* str) override{
        return str;
    }
#ifdef _WIN32
    virtual const wchar_t* translateW(const wchar_t* str) override {
        return str;
    }
#endif
};

#ifdef _WIN32

CAppModule _Module;

// Convert UNICODE (UCS-2) command line arguments to utf-8
char ** convertArgv(int argc, _TCHAR* argvW[]) {
    char ** result = new char *[argc];
    for (int i = 0; i < argc; i++) {
        std::string unicodeString = IuCoreUtils::WstringToUtf8(argvW[i]);
        char *buffer = new char[unicodeString.length() + 1];
        strcpy(buffer, unicodeString.c_str());
        result[i] = buffer;
    }
    return result;
}

void freeArgv(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        delete[] argv[i];
    }
    delete[] argv;
}

class ArgDeleter {
public:
    ArgDeleter(int argc, char *argv[]) : argc_(argc), argv_(argv){
        
    }
    ~ArgDeleter() {
        freeArgv(argc_, argv_);
    }
    DISALLOW_COPY_AND_ASSIGN(ArgDeleter);
protected:
    int argc_;
    char **argv_;
};

int _tmain(int argc, _TCHAR* argvW[]) {
    char **argv = convertArgv(argc, argvW);
    //ArgDeleter deleter(argc, argv);
    //FLAGS_logtostderr = true;
#else
int main(int argc, char *argv[]){
#endif
    ::testing::InitGoogleTest(&argc, argv);

    std::string testDataDir, testScriptsDir;
    try {
        namespace po = boost::program_options;
        po::options_description desc("General options");
        
        desc.add_options()
            ("help,h", "Show help")
            ("dir,d", po::value<std::string>(&testDataDir)->required(), "Test data directory")
            ("sqdir,s", po::value<std::string>(&testScriptsDir)->required(), "Squirrel tests directory (containing .nut files) ")
            ;

        po::variables_map vm;
        po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc)/*.allow_unregistered()*/.run();
        po::store(parsed, vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }
        po::notify(vm);
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    if (!IuCoreUtils::DirectoryExists(testDataDir)) {
        std::cout << "Directory '" << testDataDir << "' does not exist." << std::endl;
        return 1;
    }
    if (!IuCoreUtils::DirectoryExists(testScriptsDir)) {
        std::cout << "Directory '" << testScriptsDir << "' does not exist." << std::endl;
        return 1;
    }
    TestHelpers::initDataPaths(testDataDir);
    

    //std::string rootDir = argv[1];
    //std::string testScriptsDir = rootDir + "/Source/Core/Scripting/API/Tests/";
    google::InitGoogleLogging("test");
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
    FLAGS_log_dir.reserve(_MAX_PATH); // comment out this line to trigger false memory leak
    FLAGS_log_link.reserve(_MAX_PATH);

    // Enable memory dump from within VS.
#endif
    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = true;
    auto defaultLogger = std::make_shared<ConsoleLogger>();
    auto uploadErrorHandler = std::make_shared<ConsoleUploadErrorHandler>();
    Translator translator;
    ServiceLocator* serviceLocator = ServiceLocator::instance();
    serviceLocator->setUploadErrorHandler(uploadErrorHandler);
    serviceLocator->setLogger(defaultLogger);
    serviceLocator->setTranslator(&translator);
    MyLogSink logSink(defaultLogger.get());
    google::AddLogSink(&logSink);

    Sqrat::SqratVM vm;
    vm.SetPrintFunc(&printHandler, &errorHandler);
    sqstd_seterrorhandlers(vm.GetVM());
    ScriptAPI::SetScriptName(vm, "test.nut");
    sq_pushroottable(vm.GetVM());
   
    SQRESULT result = sqtest_register_lib(vm.GetVM());
    if (SQ_FAILED(result)) {
        fprintf(stderr, "call to sqtest_register_lib failed.\n");
        return 1;
    }
    sq_pop(vm.GetVM(), 1);
    ScriptAPI::RegisterAPI(vm);
    sqtest_addtest(vm.GetVM(), (testScriptsDir + "globals.nut").c_str());


    int res = RUN_ALL_TESTS();

    ScriptAPI::ClearVmData(vm);
    google::ShutdownGoogleLogging();
    return res;
}

