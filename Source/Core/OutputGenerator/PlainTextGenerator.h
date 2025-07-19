
#pragma once

#include <string>
#include <vector>

#include "AbstractCodeGenerator.h"

namespace Uptooda::Core::OutputGenerator {

class PlainTextGenerator : public AbstractCodeGenerator {
public:
    PlainTextGenerator() = default;
    GeneratorID id() const override;
private:
    std::string generateCodeForItem(const UploadObject& item, int index);
};

}
