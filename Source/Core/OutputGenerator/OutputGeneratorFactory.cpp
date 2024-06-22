#include "OutputGeneratorFactory.h"

#include "BBCodeGenerator.h"
#include "MarkdownGenerator.h"
#include "HTMLGenerator.h"
#include "PlainTextGenerator.h"
#include "JSONGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

std::unique_ptr<AbstractOutputGenerator> OutputGeneratorFactory::createOutputGenerator(CodeLang lang, CodeType codeType) {
    std::unique_ptr<AbstractOutputGenerator> tmp;
    switch (lang) {
    case clBBCode:
        tmp = std::make_unique<BBCodeGenerator>();
        break;
    case clHTML:
        tmp = std::make_unique<HTMLGenerator>();
        break;
    case clMarkdown:
        tmp = std::make_unique<MarkdownGenerator>();
        break;
    case clPlain:
        tmp = std::make_unique<PlainTextGenerator>();
        break;
    case clJSON:
        tmp = std::make_unique<JSONGenerator>();
        break;
    }
    if (!tmp) {
        return tmp;
    }
    tmp->setType(codeType);
    return tmp;
}

}
