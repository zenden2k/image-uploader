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
#include <thread>

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
    int fseek_64(FILE *stream, int64_t offset, int origin);

    bool FileExists(const std::string& fileName);
    bool DirectoryExists(const std::string path);
    const std::string ExtractFilePath(const std::string& fileName);
    std::string ExtractFileName(const std::string fileName);
    std::string ExtractFileExt(const std::string fileName);
    const std::string ExtractFileNameNoExt(const std::string& fileName);
    std::string ExtractFileNameFromUrl(const std::string fileName);
    std::string incrementFileName(const std::string& originalFileName, int counter);
    std::string toString(int value);
    std::string toString(unsigned int value);
    std::string toString(double value, int precision);
    std::string Utf8ToSystemLocale(const std::string& str);
    std::string SystemLocaleToUtf8(const std::string& str);
    std::string int64_tToString(int64_t value);
    int64_t stringToInt64(const std::string fileName);
    std::string GetFileMimeType(const std::string);
    std::string GetDefaultExtensionForMimeType(const std::string);
    std::string StrReplace(std::string text, std::string s, std::string d);
    std::string ConvertToUtf8(const std::string &text, const std::string codePage);
    bool ReadUtf8TextFile(std::string utf8Filename, std::string& data);
    bool PutFileContents(const std::string& utf8Filename, const std::string& content);
    const std::string GetFileContents(const std::string& filename);

    // getFileSize retrieves the size of the specified file, in bytes.
    // It supports large files; filename must be utf8 encoded
    int64_t getFileSize(std::string utf8Filename);
    const std::wstring Utf8ToWstring(const std::string &str);
    const std::string WstringToUtf8(const std::wstring &str);

    const std::string timeStampToString(time_t t);
    std::string fileSizeToString(int64_t nBytes);
    bool createDirectory(const std::string& path, unsigned int mode=0);
    bool copyFile(const std::string& src, const std::string & dest, bool overwrite = true);
    const std::string Utf8ToAnsi(const std::string &str, int codepage);
    bool RemoveFile(const std::string& utf8Filename);
    bool MoveFileOrFolder(const std::string& from ,const std::string& to);
    std::string ThreadIdToString(const std::thread::id& id);
};
#endif
