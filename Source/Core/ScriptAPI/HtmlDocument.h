#ifndef IU_CORE_HTMLDOCUMENT_H
#define IU_CORE_HTMLDOCUMENT_H 
#include <Core/Utils/CoreTypes.h>
#include "HtmlElement.h"
#include <Core/Squirrelnc.h>
namespace ScriptAPI {

class HtmlDocumentPrivate;

class HtmlDocument {
public:
	HtmlDocument();
	HtmlDocument(HtmlDocumentPrivate* pr);
	HtmlElement rootElement();
	HtmlElement getElementById(const std::string& id);
	SquirrelObject getElementsByTagName(const std::string& tag);
	SquirrelObject getElementsByName(const std::string& name);
protected:
	std_tr::shared_ptr<HtmlDocumentPrivate> d_;
};

void RegisterHtmlDocumentClass();

}
#endif