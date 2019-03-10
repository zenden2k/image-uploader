#ifndef IU_CORE_SCRIPTING_API_GUMBOBINDINGS_DOCUMENT_H
#define IU_CORE_SCRIPTING_API_GUMBOBINDINGS_DOCUMENT_H

#pragma once

#include <string>
#include "Core/Scripting/Squirrelnc.h"

#include "Core/3rdpart/GumboQuery/Document.h"
#include "Core/3rdpart/GumboQuery/Node.h"
#include <memory>

namespace ScriptAPI {

/**
(gumbo-query) HTML document.
* Example:
* @include gumbo_query.nut
@since version 1.3.2 build 4451
*/
class Document {
public:
    Document();
    explicit Document(const std::string& text);

    CSelection find(const std::string& aSelector);
protected:
    std::shared_ptr<CDocument> doc_;
};

/* @cond PRIVATE */
void RegisterGumboClasses(Sqrat::SqratVM& vm);
/* @endcond */

}
#endif
