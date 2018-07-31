#include <gtest/gtest.h>
#ifdef _MSC_VER
    #include <vld.h> //Check for memory leaks
#endif
#define _SQ64
#include "Core/Scripting/Squirrelnc.h"
#include <sqtest/sqtest.h>
#include "Core/Scripting/API/ScriptAPI.h"
#include "Core/Logging.h"
#include "Core/Logging/ConsoleLogger.h"
#include "Core/Upload/ConsoleUploadErrorHandler.h"
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include "Core/Logging/MyLogSink.h"
#include "Core/ServiceLocator.h"
#include <vector>
#include <memory>

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
#ifdef _WIN32
// Convert UNICODE (UCS-2) command line arguments to utf-8
char ** convertArgv(int argc, _TCHAR* argvW[]) {
    char ** result = new char *[argc];
    for (int i = 0; i < argc; i++) {
        std::string unicodeString = IuCoreUtils::WstringToUtf8(argvW[i]).c_str();
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

int _tmain(int argc, _TCHAR* argvW[]) {
    char **argv = convertArgv(argc, argvW);
    //FLAGS_logtostderr = true;
#else
int main(int argc, char *argv[]){
#endif
    if (argc < 2) {
        fprintf(stderr, "Test scripts directory is not provided as command line argument.\r\n");
#ifdef _WIN32
        freeArgv(argc, argv);
#endif
        return 1;
    }
    std::string rootDir = argv[1];
    std::string testScriptsDir = rootDir + "/Source/Core/Scripting/API/Tests/";
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
    ConsoleLogger defaultLogger;
    ConsoleUploadErrorHandler uploadErrorHandler;
    ServiceLocator* serviceLocator = ServiceLocator::instance();
    serviceLocator->setUploadErrorHandler(&uploadErrorHandler);
    serviceLocator->setLogger(&defaultLogger);
    MyLogSink logSink(&defaultLogger);
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
    ::testing::InitGoogleTest(&argc, argv);

    int res = RUN_ALL_TESTS();

    ScriptAPI::ClearVmData(vm);
    google::ShutdownGoogleLogging();
#ifdef _WIN32
    freeArgv(argc, argv);
#endif
    return res;
}

