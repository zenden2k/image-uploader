#ifndef IU_CORE_SCRIPTAPI_FUNCTIONS_H
#define IU_CORE_SCRIPTAPI_FUNCTIONS_H

#include <string>
#include "../Squirrelnc.h"

namespace ScriptAPI {
	const std::string GetScriptsDirectory();
	const std::string GetAppLanguageFile();
	/*SquirrelObject*/bool IncludeScript(const std::string& filename);
	void RegisterFunctions(HSQUIRRELVM vm);
	
	void RegisterShortTranslateFunctions(bool tr);
    void CleanUpFunctions();
}

#endif