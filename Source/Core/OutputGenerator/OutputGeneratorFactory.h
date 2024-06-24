#pragma once

#include <memory>

#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class XmlTemplateList;

class OutputGeneratorFactory {
public:
    std::unique_ptr<AbstractOutputGenerator> createOutputGenerator(GeneratorID gid, /*CodeLang lang,*/ CodeType codeType, XmlTemplateList* xmlTemplateList = nullptr);
};

}
