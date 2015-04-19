/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "Core/3rdpart/pcreplusplus.h"
#include <string>
#include "../Squirrelnc.h"
#include "Core/Utils/CoreTypes.h"
namespace ScriptAPI {


class RegularExpression  {
public:
	//  Pcre(const std::string& expression);
	RegularExpression(); 
	//RegularExpression(const RegularExpression& r);
	RegularExpression(const std::string& expression, const std::string& flags);
	  Sqrat::Array split(const std::string& piece);
	  Sqrat::Array splitWithLimit(const std::string& piece, int limit);
	  Sqrat::Array splitWithLimitOffset(const std::string& piece, int limit, int start_offset);
	  Sqrat::Array splitWithLimitStartEndOffset(const std::string& piece, int limit, int start_offset, int end_offset);

	  const std::string getMatch(int pos) const;
	  bool search(const std::string& stuff);
	  bool searchWithOffset(const std::string& stuff, int OffSet);
      Sqrat::Array findAll(const std::string& stuff);
	  const std::string replace(const std::string& piece, const std::string& with);
	  int getEntireMatchStart() const;
	  int getMatchStart(int pos) const;
	  int getEntireMatchEnd() const;
	  int getMatchEnd(int pos) const;
	  bool match(const std::string& stuff);
	  bool matched();
	  int  matchesCount();
	  Sqrat::Array getSubStrings();
protected:
	std_tr::shared_ptr<pcrepp::Pcre> pcre_;

};

RegularExpression CreateRegExp(const std::string& expression, const std::string& flags = "");
void RegisterRegularExpressionClass(Sqrat::SqratVM& vm);
}

#endif