#pragma once

#include "AbstractCodeGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class HTMLGenerator : public AbstractCodeGenerator {
public:
    GeneratorID id() const override;
private:
    std::string image(const std::string& url, const std::string& alt) override;
    std::string link(const std::string& url, const std::string& body) override;
    std::string group(const std::string& fileName, const std::string& content, size_t index) override;
    std::string itemSeparator() const override;
    std::string lineSeparator() const override;
    std::string rowSeparator() const override;
};    

}



