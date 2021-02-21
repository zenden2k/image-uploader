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
#ifndef IU_CORE_SCRIPTAPI_H
#define IU_CORE_SCRIPTAPI_H

#include <string>

#include "Core/3rdpart/pcreplusplus.h"
#include "../Squirrelnc.h"
#include "Core/Utils/CoreTypes.h"

namespace ScriptAPI {

/**
PCRE-compatible regular expression
*/
class RegularExpression  {
public:
    RegularExpression();
    /**
        Creating instance: local regex = CRegExp(pattern, flags);

        pattern - regular expression, 

        flags:

        <b>i</b> - PCRE_CASELESS

        If this modifier is set, letters in the pattern match both upper and lower case letters.

        <b>m</b> - PCRE_MULTILINE

        By default, PCRE treats the subject string as consisting of a single "line" of characters 
            (even if it actually contains several newlines). The "start of line" metacharacter (^)
            matches only at the start of the string, while the "end of line" metacharacter ($) 
            matches only at the end of the string, or before a terminating newline (unless D 
            modifier is set). This is the same as Perl. When this modifier is set, the "start 
            of line" and "end of line" constructs match immediately following or immediately
            before any newline in the subject string, respectively, as well as at the very start 
            and end. This is equivalent to Perl's /m modifier. If there are no "\n" characters in
            a subject string, or no occurrences of ^ or $ in a pattern, setting this modifier has 
            no effect.

        <b>s</b> - PCRE_DOTALL

        If this modifier is set, a dot metacharacter in the pattern matches all characters, including 
            newlines. Without it, newlines are excluded. This modifier is equivalent to Perl's /s 
            modifier. A negative class such as [^a] always matches a newline character, independent 
            of the setting of this modifier.

        <b>x</b> - PCRE_EXTENDED

        If this modifier is set, whitespace data characters in the pattern are totally ignored except 
            when escaped or inside a character class, and characters between an unescaped # outside 
            a character class and the next newline character, inclusive, are also ignored. This is 
            equivalent to Perl's /x modifier, and makes it possible to include commentary inside 
            complicated patterns. Note, however, that this applies only to data characters. 
            Whitespace characters may never appear within special character sequences in a pattern, 
            for example within the sequence (?( which introduces a conditional subpattern.

        <b>u</b> - PCRE_UTF8|PCRE_UCP

        This modifier turns on additional functionality of PCRE that is incompatible with Perl. 
            Pattern and subject strings are treated as UTF-8. Five and six octet UTF-8 sequences are 
            regarded as invalid.

        <b>g</b> - global
     */
    RegularExpression(const std::string& expression, const std::string& flags);
    Sqrat::Array split(const std::string& piece);
    Sqrat::Array splitWithLimit(const std::string& piece, int limit);
    Sqrat::Array splitWithLimitOffset(const std::string& piece, int limit, int start_offset);
    Sqrat::Array splitWithLimitStartEndOffset(const std::string& piece, int limit, int start_offset, int end_offset);

    std::string getMatch(int pos) const;
    bool search(const std::string& stuff);
    bool searchWithOffset(const std::string& stuff, int OffSet);
    Sqrat::Array findAll(const std::string& stuff);
    std::string replace(const std::string& piece, const std::string& with);
    int getEntireMatchStart() const;
    int getMatchStart(int pos) const;
    int getEntireMatchEnd() const;
    int getMatchEnd(int pos) const;
    bool match(const std::string& stuff);
    bool matched();
    int matchesCount();
    Sqrat::Array getSubStrings();
protected:
    std::shared_ptr<pcrepp::Pcre> pcre_;

};

/* @cond PRIVATE */
RegularExpression CreateRegExp(const std::string& expression, const std::string& flags = "");
void RegisterRegularExpressionClass(Sqrat::SqratVM& vm);
/* @endcond */
}

#endif