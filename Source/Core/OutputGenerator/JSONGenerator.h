
#pragma once

#include <string>
#include <vector>

#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class JSONGenerator : public AbstractOutputGenerator {
public:
    JSONGenerator() = default;
    GeneratorID id() const override;
protected:
    std::string doGenerate(const std::vector<UploadObject>& items) override;
private:
};

}
