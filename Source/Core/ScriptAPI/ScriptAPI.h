#ifndef IU_CORE_SCRIPTAPI_SCRIPTAPI_H
#define IU_CORE_SCRIPTAPI_SCRIPTAPI_H

#include "Functions.h"
#include "RegularExpression.h"
#include "WebBrowser.h"

namespace ScriptAPI {

extern SquirrelObject* RootTable;
void RegisterClasses(SquirrelObject* rootTable);

}
#endif