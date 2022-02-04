#include "LangHelper.h"

#include <filesystem>
#include <glog/logging.h>

namespace LangHelper {

std::vector<std::wstring> getLanguageList(const std::wstring& languagesDirectory) {
    namespace fs = std::filesystem;
    std::vector<std::wstring> result;
    try {

        for (const auto& p : fs::directory_iterator(languagesDirectory, fs::directory_options::skip_permission_denied)) {
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
    } catch (fs::filesystem_error &e) {
        //LOG(ERROR) << e.what();
    }
    return result;
}

}
