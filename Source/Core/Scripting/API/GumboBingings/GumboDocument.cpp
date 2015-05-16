#include "GumboDocument.h"

namespace ScriptAPI {

Document::Document() : doc_(new CDocument()) {
}

Document::Document(const std::string& text) : doc_(new CDocument()) {
    doc_->parse(text);
}

CSelection Document::find(const std::string& aSelector) {
    return doc_->find(aSelector);
}

void RegisterGumboClasses(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
    Sqrat::RootTable& root = vm.GetRootTable();
    root.Bind("Document", Class<Document>(vm.GetVM(), "Document")
        .Ctor()
        .Ctor<const std::string&>()
        .Func("find", &Document::find)
    );

    root.Bind("Selection", Class<CSelection>(vm.GetVM(), "Selection")
        .Func("find", &CSelection::find)
        .Func("at", &CSelection::nodeAt)
        .Func("size", &CSelection::nodeNum)
        .Func("length", &CSelection::nodeNum)
        .Func("text", &CSelection::text)
        .Func("ownText", &CSelection::ownText)
        .Func("tagName", &CSelection::tagName)
        .Func("each", &CSelection::squirrelEach)
        .Func("attr", &CSelection::attribute)
        );

    root.Bind("Node", Class<CNode>(vm.GetVM(), "Node")
        .Func("valid", &CNode::valid)
        .Func("parent", &CNode::parent)
        .Func("next", &CNode::nextSibling)
        .Func("prev", &CNode::prevSibling)
        .Func("childAt", &CNode::childAt)
        .Func("childCount", &CNode::childNum)
        .Func("attr", &CNode::attribute)
        .Func("text", &CNode::text)
        .Func("ownText", &CNode::ownText)
        .Func("tagName", &CNode::tag)
        .Func("find", &CNode::find)
        );
}
}