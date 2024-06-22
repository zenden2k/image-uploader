
#pragma once

#include <string>
#include <vector>

#include "AbstractCodeGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class PlainTextGenerator : public AbstractCodeGenerator {
public:
    PlainTextGenerator() = default;
    CodeLang lang() const override;
private:
};

}
