#include "ScriptAPI.h"
#include "WebBrowser.h"

namespace ScriptAPI {

void RegisterClasses(SquirrelObject* rootTable) {
	RegisterWebBrowserClass();
}

}