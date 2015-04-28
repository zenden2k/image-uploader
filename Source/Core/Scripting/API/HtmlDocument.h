#ifndef IU_CORE_HTMLDOCUMENT_H
#define IU_CORE_HTMLDOCUMENT_H 
#include "Core/Utils/CoreTypes.h"
#include "HtmlElement.h"
#include "Core/Scripting/Squirrelnc.h"
namespace ScriptAPI {;

class HtmlDocumentPrivate;
/**
Represents a web browser's html document.
@since version 1.3.1 build 4272
*/
class HtmlDocument {
public:
	HtmlDocument();
    /*! @cond PRIVATE */
	HtmlDocument(HtmlDocumentPrivate* pr);
    /*! @endcond */
	HtmlElement rootElement();
	HtmlElement getElementById(const std::string& id);
	Sqrat::Array getElementsByTagName(const std::string& tag);
	Sqrat::Array getElementsByName(const std::string& name);
	HtmlElement querySelector(const std::string& query);
    /**
    Return an array of HtmlElement matched to query.
    */
	Sqrat::Array querySelectorAll(const std::string& query);
	const std::string getHTML();
protected:
	std_tr::shared_ptr<HtmlDocumentPrivate> d_;
};

void RegisterHtmlDocumentClass(Sqrat::SqratVM& vm);

}
#endif