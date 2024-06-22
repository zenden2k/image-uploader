#include "BBCodeGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

CodeLang BBCodeGenerator::lang() const {
    return clBBCode;
}

std::string BBCodeGenerator::image(const std::string& url, const std::string& alt) {
    return "[img]" + url + "[/img]";
}

std::string BBCodeGenerator::link(const std::string& url, const std::string& body) {
    return "[url=" + url + "]" + body + "[/url]";
}

std::string BBCodeGenerator::group(const std::string& fileName, const std::string& content, size_t index) {
    std::string result;
    if (groupByFile_ && !content.empty()) {
        if (index) {
            result += "\r\n";
        }
        result += fileName + "\r\n\r\n";
    }

    result += content;
    return result;
}

}
