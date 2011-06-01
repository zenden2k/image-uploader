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

#ifndef IU_CORE_UTILS_STRINGUTILS_H
#define IU_CORE_UTILS_STRINGUTILS_H

#include <cstdio>
#include <string>
#include <vector>
#include "CoreTypes.h"

namespace IuStringUtils
{
	std::string Trim(const std::string& str);
	Utf8String Replace(const Utf8String& text, const Utf8String& s, const Utf8String& d);
	void Split(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount);
	
	// Current version of toLower works only with ASCII strings
	std::string toLower(const std::string& str);

	//  The stricmp() function compares the two strings s1 and s2, 
	//  ignoring the case of the characters. It returns an integer less than, 
	//	 equal to, or greater than zero if s1 is found, respectively, to be less than, 
	//	 to match, or be greater than s2.
	int stricmp(const char *s1, const char *s2);
};

#endif
