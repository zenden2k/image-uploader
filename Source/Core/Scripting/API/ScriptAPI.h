#ifndef IU_CORE_SCRIPTAPI_SCRIPTAPI_H
#define IU_CORE_SCRIPTAPI_SCRIPTAPI_H

/*! @cond PRIVATE */

#include "Functions.h"
#include <memory>
#include "Core/3rdpart/fastdelegate.h"
namespace ScriptAPI {

class WebBrowserPrivateBase;
void RegisterAPI(Sqrat::SqratVM& vm);
void CleanUp();
HSQUIRRELVM GetCurrentThreadVM();
void SetCurrentThreadVM(HSQUIRRELVM vm);
void StopAssociatedBrowsers(HSQUIRRELVM vm);
void AddBrowserToVM(HSQUIRRELVM vm, WebBrowserPrivateBase* browser);
void RemoveBrowserToVM(HSQUIRRELVM vm, WebBrowserPrivateBase* browser);
typedef fastdelegate::FastDelegate1<const std::string&> PrintCallback;
void SetPrintCallback(Sqrat::SqratVM& vm, const PrintCallback& callback);
void SetScriptName(Sqrat::SqratVM& vm, const std::string& fileName);
void ClearVmData(Sqrat::SqratVM& vm);
const std::string GetScriptName(HSQUIRRELVM);
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