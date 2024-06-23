#include "OutputGeneratorFactory.h"

#include "BBCodeGenerator.h"
#include "MarkdownGenerator.h"
#include "HTMLGenerator.h"
#include "PlainTextGenerator.h"
#include "JSONGenerator.h"
#include "XmlTemplateGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

std::unique_ptr<AbstractOutputGenerator> OutputGeneratorFactory::createOutputGenerator(GeneratorID gid, /*CodeLang lang, */ CodeType codeType,
                                                                    XmlTemplateList* xmlTemplateList) {
    std::unique_ptr<AbstractOutputGenerator> tmp;
    switch (gid) {
    case gidBBCode:
        tmp = std::make_unique<BBCodeGenerator>();
        break;
    case gidHTML:
        tmp = std::make_unique<HTMLGenerator>();
        break;
    case gidMarkdown:
        tmp = std::make_unique<MarkdownGenerator>();
        break;
    case gidPlain:
        tmp = std::make_unique<PlainTextGenerator>();
        break;
    case gidJSON:
        tmp = std::make_unique<JSONGenerator>();
        break;
    case gidXmlTemplate:
        tmp = std::make_unique<XmlTemplateGenerator>(xmlTemplateList);
        break;
    }
    if (!tmp) {
        return tmp;
    }
    tmp->setType(codeType);
    return tmp;
}

}
