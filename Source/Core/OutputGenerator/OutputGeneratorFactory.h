#pragma once

#include <memory>

#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class OutputGeneratorFactory {
public:
    std::unique_ptr<AbstractOutputGenerator> createOutputGenerator(CodeLang lang, CodeType codeType);;
};

}
