
#pragma once

#include <string>
#include <vector>

#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class JSONGenerator : public AbstractOutputGenerator {
public:
    JSONGenerator() = default;
    CodeLang lang() const override;
    std::string generate(const std::vector<UploadObject>& items) override;
private:
};

}
