#include "TestHelpers.h"

#include <boost/filesystem.hpp>

namespace TestHelpers {

std::string testDataPath;

void initDataPaths(const std::string& path) {
    testDataPath = path;
}

std::string resolvePath(const std::string &relPath) {
    namespace fs = boost::filesystem;
    fs::path dir(testDataPath);
    fs::path full_path = dir / relPath;
    return full_path.string();
}

}
