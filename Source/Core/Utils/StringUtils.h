/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef IU_CORE_UTILS_STRINGUTILS_H
#define IU_CORE_UTILS_STRINGUTILS_H

#include <string>
#include <string_view>
#include <vector>
#include <boost/format.hpp>
#include "CoreTypes.h"

namespace IuStringUtils
{
    std::string Trim(const std::string& str);
    std::string_view TrimSV(std::string_view str);
    std::string Replace(const std::string& text, const std::string& s, const std::string& d);
    void Split(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount = -1);
    std::vector<std::string_view> SplitSV(std::string_view strv, std::string_view delims, int maxCount = -1);
    std::string Join(const std::vector<std::string>& strings, const std::string& delim);
    std::string Tail(std::string const& source, size_t length);

    // Current version of toLower works only with ASCII strings
    std::string toLower(const std::string& str);

    //  The stricmp() function compares the two strings s1 and s2, 
    //  ignoring the case of the characters. It returns an integer less than, 
    //     equal to, or greater than zero if s1 is found, respectively, to be less than, 
    //     to match, or be greater than s2.
    // IT WORKS ONLY WITH ANSI STRINGS!
    int stricmp(const char *s1, const char *s2);
    std::string ConvertUnixLineEndingsToWindows(const std::string& text);
    size_t LengthOfUtf8String(const std::string &utf8_string);
    boost::format FormatNoExcept(const std::string& str);
    bool Match(char const* needle, char const* haystack);
};

#endif
