#ifndef IU_CORE_SCRIPTING_API_GUMBOBINDINGS_DOCUMENT_H
#define IU_CORE_SCRIPTING_API_GUMBOBINDINGS_DOCUMENT_H

#pragma once

#include <string>
#include "Core/Scripting/Squirrelnc.h"

#include "Core/3rdpart/GumboQuery/Document.h"
#include "Core/3rdpart/GumboQuery/Node.h"

namespace ScriptAPI {

/**
HTML document (gumbo-query).
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

void RegisterGumboClasses(Sqrat::SqratVM& vm);

}
#endif
