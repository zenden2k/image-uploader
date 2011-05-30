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

#ifndef _IU_CORE_UTILS_H
#define _IU_CORE_UTILS_H

#include <cstdio>
#include <string>
#include "CoreTypes.h"

namespace IuCoreUtils
{
	// A version of fopen() function which supports utf8 file names
	FILE * fopen_utf8(const char * filename, const char * mode);

	bool FileExists(const Utf8String fileName);
	bool DirectoryExists(const Utf8String path);
	Utf8String ExtractFilePath(const Utf8String fileName);
	Utf8String ExtractFileName(const Utf8String fileName);
	Utf8String ExtractFileExt(const Utf8String fileName);
	Utf8String ExtractFileNameNoExt(const Utf8String fileName);
	Utf8String toString(int value);
	Utf8String toString(unsigned int value);
	Utf8String toString(double value, int precision);
   std::string Utf8ToSystemLocale(const Utf8String& str);
	std::string SystemLocaleToUtf8(const Utf8String& str);
	Utf8String zint64ToString(zint64 value);
	zint64 stringTozint64(const Utf8String fileName);
	Utf8String GetFileMimeType(const Utf8String);
	Utf8String StrReplace(Utf8String text, Utf8String s, Utf8String d);
	Utf8String ConvertToUtf8(const Utf8String &text, const Utf8String codePage);
	bool ReadUtf8TextFile(Utf8String utf8Filename, Utf8String& data);

	// getFileSize retrieves the size of the specified file, in bytes.
	// It supports large files; filename must be utf8 encoded
	zint64 getFileSize(Utf8String utf8Filename);
	const std::wstring Utf8ToWstring(const Utf8String &str);
	const Utf8String WstringToUtf8(const std::wstring &str);
	const std::string CalcMD5Hash(const void* data, size_t size);
	const std::string CalcMD5Hash(const std::string &data);
	const std::string timeStampToString(time_t t);
	Utf8String fileSizeToString(zint64 nBytes);
	bool createDirectory(const Utf8String path);
	bool copyFile(const std::string& src, const std::string & dest, bool overwrite = true);
};
#endif
