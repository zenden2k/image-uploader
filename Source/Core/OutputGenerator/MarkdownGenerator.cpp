#include "MarkdownGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

CodeLang MarkdownGenerator::lang() const{
    return clMarkdown;
}

std::string MarkdownGenerator::image(const std::string& url, const std::string& alt) {
    return "![" + alt + "](" + url + ")";
}

std::string MarkdownGenerator::link(const std::string& url, const std::string& body) {
    return "[" + body + "](" + url + ")";
}

std::string MarkdownGenerator::group(const std::string& fileName, const std::string& content, size_t index) {
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

std::string MarkdownGenerator::itemSeparator() const {
    return "  ";
}

std::string MarkdownGenerator::lineSeparator() const {
    return "\r\n\r\n";
}

}
