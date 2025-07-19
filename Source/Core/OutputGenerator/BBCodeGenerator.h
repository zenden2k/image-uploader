#pragma once

#include "AbstractCodeGenerator.h"

namespace Uptooda::Core::OutputGenerator {

class BBCodeGenerator: public AbstractCodeGenerator {
public:
    GeneratorID id() const override;
private:
    std::string image(const std::string& url, const std::string& alt) override;
    std::string link(const std::string& url, const std::string& body) override;
    std::string group(const std::string& fileName, const std::string& content, size_t index) override;
};
}
