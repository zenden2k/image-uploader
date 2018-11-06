#ifndef IU_TESTS_TESTHELPERS_H
#define IU_TESTS_TESTHELPERS_H

#pragma once

#include <string>

namespace TestHelpers {
    void initDataPaths(const std::string& testDataPath);
    std::string resolvePath(const std::string &relPath);
}

#endif