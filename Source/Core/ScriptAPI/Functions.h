#ifndef IU_CORE_SCRIPTAPI_FUNCTIONS_H
#define IU_CORE_SCRIPTAPI_FUNCTIONS_H

#include <string>
#include "../Squirrelnc.h"

namespace ScriptAPI {
	const std::string GetScriptsDirectory();
	const std::string GetAppLanguageFile();
	SquirrelObject IncludeScript(const std::string& filename);
	void RegisterFunctions(SquirrelObject* rootTable);
	
	void RegisterShortTranslateFunctions(bool tr, bool underscore);
    void CleanUpFunctions();
}

#endif