#ifndef IU_CORE_SCRIPTAPI_SCRIPTAPI_H
#define IU_CORE_SCRIPTAPI_SCRIPTAPI_H

#include "Functions.h"

namespace ScriptAPI {

void RegisterAPI(Sqrat::SqratVM& vm);
void CleanUp();

}
#endif