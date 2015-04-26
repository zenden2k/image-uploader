#include "Process.h"
#include <boost/process.hpp>
#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"
#include "ScriptAPI.h"
#include "../Squirrelnc.h"

namespace ScriptAPI {

namespace bp = ::boost::process;

class ProcessPrivate {
public:
    ProcessPrivate()
    {
#ifdef _WIN32
        outputEncoding_ = "cp_oem";
#endif
    }
    bool start()
    {
        std::vector<std::string> args = {/*IuCoreUtils::ExtractFileName*/(executable_) };
        Sqrat::Array::iterator it;
        while (arguments_.Next(it))
        {
            Sqrat::Object obj(it.getValue(), GetCurrentThreadVM().GetVM());
            args.push_back(obj.Cast<std::string>());
        }

        try
        {
            bp::context ctx;
            ctx.stdout_behavior = readProcessOutput_ ? bp::capture_stream() : bp::silence_stream();
            ctx.environment = boost::process::self::get_environment();
            child_.reset(new bp::child(bp::launch(executable_, args, ctx)));
        }
        catch (std::exception & ex){
            LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what());
            return false;
        }

        return true;
    }

    bool launchShell(const std::string& command)
    {
        try
        {
            bp::context ctx;
            ctx.stdout_behavior = readProcessOutput_ ? bp::capture_stream() : bp::silence_stream();
            ctx.environment = boost::process::self::get_environment();
            child_.reset(new bp::child(bp::launch_shell(command, ctx)));
        }
        catch (std::exception & ex){
            LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what());
            return false;
        }
        return true;
    }

    void readOutput(std::string& res )
    {
        if (!child_)
        {
            LOG(ERROR) << BOOST_CURRENT_FUNCTION << "\r\n" << "Process not started";
            return;
        }
        bp::pistream &stream = child_->get_stdout();
        std::istreambuf_iterator<char> eos;

        res = std::string(std::istreambuf_iterator<char>(stream), eos);
        if (!outputEncoding_.empty())
        {
            res = IuCoreUtils::ConvertToUtf8(res, outputEncoding_);
        } 
        #ifndef _WIN32
        else
        {
            res = IuCoreUtils::SystemLocaleToUtf8(res);
        }
        #endif
       
    }

    int waitForExit()
    {
        if (!child_)
        {
            LOG(ERROR) << BOOST_CURRENT_FUNCTION << "\r\n" << "Process not started";
            return EXIT_FAILURE;
        }
        bp::status status = child_->wait();
        return status.exited() ? status.exit_status() : EXIT_FAILURE;
    }
    std::string executable_;
    Sqrat::Array arguments_;
    bool readProcessOutput_;
    std::unique_ptr<bp::child> child_;
    std::string outputEncoding_;
};

Process::Process()
{
    init();
}

Process::Process(const std::string& program)
{
    init();
    d_->executable_ = program;
}

Process::Process(const std::string& executable, bool findInPath)
{
    init();
    d_->executable_ = findInPath ? bp::find_executable_in_path(executable) : executable;
}

void Process::setExecutable(const std::string& program)
{
    d_->executable_ = program;
}

void Process::setArguments(Sqrat::Array arguments)
{
    d_->arguments_ = arguments;
}

bool Process::launchInShell(const std::string& command)
{
    return d_->launchShell(command);
}

bp::child start_child(bp::context ctx)
{
#if defined(BOOST_POSIX_API) 
    return bp::launch(std::string("env"), std::vector<std::string>(), ctx); 
#elif defined(BOOST_WINDOWS_API) 
    return bp::launch_shell("set", ctx);
#else 
#  error "Unsupported platform." 
#endif 
}

bool Process::start()
{
    return d_->start();
}

int Process::waitForExit()
{
    return d_->waitForExit();
}

const std::string Process::readOutput()
{
    std::string res;
    d_->readOutput(res);
    return res;
}

void Process::setHidden(bool hidden)
{

}

void Process::setCaptureOutput(bool read)
{
    d_->readProcessOutput_ = read;
}

const std::string Process::findExecutableInPath(const std::string& executable)
{
    return bp::find_executable_in_path(executable);
}

void Process::setOutputEncoding(const std::string& encoding)
{
    d_->outputEncoding_ = encoding;
}

void Process::init()
{
    d_.reset(new ProcessPrivate());
    d_->readProcessOutput_ = false;
    d_->readProcessOutput_ = false;
}

void RegisterProcessClass(Sqrat::SqratVM& vm)
{
    using namespace Sqrat;
    Sqrat::RootTable& root = vm.GetRootTable();
    root.Bind("Process", Class<Process>(vm.GetVM(), "Process")
        .Ctor()
        .Ctor<const std::string&>()
        .Ctor<const std::string&,bool>()
        .Func("setProgram", &Process::setExecutable)
        .Func("setArguments", &Process::setArguments)
        .Func("findExecutableInPath", &Process::findExecutableInPath)
        .Func("launchInShell", &Process::launchInShell)
        .Func("readOutput", &Process::readOutput)
        .Func("setReadOutput", &Process::setCaptureOutput)
        .Func("start", &Process::start)
    );

}
}