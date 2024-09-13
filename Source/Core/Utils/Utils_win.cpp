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

#include "CoreUtils.h"

#include <windows.h>
#include <ObjIdl.h>
#include "Core/3rdpart/codepages.h"

namespace IuCoreUtils
{

std::wstring strtows(const std::string &str, UINT codePage)
{
    std::wstring ws;
    int n = MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size() + 1), /*dst*/NULL, 0);
    if (n) {
        ws.reserve(n);
        ws.resize(n - 1);
        if (MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size() + 1), /*dst*/&ws[0], n) == 0)
            ws.clear();
    }
    return ws;
}

std::string wstostr(const std::wstring &ws, UINT codePage)
{
    // prior to C++11 std::string and std::wstring were not guaranteed to have their memory be contiguous,
    // although all real-world implementations make them contiguous
    std::string str;
    int srcLen = static_cast<int>(ws.size());
    int n = WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, NULL, 0, /*defchr*/0, NULL);
    if (n) {
        str.reserve(n);
        str.resize(n - 1);
        if (WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, &str[0], n, /*defchr*/0, NULL) == 0)
            str.clear();
    }
    return str;
}

std::string AnsiToUtf8(const std::string &str, int codepage)
{
    return wstostr(strtows(str, codepage), CP_UTF8);
}

std::string WstringToUtf8(const std::wstring &str)
{
    return wstostr(str, CP_UTF8);
}

std::wstring Utf8ToWstring(const std::string &str)
{
    return strtows(str, CP_UTF8);
}

std::string Utf8ToAnsi(const std::string &str, int codepage)
{
    return wstostr(strtows(str, CP_UTF8), codepage);
}

std::string Utf16ToUtf8(const std::u16string& src) {
    std::string str;
    int srcLen = static_cast<int>(src.size());
    int n = WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<LPCWSTR>(src.c_str()), srcLen + 1, NULL, 0, /*defchr*/0, NULL);
    if (n) {
        str.reserve(n);
        str.resize(n - 1);
        if (WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<LPCWSTR>(src.c_str()), srcLen + 1, &str[0], n, /*defchr*/0, NULL) == 0)
            str.clear();
    }
    return str;
}

std::string ConvertToUtf8(const std::string &text, const std::string& codePage)
{
    int codePageNum = CodepageByName(codePage);
    if (codePageNum != CP_UTF8) {
        return AnsiToUtf8(text, codePageNum);
    }

    return text;
}
/*
std::string GetFileMimeType(const std::string& fileName)
{
    std::string DefaultMimeType = "application/octet-stream";
    FILE* InputFile = FopenUtf8(fileName.c_str(), "rb");
    if (!InputFile) {
        return DefaultMimeType;
    }

    BYTE byBuff[256];
    auto nRead = static_cast<unsigned int>(fread(byBuff, 1, 256, InputFile));

    fclose(InputFile);

    PWSTR szMimeW = nullptr;

    if (NOERROR != FindMimeFromData(nullptr, nullptr, byBuff, nRead, nullptr, 0, &szMimeW, 0)) {
        return DefaultMimeType;
    }

    std::string result = WstringToUtf8(szMimeW);
    CoTaskMemFree(szMimeW);

    if (result == "image/x-png") {
        result = "image/png";
    } else if (result == "image/pjpeg") {
        result = "image/jpeg";
    } else if (result == "application/octet-stream") {
        if (byBuff[0] == 'R' && byBuff[1] == 'I' &&byBuff[2] == 'F' &&byBuff[3] == 'F'
            && byBuff[8] == 'W' && byBuff[9] == 'E' && byBuff[10] == 'B'&& byBuff[11] == 'P'
            ) {
            result = "image/webp";
        }
    }

    return result;
}*/

bool DirectoryExists(const std::string& path)
{
    DWORD dwFileAttributes = GetFileAttributes(Utf8ToWstring(path).c_str());
    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES && (dwFileAttributes &
        FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }
    return false;
}

bool CreateDir(const std::string& path_, unsigned int mode)
{
    std::string path = path_;
    if (path.empty()) return false;
    if (path[path.length() - 1] == '\\' || path[path.length() - 1] == '/') {
        path.resize(path.length() - 1);
    }
    std::wstring wstrFolder = Utf8ToWstring(path);

    DWORD dwAttrib = GetFileAttributes(wstrFolder.c_str());

    // already exists ?
    if (dwAttrib != 0xffffffff)
        return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

    // recursively create from the top down
    char* szPath = _strdup(path.c_str());
    char * p = 0;
    for (int i = path.length() - 1; i >= 0; i--) {
        if (szPath[i] == '\\' || szPath[i] == '/') {
            p = szPath + i;
            break;
        }
    }
    if (p) {
        // The parent is a dir, not a drive
        *p = '\0';

        // if can't create parent
        if (!CreateDir(szPath)) {
            free(szPath);
            return false;
        }
        free(szPath);

        if (!::CreateDirectory(wstrFolder.c_str(), NULL)) {
            //LOG(WARNING) << wstrFolder;
            return false;
        }
    }

    return TRUE;
}

bool MoveFileOrFolder(const std::string& from, const std::string& to) {
    return MoveFile(IuCoreUtils::Utf8ToWstring(from).c_str(), IuCoreUtils::Utf8ToWstring(to).c_str()) != FALSE;
}

}
