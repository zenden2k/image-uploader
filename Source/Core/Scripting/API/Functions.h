#ifndef IU_CORE_SCRIPTAPI_FUNCTIONS_H
#define IU_CORE_SCRIPTAPI_FUNCTIONS_H

#include <string>
#include "../Squirrelnc.h"

namespace ScriptAPI {
    const std::string GetScriptsDirectory();
    const std::string GetAppLanguageFile();
    /*SquirrelObject*/Sqrat::Object  IncludeScript(const std::string& filename);
    void RegisterFunctions(Sqrat::SqratVM& vm);
    
    void RegisterShortTranslateFunctions(Sqrat::SqratVM& vm);
    void CleanUpFunctions();
}

#endif