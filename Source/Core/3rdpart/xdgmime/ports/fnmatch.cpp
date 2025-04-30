#include "fnmatch.h"

#include "Core/Utils/StringUtils.h"

extern "C" {

int fnmatch(const char* pattern, const char* string, int flags) {
    return IuStringUtils::PatternMatch(pattern, string, flags);
}

}
