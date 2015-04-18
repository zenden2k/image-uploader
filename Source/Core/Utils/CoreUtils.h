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

#ifndef _IU_CORE_UTILS_H
#define _IU_CORE_UTILS_H

#pragma once
#include <cstdio>
#include <string>
#include "CoreTypes.h"

#if defined(_MSC_VER) && _MSC_VER < 1800 

long round(float number);
#endif

#ifdef _WIN32

int gettimeofday(struct timeval * tp, struct timezone * tzp);
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
