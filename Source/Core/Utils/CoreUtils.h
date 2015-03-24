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

#ifndef _IU_CORE_UTILS_H
#define _IU_CORE_UTILS_H

#include <cstdio>
#include <string>
#include "CoreTypes.h"

#if !defined(_MSC_VER) || _MSC_VER < 1800 

long round(float number);
#endif


namespace IuCoreUtils
{
	// A version of fopen() function which supports utf8 file names
	FILE * fopen_utf8(const char * filename, const char * mode);

	bool FileExists(const Utf8String& fileName);
	bool DirectoryExists(const Utf8String path);
	const Utf8String ExtractFilePath(const Utf8String& fileName);
	Utf8String ExtractFileName(const Utf8String fileName);
	Utf8String ExtractFileExt(const Utf8String fileName);
	const Utf8String ExtractFileNameNoExt(const Utf8String& fileName);
	Utf8String toString(int value);
	Utf8String toString(unsigned int value);
	Utf8String toString(double value, int precision);
   std::string Utf8ToSystemLocale(const Utf8String& str);
	std::string SystemLocaleToUtf8(const Utf8String& str);
	Utf8String int64_tToString(int64_t value);
	int64_t stringToint64_t(const Utf8String fileName);
	Utf8String GetFileMimeType(const Utf8String);
	Utf8String GetDefaultExtensionForMimeType(const Utf8String);
	Utf8String StrReplace(Utf8String text, Utf8String s, Utf8String d);
	Utf8String ConvertToUtf8(const Utf8String &text, const Utf8String codePage);
	bool ReadUtf8TextFile(Utf8String utf8Filename, Utf8String& data);
	bool PutFileContents(const Utf8String& utf8Filename, const Utf8String& content);

	// getFileSize retrieves the size of the specified file, in bytes.
	// It supports large files; filename must be utf8 encoded
	int64_t getFileSize(Utf8String utf8Filename);
	const std::wstring Utf8ToWstring(const Utf8String &str);
	const Utf8String WstringToUtf8(const std::wstring &str);

	const std::string timeStampToString(time_t t);
	Utf8String fileSizeToString(int64_t nBytes);
	bool createDirectory(const Utf8String& path, unsigned int mode=0);
	bool copyFile(const std::string& src, const std::string & dest, bool overwrite = true);
	const std::string Utf8ToAnsi(const std::string &str, int codepage);
	bool RemoveFile(const Utf8String& utf8Filename);
	bool MoveFileOrFolder(const Utf8String& from ,const Utf8String& to);
};
#endif
