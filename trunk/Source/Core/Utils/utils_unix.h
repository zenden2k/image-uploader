/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _CORE_UTILS_UTILS_UNIX_H_
#define _CORE_UTILS_UTILS_UNIX_H_

#include <stdio.h>
#include <sstream>  // ostringstream
#include <iostream>
#include <string>
#include "Core/3rdpart/utf8.h"
#include "Core/Utils/StringUtils.h"

namespace IuCoreUtils {

const std::wstring Utf8ToWstring(const std::string &str) {
	using namespace utf8;
	std::wstring res;
	try {
		if (sizeof(wchar_t) == 2)
			utf8to16(str.begin(), str.end(), back_inserter(res));
		else if (sizeof(wchar_t) == 4)
			utf8to32(str.begin(), str.end(), back_inserter(res));
	}
	catch(...) { }
	return res;
}

const Utf8String WstringToUtf8(const std::wstring &str) {
	using namespace utf8;
	std::string res;
	try {
      if (sizeof(wchar_t) == 2)
         utf16to8(str.begin(), str.end(), back_inserter(res));
      else if (sizeof(wchar_t) == 4)
          utf16to8(str.begin(), str.end(), back_inserter(res));
	}
	catch(...) { }
	return res;
}

Utf8String ConvertToUtf8(const Utf8String &text, const Utf8String codePage) {
	// FIXME: stub
	return text;
}

Utf8String GetFileMimeType(const Utf8String name)
{
	std::string defaultType = "application/octet-stream";
	FILE* stream = popen(Utf8ToSystemLocale("file -b --mime-type '" + name + "'").c_str(), "r");
	if(!stream)
		return defaultType;
    std::ostringstream output;

	while(!feof(stream) && !ferror(stream)) {
		char buf[128];
		int bytesRead = fread(buf, 1, 128, stream);
		output.write(buf, bytesRead);
	}
	pclose(stream);
	std::string result = IuStringUtils::Trim(output.str());
	return result;
}
}

#endif  // _CORE_UTILS_UTILS_UNIX_H_
