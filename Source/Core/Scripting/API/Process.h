#ifndef IU_CORE_SCRIPTAPI_PROCESS_H
#define IU_CORE_SCRIPTAPI_PROCESS_H

#pragma once

#include "Core/Scripting/Squirrelnc.h"
#include <memory>

namespace ScriptAPI {

class ProcessPrivate;

/*!
* @brief The Process class is used to start external programs and to communicate with them.  
* @since version 1.3.2
*/
class Process {
public:
    /**
    * Default empty constructor
    */
    Process();

    /**
    * Constructor which takes executable name (should be absolute or relative path)
    */
    Process(const std::string& executable);

    /**
    * @param findInPath if true, executable is being searched in directories listed in PATH environment variable
    */
    Process(const std::string& executable, bool findInPath);

    void setExecutable(const std::string& program);
    void setArguments(Sqrat::Array arguments);

    /*!
    * @brief Launches a shell-based command. 
    * 
    * Executes the given command through the default system shell. The 
    * command is subject to pattern expansion, redirection and pipelining. 
    * 
    * This function behaves similarly to the system(3) system call. In a 
    * POSIX system, the command is fed to /bin/sh whereas under a Windows 
    * system, it is fed to cmd.exe. It is difficult to write portable 
    * commands as the first parameter, but this function comes in handy in 
    * multiple situations. 
    */
    bool launchInShell(const std::string & command);
    bool start();

    /*!
    * Wait indefinitely for the associated process to exit.
    * @return Exist code returned by child process
    */
    int waitForExit();
    std::string readOutput();

    /**
    * If set to true, process window is hidden (Windows only)
    */
    void setHidden(bool hidden);
    void setCaptureOutput(bool read);
    std::string findExecutableInPath(const std::string& executable);

    /**
    Set the encoding of process output stream.
    By default, CP_OEMCP is used  on Windows and current locale on other systems. (UTF-8 on most of modern UNIX-bases systems) 
    */
    void setOutputEncoding(const std::string& encoding);
protected:
    std::shared_ptr<ProcessPrivate> d_;
    void init();
};

/* @cond PRIVATE */
void RegisterProcessClass(Sqrat::SqratVM& vm);
/* @endcond */
}

#endif
