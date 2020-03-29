#ifndef IU_TESTS_TESTHELPERS_H
#define IU_TESTS_TESTHELPERS_H

#pragma once

#include <string>
#include <vector>

namespace TestHelpers {
    void initDataPaths(const std::string& testDataPath);
    std::string resolvePath(const std::string &relPath);
    bool getFileContents(const std::string& fileName, std::vector<std::string> & vecOfStrs);
}

#endif