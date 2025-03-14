#pragma once

#include <string>
#include <unordered_map>

class MimeTypeHelper {
public:
    static std::string getDefaultExtensionForMimeType(const std::string& mimeType);
private:
    static std::unordered_map<std::string, std::string> mimeToExt_;
};
