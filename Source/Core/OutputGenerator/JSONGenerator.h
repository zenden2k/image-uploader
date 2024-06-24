
#pragma once

#include <string>
#include <vector>

#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class JSONGenerator : public AbstractOutputGenerator {
public:
    JSONGenerator() = default;
    GeneratorID id() const override;
    std::string generate(const std::vector<UploadObject>& items) override;
private:
};

}
