/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>

    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "StringUtils.h"
 
#include <ctype.h>
#include <cstdio>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif

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
		res = "";
	}
	else
		res = str.substr( startpos, endpos - startpos + 1 );
	return res;
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
			tokens.push_back(str.substr(lastPos, str.length()));
			break;
		}
		else
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

Utf8String Replace(const Utf8String& text, const Utf8String& s, const Utf8String& d)
{
	std::string result = text;
	for (unsigned index = 0; index = result.find(s, index), index != std::string::npos; )
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
	for (; *s1 && *s2 && (toupper((unsigned char)*s1) == toupper((unsigned char)*s2)); ++s1, ++s2)
		;
	return *s1 - *s2;
#endif
}

std::string toLower(const std::string& str)
{
	std::string s1 = str;
	for (int i = 0; i < s1.length(); i++)
		s1[i] = ::tolower(s1[i]);
	// std::string s1;
	// std::transform(str.begin(), str.end(), std::back_inserter(s1), std::tolower);
	return s1;
}
}
