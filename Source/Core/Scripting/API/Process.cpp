#include "Process.h"

#undef environ
#include <algorithm>
#include <boost/process.hpp>
#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"
#include "ScriptAPI.h"
#include "../Squirrelnc.h"

#ifdef _WIN32
//#include <boost/process/windows.hpp>
#include <boost/winapi/process.hpp>
#include <boost/winapi/show_window.hpp>
#include <boost/winapi/basic_types.hpp>
#endif
namespace ScriptAPI {

namespace bp = ::boost::process;

#ifdef _WIN32
// Thanks to https://stackoverflow.com/questions/43582022/boostprocess-hide-console-on-windows

struct show_window
    : ::boost::process::detail::handler_base
{
private: 
    ::boost::winapi::WORD_ const m_flag;

public: explicit
    show_window(bool const show) noexcept
    : m_flag{ show ? ::boost::winapi::SW_SHOWNORMAL_ : ::boost::winapi::SW_HIDE_ }
    {}

    // this function will be invoked at child process constructor before spawning process
    template <class WindowsExecutor>
    void on_setup(WindowsExecutor &e) const {
        // we have a chance to adjust startup info
        e.startup_info.dwFlags |= ::boost::winapi::STARTF_USESHOWWINDOW_;
        e.startup_info.wShowWindow |= m_flag;
    }
};
#else
struct show_window
    : ::boost::process::detail::handler_base
{
    show_window(bool const show) noexcept 
    {}

    template<typename Executor> void on_setup(Executor &) const{}
    template<typename Executor>
    void on_error(Executor &, const std::error_code &) const{}
    template<typename Executor> void on_success(Executor &) const{}
    template<typename Executor>
    void on_fork_error(Executor &, const std::error_code &) const{}
    template<typename Executor> void on_exec_setup(Executor &) const{}
    template<typename Executor>
    void on_exec_error(Executor &, const std::error_code &) const{}
};
#endif

class ProcessPrivate {
public:
    ProcessPrivate() :readProcessOutput_(false), hidden_(false)
    {
#ifdef _WIN32
        outputEncoding_ = "cp_oem";
#endif
    }

    ~ProcessPrivate() {
        if (child_) {
            child_->detach();
        }
    }
    bool start()
    {
        std::vector<std::string> args;
        Sqrat::Array::iterator it;
        if (!arguments_.IsNull()) {
            while (arguments_.Next(it))
            {
                Sqrat::Object obj(it.getValue(), GetCurrentThreadVM());
                args.push_back(obj.Cast<std::string>());
            }
        }

        try
        {   
            if (readProcessOutput_) {
                inputStream_ = std::make_unique<bp::ipstream>();
                child_.reset(new bp::child(executable_, bp::args(args), show_window{ !hidden_ }, bp::std_out > *inputStream_));
            } else {
                child_.reset(new bp::child(executable_, bp::args(args), show_window{ !hidden_ }));
            }
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
            std::vector<std::string> args;
            Sqrat::Array::iterator it;
            if (!arguments_.IsNull()) {
                while (arguments_.Next(it))
                {
                    Sqrat::Object obj(it.getValue(), GetCurrentThreadVM());
                    args.push_back(obj.Cast<std::string>());
                }
            }

            if (readProcessOutput_) {
                inputStream_ = std::make_unique<bp::ipstream>();
                bp::system(executable_, bp::args(args), show_window{ !hidden_ }, bp::std_out > *inputStream_);
            }
            else {
                bp::system(executable_, bp::args(args), show_window{ !hidden_ });
            }
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
        
        std::istreambuf_iterator<char> eos;

        res = std::string(std::istreambuf_iterator<char>(*inputStream_), eos);
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
        std::error_code code;
        child_->wait(code);
        return child_->exit_code();
    }
    std::string executable_;
    Sqrat::Array arguments_;
    bool readProcessOutput_;
    std::unique_ptr<bp::child> child_;
    std::string outputEncoding_;
    std::unique_ptr<bp::ipstream> inputStream_;
    bool hidden_;
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
    try {
        d_->executable_ = findInPath ? bp::search_path(executable).string() : executable;
    } catch ( std::exception& ex )
    {
        LOG(ERROR) << ex.what();
    }
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
    d_->hidden_ = true;
}

void Process::setCaptureOutput(bool read)
{
    d_->readProcessOutput_ = read;
}

const std::string Process::findExecutableInPath(const std::string& executable)
{
    return bp::search_path(executable).string();
}

void Process::setOutputEncoding(const std::string& encoding)
{
    d_->outputEncoding_ = encoding;
}

void Process::init()
{
    d_.reset(new ProcessPrivate());
    d_->readProcessOutput_ = false;
    d_->hidden_ = false;
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
        .Func("setCaptureOutput", &Process::setCaptureOutput)
        .Func("waitForExit", &Process::waitForExit)
        .Func("setHidden", &Process::setHidden)
        .Func("start", &Process::start)
    );

}
}