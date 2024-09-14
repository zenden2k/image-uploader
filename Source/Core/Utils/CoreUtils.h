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

#ifndef IU_CORE_UTILS_H
#define IU_CORE_UTILS_H

#pragma once

#include <cstdio>
#include <string>
#include <thread>
#include <functional>

#include "CoreTypes.h"

#if defined(_MSC_VER) && _MSC_VER < 1800 
long round(float number);
#endif

#ifdef _WIN32
int gettimeofday(struct timeval * tp, struct timezone * tzp);
#endif

template <typename T>
class defer {
    std::function<T()> mFunctor;
public:
    defer& operator=(const defer&) = delete;
    defer(const defer&) = delete;

    defer(std::function<T()> functor) : mFunctor(functor) {}
    ~defer() {
        try {
            mFunctor();
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
        }
    }
};

struct freer
{
    void operator()(void* p) const noexcept {
        std::free(p);
    }
};

template<class T>
using unique_c_ptr = std::unique_ptr<T, freer>;

template<class T>
[[nodiscard]] unique_c_ptr<T>
make_unique_malloc(std::size_t size) noexcept
{
    static_assert(std::is_trivial_v<T>);
    return unique_c_ptr<T>{static_cast<T*>(std::malloc(size))};
}
namespace IuCoreUtils
{
    // A version of fopen() function which supports utf8 file names
    FILE * FopenUtf8(const char * filename, const char * mode);
    int Fseek64(FILE *stream, int64_t offset, int origin);
    int64_t Ftell64(FILE *a);

    bool FileExists(const std::string& fileName);
    bool DirectoryExists(const std::string& path);
    std::string ExtractFilePath(const std::string& fileName);
    std::string ExtractFileName(const std::string& fileName);
    std::string ExtractFileExt(const std::string& fileName);
    std::string ExtractFileNameNoExt(const std::string& fileName);
    std::string ExtractFileNameFromUrl(const std::string& url);
    std::string IncrementFileName(const std::string& originalFileName, int counter);
    std::string ToString(double value, int precision);
    std::string Utf8ToSystemLocale(const std::string& str);
    std::string SystemLocaleToUtf8(const std::string& str);
    std::string Int64ToString(int64_t value);
    int64_t StringToInt64(const std::string& str);
    std::string GetFileMimeType(const std::string&);
    std::string GetFileMimeTypeByName(const std::string& fileName);
    std::string GetDefaultExtensionForMimeType(const std::string&);
    std::string StrReplace(std::string text, std::string s, std::string d);
    std::string ConvertToUtf8(const std::string &text, const std::string& codePage);
    bool ReadUtf8TextFile(const std::string& utf8Filename, std::string& data);
    bool PutFileContents(const std::string& utf8Filename, const std::string& content);
    std::string GetFileContents(const std::string& filename);

    // This function retrieves the size of the specified file, in bytes.
    // It supports large files; filename must be utf8 encoded
    int64_t GetFileSize(const std::string& utf8Filename);
    std::wstring Utf8ToWstring(const std::string &str);
    std::string WstringToUtf8(const std::wstring &str);

    // Convert UTF16-LE encoded string to Utf-8
    std::string Utf16ToUtf8(const std::u16string& src);

    std::string TimeStampToString(time_t t);
    std::string FileSizeToString(int64_t nBytes);
    bool CreateDir(const std::string& path, unsigned int mode=0);
    bool CopyFileToDest(const std::string& src, const std::string & dest, bool overwrite = true);
    std::string Utf8ToAnsi(const std::string &str, int codepage);
    bool RemoveFile(const std::string& utf8Filename);
    bool MoveFileOrFolder(const std::string& from ,const std::string& to);
    std::string ThreadIdToString(const std::thread::id& id);
    void DatePlusDays(struct tm* date, int days);

    using ThreadExitFunctionPointer = void(*)();
    void OnThreadExit(ThreadExitFunctionPointer func);
};
#endif
