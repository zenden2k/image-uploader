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
#ifndef _CORE_UTILS_UTILS_UNIX_H_
#define _CORE_UTILS_UTILS_UNIX_H_

#include <stdio.h>
#include <sstream>  // ostringstream
#include <iostream>
#include <string>
#include "Core/3rdpart/utf8.h"
#include "Core/Utils/StringUtils.h"
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <string.h>
#include <sys/stat.h>   

typedef struct stat Stat;

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

bool copyFile(const std::string& from, const std::string & to, bool overwrite)
{
    const int BUFSIZE = 64 * 1024;
    char buf[BUFSIZ];
    size_t size;

    FILE* source = fopen_utf8(from.c_str(), "rb");
    if ( !source ) {
        return false;
    }
    if ( !overwrite && FileExists(to)) {
        return false;
    }
    FILE* dest = fopen_utf8(to.c_str(), "wb");
    if ( !dest ) {
        return false;
    }

    while (size = fread(buf, 1, BUFSIZE, source)) {
        fwrite(buf, 1, size, dest);
    }

    fclose(source);
    fclose(dest);
    return true;
}


static int do_mkdir(const char *path, mode_t mode)
{
    Stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
int mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(copypath);
    return (status);
}

bool createDirectory(const Utf8String& path,unsigned int mode)
{
    return mkpath(path.c_str(), (mode_t)mode) == 0;
}

bool RemoveFile(const Utf8String& utf8Filename) {
    return remove(utf8Filename.c_str())==0;
}

bool MoveFileOrFolder(const Utf8String& from ,const Utf8String& to) {
    return rename(from.c_str() ,to.c_str())==0;
}

}

#endif  // _CORE_UTILS_UTILS_UNIX_H_
