#include "LangHelper.h"

#include <filesystem>

namespace LangHelper {

std::vector<std::wstring> getLanguageList(const std::wstring& languagesDirectory) {
    namespace fs = std::filesystem;
    std::vector<std::wstring> result;
    for (const auto& p : fs::directory_iterator(languagesDirectory)) {
        fs::path path = p.path();
        if (path.extension() != ".lng") {
            continue;
        }
        std::wstring basename = path.stem();
        if (basename == L"English") {
            continue;
        }
        result.push_back(basename);
    }
    return result;
}

}