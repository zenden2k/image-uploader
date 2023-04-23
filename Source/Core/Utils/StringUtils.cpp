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

#include "StringUtils.h"
 
#include <cctype>
#include <cstdio>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#else
#include <strings.h>
#endif
#include <utf8/unchecked.h>

namespace IuStringUtils {

std::string Trim(const std::string& str)
{
    std::string res;
    // Trim Both leading and trailing spaces
    size_t startpos = str.find_first_not_of(" \t\r\n"); // Find the first character position after excluding leading blank spaces
    size_t endpos = str.find_last_not_of(" \t\r\n"); // Find the first character position from reverse af

    // if all spaces or empty return an empty string
    if ((std::string::npos == startpos) || (std::string::npos == endpos))
    {
        res.clear();
    }
    else
        res = str.substr( startpos, endpos - startpos + 1 );
    return res;
}

std::string_view TrimSV(std::string_view str)
{
    std::string_view res;
    // Trim Both leading and trailing spaces
    size_t startpos = str.find_first_not_of(" \t\r\n"); // Find the first character position after excluding leading blank spaces
    size_t endpos = str.find_last_not_of(" \t\r\n"); // Find the first character position from reverse af

    // if all spaces or empty return an empty string
    if ((std::string::npos == startpos) || (std::string::npos == endpos))
    {
        return res;
    }
    return str.substr( startpos, endpos - startpos + 1 );
}

void Split(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount)
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
    int counter = 0;
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        counter++;
        if (counter == maxCount)
        {
            tokens.emplace_back(str.substr(lastPos, str.length()));
            break;
        }
        else
            // Found a token, add it to the vector.
            tokens.emplace_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

std::vector<std::string_view> SplitSV(std::string_view strv, std::string_view delims, int maxCount) {
    std::vector<std::string_view> output;
    size_t first = 0;
    int counter = 0;
    while (first < strv.size())
    {
        counter++;
        if (counter == maxCount)
        {
            output.emplace_back(strv.substr(first));
            break;
        } else {
            const auto second = strv.find_first_of(delims, first);

            if (first != second)
                output.emplace_back(strv.substr(first, second - first));

            if (second == std::string_view::npos)
                break;

            first = second + 1;
        }
    }

    return output;
}

std::string Replace(const std::string& text, const std::string& s, const std::string& d)
{
    std::string result = text;
    for (size_t index = 0; index = result.find(s, index), index != std::string::npos; )
    {
        result.replace(index, s.length(), d);
        index += d.length();
    }
    return result;
}

// As far as C++ standart doesn't have a case insensitive string comparison function, I wrote my own
int stricmp(const char* s1, const char* s2)
{
#ifdef _WIN32
    return lstrcmpiA(s1, s2);
#else 
    return strcasecmp(s1, s2);
    /*for (; *s1 && *s2 && (toupper((unsigned char)*s1) == toupper((unsigned char)*s2)); ++s1, ++s2)
        ;
    return *s1 - *s2;*/
#endif
}

std::string toLower(const std::string& str)
{
    std::string s1 = str;
    for (size_t i = 0; i < s1.length(); i++)
        s1[i] = static_cast<char>(::tolower(s1[i]));
    // std::string s1;
    // std::transform(str.begin(), str.end(), std::back_inserter(s1), std::tolower);
    return s1;
}

std::string Tail(std::string const& source, size_t length) {
    if (length >= source.size()) {
        return source;
    }
    return source.substr(source.size() - length);
}

std::string ConvertUnixLineEndingsToWindows(const std::string& text) {
    if ( text.empty() ) {
        return text;
    }
    const char* srcString = text.c_str();
    size_t srcLen = text.length();
    char * res = new char[(srcLen*2)+1];
    char * cur = res;
    const char * srcEnd = srcString + srcLen;
    const char* src = srcString;
    while ( src < srcEnd ) {
        if ( *src == '\r' ) {
            *cur = *src;
            src++;
            cur++;
            *cur = *src;
        } else if ( *src == '\n' ) {
            *cur = '\r';
            cur++;
            *cur = '\n';
            
        } else {
            *cur = *src;
        }
        src++;
        cur++;
    }
    *cur = 0;
    std::string result(res, cur-res-1);
    delete[] res;
    return result;
}

size_t LengthOfUtf8String(const std::string &utf8_string) {
    return utf8::unchecked::distance(utf8_string.begin(), utf8_string.end());
}

std::string Join(const std::vector<std::string>& strings, const std::string& delim)
{
    std::ostringstream result;

    if (!strings.empty()) {
        auto next = strings.begin();
        result << *next;
        for (++next; next != strings.end(); ++next) {
            result << delim << *next;
        }
    }
    return result.str();
}

}
