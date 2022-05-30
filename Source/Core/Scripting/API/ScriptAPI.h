#ifndef IU_CORE_SCRIPTAPI_SCRIPTAPI_H
#define IU_CORE_SCRIPTAPI_SCRIPTAPI_H

/*! @cond PRIVATE */

#include <functional>
#include "Functions.h"

namespace ScriptAPI {

class Stoppable {
public:
    virtual ~Stoppable() {}
    virtual void stop() = 0;
};

class WebBrowserPrivateBase;
void RegisterAPI(Sqrat::SqratVM& vm);
void CleanUp();
HSQUIRRELVM GetCurrentThreadVM();
void SetCurrentThreadVM(HSQUIRRELVM vm);
void StopAssociatedServices(HSQUIRRELVM vm);
void AddServiceToVM(HSQUIRRELVM vm, Stoppable* service);
void RemoveServiceFromVM(HSQUIRRELVM vm, Stoppable* service);
typedef std::function<void(const std::string&)> PrintCallback;
void SetPrintCallback(Sqrat::SqratVM& vm, const PrintCallback& callback);
void SetScriptName(Sqrat::SqratVM& vm, const std::string& fileName);
void ClearVmData(Sqrat::SqratVM& vm);
const std::string GetScriptName(HSQUIRRELVM);
void SetCurrentTopLevelFileName(Sqrat::SqratVM& vm, const std::string& fileName);
std::string GetCurrentTopLevelFileName();
void FlushSquirrelOutput(HSQUIRRELVM vm);
template<typename T>
    T GetValue(const Sqrat::SharedPtr<T> p)
    {
        if (!!p) {
            return *p.Get();
        }
        return T();
    }
}

/*! @endcond */


#endif