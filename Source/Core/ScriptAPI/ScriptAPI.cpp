#include "ScriptAPI.h"
#include "WebBrowser.h"
#include "HtmlDocument.h"
#include "HtmlElement.h"

namespace ScriptAPI {

void RegisterClasses(SquirrelObject* rootTable) {
	RegisterWebBrowserClass();
	RegisterHtmlDocumentClass();
	RegisterHtmlElementClass();
}

}