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
Sqrat::SqratVM& GetCurrentThreadVM();
void SetCurrentThreadVM(Sqrat::SqratVM& vm);
void StopAssociatedBrowsers(Sqrat::SqratVM& vm);
void AddBrowserToVM(Sqrat::SqratVM& vm, WebBrowserPrivateBase* browser);
void RemoveBrowserToVM(Sqrat::SqratVM& vm, WebBrowserPrivateBase* browser);
typedef fastdelegate::FastDelegate1<const std::string&> PrintCallback;
void SetPrintCallback(Sqrat::SqratVM& vm, const PrintCallback& callback);
void SetScriptName(Sqrat::SqratVM& vm, const std::string& fileName);
void ClearVmData(Sqrat::SqratVM& vm);
const std::string GetScriptName(HSQUIRRELVM);
void FlushSquirrelOutput(Sqrat::SqratVM& vm);
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