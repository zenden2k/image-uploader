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
	Sqrat::Array getElementsByTagName(const std::string& tag);
	Sqrat::Array getElementsByName(const std::string& name);
	HtmlElement querySelector(const std::string& query);
	Sqrat::Array querySelectorAll(const std::string& query);
	const std::string getHTML();
protected:
	std_tr::shared_ptr<HtmlDocumentPrivate> d_;
};

void RegisterHtmlDocumentClass(Sqrat::SqratVM& vm);

}
#endif