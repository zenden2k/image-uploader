#include "TestHelpers.h"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

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

bool getFileContents(std::string fileName, std::vector<std::string> & vecOfStrs) {
    std::ifstream in(fileName.c_str());

    // Check if object is valid
    if (!in) {
        
        return false;
    }

    std::string str;

    while (std::getline(in, str)) {
        // Line contains string of length > 0 then save it in vector
        if (str.size() > 0)
            vecOfStrs.push_back(str);
    }
    in.close();
    return true;
}

}
