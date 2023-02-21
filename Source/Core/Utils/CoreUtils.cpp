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

#include <ctime>
#include <locale>
#include <cstdio>
#include <filesystem>
#include <map>
#include <string>

#ifdef _WIN32
    #include <WinSock.h>
    #include "Func/WinUtils.h"
#else
    #include <boost/filesystem.hpp>
    #ifdef __APPLE__
        #include <sys/uio.h>
    #else
        #include <sys/io.h>
    #endif
    #include <sys/stat.h>

#endif

#include "Core/3rdpart/UriParser.h"


#if defined(_MSC_VER) && _MSC_VER < 1800 
long round(float number){
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}
#endif

#ifdef _WIN32

/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);

/*
* PostgreSQL's implementation of gettimeofday for windows:
*
* timezone information is stored outside the kernel so tzp isn't used anymore.
*
* Note: this function is not for Win32 high precision timing purpose. See
* elapsed_time().
*/
int
gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = static_cast<long>((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = static_cast<long>(system_time.wMilliseconds * 1000);

    return 0;
}
#endif

namespace IuCoreUtils {

FILE * FopenUtf8(const char * filename, const char * mode)
{
#ifdef _MSC_VER
    return _wfopen(Utf8ToWstring(filename).c_str(), Utf8ToWstring(mode).c_str());
#else
    return fopen(Utf8ToSystemLocale(filename).c_str(), mode);
#endif
}

int Fseek64(FILE* stream, int64_t offset, int origin)
{
#ifdef _MSC_VER
    return _fseeki64(stream, offset, origin);
#else
    return fseeko64(stream, offset, origin);
#endif
}

int64_t Ftell64(FILE *a)
{
#ifdef __CYGWIN__
    return ftell(a);
#elif defined (_WIN32)
    return _ftelli64(a);
#else
    return ftello(a);
#endif
}

bool FileExists(const std::string& fileName)
{
    #ifdef WIN32
    DWORD res = GetFileAttributes(Utf8ToWstring(fileName).c_str());
    if (res == static_cast<DWORD>(-1)) {
        switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return false;
        case ERROR_ACCESS_DENIED:
            return true;
        default:
            return false;
        }
    }
    #else
        return boost::filesystem::exists(fileName);
    #endif
    return true;
}

typedef std::codecvt_base::result res;
typedef std::codecvt<wchar_t, char, mbstate_t> codecvt_type;
std::mbstate_t state;

std::codecvt_base::result fromWstring (const std::wstring & str,
   const std::locale & loc, std::string & out)
{
  const codecvt_type& cdcvt = std::use_facet<codecvt_type>(loc);
  std::codecvt_base::result r;

  const wchar_t *in_next = 0;
  char *out_next = 0;

  std::wstring::size_type len = str.size () << 2;

  char * chars = new char [len + 1];

  r = cdcvt.out (state, str.c_str (), str.c_str () + str.size (), in_next,
                 chars, chars + len, out_next);
  *out_next = '\0';
  out = chars;

  delete [] chars;

  return r;
}

std::codecvt_base::result toWstring (const std::string & str, 
   const std::locale & loc, std::wstring & out)
{ 
    const codecvt_type& cdcvt = std::use_facet<codecvt_type>(loc);
    std::codecvt_base::result r;
  
    wchar_t * wchars = new wchar_t[str.size() + 1];
 
    const char *in_next = 0;
    wchar_t *out_next = 0;
  
    r = cdcvt.in (state, str.c_str (), str.c_str () + str.size (), in_next,
                wchars, wchars + str.size () + 1, out_next);
    *out_next = '\0';
    out = wchars;    
  
    delete [] wchars;
  
    return r;
}

std::string SystemLocaleToUtf8(const std::string& str)
{
    std::locale const oloc = std::locale ("");
    std::wstring out;
    std::wstring wideStr;
    toWstring (str, oloc, wideStr);
    return WstringToUtf8(wideStr);
}

std::string Utf8ToSystemLocale(const std::string& str)
{
   std::wstring wideStr = Utf8ToWstring(str);
   std::locale const oloc = std::locale ("");
   std::string out;
   /*std::codecvt_base::result r = */fromWstring (wideStr, oloc, out);
   return out;
}

std::string ExtractFileName(const std::string& fileName)
{
        std::string temp = fileName;
        int Qpos = temp.find_last_of('?'); //FIXME
        if(Qpos>=0) temp = temp.substr(0, Qpos-1);
        int i,len = temp.length();
        for(i=len-1; i>=0; i--)
        {
            if(temp[i] == '\\' || temp[i]=='/')
                break;
        }
        return temp.substr(i+1);
}

std::string ExtractFileExt(const std::string& fileName)
{
    int nLen = fileName.length();

    std::string result;
    for( int i=nLen-1; i>=0; i-- )
    {
        if(fileName[i] == '.')
        {
            result = fileName.substr(i + 1);
            break;
        }
        else if(fileName[i] == '\\' || fileName[i] == '/') break;
    }
    return result;
}

std::string ExtractFilePath(const std::string& fileName)
{
    int i, len = fileName.length();
    for(i = len-1; i >= 0; i--)
    {
        if(fileName[i] == '\\' || fileName[i]=='/')
        {
            return fileName.substr(0, i+1);
        }
            
    }
    return std::string();
}

std::string ExtractFileNameNoExt(const std::string& fileName)
{
    std::string result = ExtractFileName(fileName);
    size_t Qpos = result.find_last_of('.');
    if (Qpos != std::string::npos) {
        result = result.substr(0, Qpos);
    }
    return result;
}

std::string ExtractFileNameFromUrl(const std::string& url)
{
    const uriparser::Uri uri(url);
    return ExtractFileName(uri.path());
}

std::string IncrementFileName(const std::string& originalFileName, int counter) {
    const std::string ext = ExtractFileExt(originalFileName);
    std::string name = ExtractFileNameNoExt(originalFileName);
    name += "(" + std::to_string(counter) + ")";
    if (!ext.empty()) {
        name += "." + ext;
    }
    return name;
}

std::string StrReplace(std::string text, std::string s, std::string d)
{
    for(size_t index=0; index=text.find(s, index), index!=std::string::npos;)
    {
        text.replace(index, s.length(), d);
        index += d.length();
    }
    return text;
}

bool ReadUtf8TextFile(const std::string& utf8Filename, std::string& data)
{
    FILE *stream = FopenUtf8(utf8Filename.c_str(), "rb");
    if (!stream) {
        return false;
    }
    fseek(stream, 0L, SEEK_END);
    size_t size = ftell(stream);
    rewind(stream);

    unsigned char buf[3]={0,0,0};
    size_t bytesRead = fread(buf, 1, 3, stream);    

    if (bytesRead == 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) // UTF8 Byte Order Mark (BOM)
    {    
        size -= 3;    
    }
    else if (bytesRead >=2 && buf[0] == 0xFF && buf[1] == 0xFE) {
        // UTF-16LE encoding
        size -= 2;
        fseek( stream, 2L,  SEEK_SET );
        std::u16string res;
        int charCount = size/2;
        res.resize(charCount);
        size_t charsRead = fread(&res[0], 2, charCount, stream);    
        res[charsRead]=0;
        fclose(stream);
        data = Utf16ToUtf8(res);
        return true;
    } 
    else {
        // no BOM was found; seeking backward
        fseek( stream, 0L,  SEEK_SET );
    }
    try {
        data.resize(size);
    } catch ( std::exception& ex ) {
        LOG(ERROR) << ex.what();
        fclose(stream);
        return false;
    }
   
    size_t bytesRead2 = fread(&data[0], 1, size, stream); 
    if (bytesRead2 == size) {
        fclose(stream);
        return true;
    }

    fclose(stream);
    return false;
}

bool PutFileContents(const std::string& utf8Filename, const std::string& content)
{
    FILE *stream = FopenUtf8(utf8Filename.c_str(), "wb");
    if (!stream) {
        return false;
    }
    fwrite(&content[0], 1, content.length(), stream);
    fclose(stream);
    return true;
}

std::string GetFileContents(const std::string& filename) {
    std::string data;
    FILE *stream = IuCoreUtils::FopenUtf8(filename.c_str(), "rb");
    if (!stream) return std::string();
    fseek(stream, 0L, SEEK_END);
    size_t size = ftell(stream);
    rewind(stream);

    //size_t size = static_cast<size_t>(IuCoreUtils::getFileSize(filename));

    try {
        data.resize(size);
    } catch (std::exception& ex) {
        LOG(ERROR) << "Unable to allocate " << size << " bytes:" << ex.what();
        fclose(stream);
        return std::string();
    }

    size_t bytesRead = fread(&data[0], 1, size, stream);
    if (bytesRead != size) {
        data.resize(bytesRead);
    }
    fclose(stream);
    return data;
}

std::string TimeStampToString(time_t t)
{
    char buf[256]{0};
    // TODO: add system locale support
    tm * timeinfo = localtime ( &t );
    if (timeinfo) {
        sprintf(buf, "%02d.%02d.%04d %02d:%02d:%02d", (int)timeinfo->tm_mday, (int)timeinfo->tm_mon + 1, (int)1900 + timeinfo->tm_year, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }
    return buf;
}

std::string Int64ToString(int64_t value)
{
    return std::to_string(value);
}

int64_t StringToInt64(const std::string& str)
{
    return strtoll(str.c_str(), nullptr, 10);
}

int64_t GetFileSize(const std::string& utf8Filename)
{
#ifdef _WIN32
   #ifdef _MSC_VER
	struct _stat64  stats;
   #else
      _stati64 stats;
   #endif
    memset(&stats, 0, sizeof(stats));
    stats.st_size = -1;
    std::wstring wideFileName = Utf8ToWstring(utf8Filename);
    if(_wstati64(wideFileName.c_str(), &stats)!=0) {
        int err = errno;
        switch (err) {
            case ENOENT: 
                LOG(WARNING) << "Call to _wstati64 failed. File not found." << std::endl <<  utf8Filename; 
            break;
            case EINVAL: 
                LOG(WARNING) << "Call to _wstati64 failed. Invalid parameter.";
            break;
            default: /* Should never be reached. */
                LOG(ERROR) << "Call to _wstati64 failed. Unexpected error in _wstati64.";
        }

        if (err != ENOENT) {
            WIN32_FILE_ATTRIBUTE_DATA fad;
            memset(&fad, 0, sizeof(fad));

            if (!GetFileAttributesEx(wideFileName.c_str(), GetFileExInfoStandard, &fad)) {
                DWORD lastError = GetLastError();
                LOG(ERROR) << WinUtils::FormatWindowsErrorMessage(lastError);
                return -1;
            }
            LARGE_INTEGER size;
            size.HighPart = fad.nFileSizeHigh;
            size.LowPart = fad.nFileSizeLow;
            return size.QuadPart;
        }
        return -1;
   }
#else
   struct stat64 stats;
   std::string path = Utf8ToSystemLocale(utf8Filename);
   if(-1 == stat64(path.c_str(), &stats))
   {
      return -1;
   }
#endif
     return stats.st_size;
}

std::string FileSizeToString(int64_t nBytes)
{
    double number = 0;
    std::string postfix;
    int precision = 0;
    if(nBytes < 0)
    {
        return "n/a";
    }

    if(nBytes < 1024)
    {
        number = double(nBytes);
        postfix = "bytes";
    }
    else if( nBytes < 1048576)
    {
        number = (double)nBytes / 1024.0;
        postfix = "KB";
    }
    else if(nBytes<((int64_t)1073741824)) /*< 1 GB*/
    {
        postfix= "MB";
        number= (double)nBytes / 1048576.0;
        precision = 1;
    }
    else if(nBytes>=1073741824)
    {
        postfix= "GB";
        precision = 1;
        number = (double)nBytes / 1073741824.0;
    }
    return ToString(number, precision) + " " + postfix;
}

std::string ToString(double value, int precision)
{
    char buffer[100];
    sprintf(buffer, ("%0." + std::to_string(precision) + "f").c_str(), value);
    return buffer;
}

std::string GetDefaultExtensionForMimeType(const std::string& mimeType) {
	if (mimeType == "image/gif") {
        return "gif";
	}
	if (mimeType == "image/png") {
        return "png";
	}
	if (mimeType == "image/jpeg") {
        return "jpg";
    }
	if (mimeType == "image/webp") {
	    return "webp";
    }
    return {};
}

std::string ThreadIdToString(const std::thread::id& id)
{
    std::stringstream threadIdSS;
    threadIdSS << id;
    return threadIdSS.str();
}

void DatePlusDays(struct tm* date, int days){
    const time_t ONE_DAY = 24 * 60 * 60;

    // Seconds since start of epoch
    time_t date_seconds = mktime(date) + (days * ONE_DAY);

    // Update caller's date
    // Use localtime because mktime converts to UTC so may change date
    *date = *localtime(&date_seconds);
}

bool CopyFileToDest(const std::string& src, const std::string& dest, bool overwrite)
{
    try {
        std::filesystem::copy(std::filesystem::u8path(src), std::filesystem::u8path(dest),
            overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none
        );
        return true;
    } catch (const std::filesystem::filesystem_error&) {
    }
    return false;
}

bool RemoveFile(const std::string& utf8Filename) {
    try {
        return std::filesystem::remove(std::filesystem::u8path(utf8Filename));
    } catch (const std::filesystem::filesystem_error&) {
    }
    return false;
}

} // end of namespace IuCoreUtils
