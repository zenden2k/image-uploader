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
#include <fstream>
#include <system_error>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <sstream>

#ifdef _WIN32
    #include <WinSock.h>
    #include "Func/WinUtils.h"
#else
    #ifdef __APPLE__
        #include <sys/uio.h>
    #else
        //#include <sys/io.h>
    #endif
    #include <sys/stat.h>
#endif

#include "Core/3rdpart/UriParser.h"
#include "Core/3rdpart/xdgmime/xdgmime.h"
#include "MimeTypeHelper.h"
#include "StringUtils.h"

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

bool FileExists(const std::string& fileName) {
    try {
        return std::filesystem::exists(std::filesystem::u8path(fileName));
    } catch (const std::exception&) {
        return false;
    }
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

std::string ExtractFileName(const std::string& fileName) {
    /*std::string temp = fileName;
    /*int Qpos = temp.find_last_of('?'); 
    if(Qpos>=0) temp = temp.substr(0, Qpos-1);*/
    int i, len = fileName.length();
    for (i = len - 1; i >= 0; i--) {
        if (fileName[i] == '\\' || fileName[i] == '/') {
            break;
        }
    }
    return fileName.substr(i + 1);
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

std::string GetFileContentsEx(const std::string& filename, int64_t offset, size_t size, bool allowPartialRead) {
    namespace fs = std::filesystem;

    fs::path filepath = fs::u8path(filename); 

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), "Failed to open file: " + filename);
    }

    file.seekg(0, std::ios::end);
    const int64_t fileSize = file.tellg();
    if (offset < 0 || offset > fileSize) {
        throw std::out_of_range("Invalid file offset");
    }

    if (size == 0) {
        return {};
    }

    size_t bytesAvailable = static_cast<size_t>(fileSize - offset);
    if (!allowPartialRead && size > bytesAvailable) {
        throw std::out_of_range("Requested size exceeds available data");
    }
    size_t read_size = (allowPartialRead) ? std::min(size, bytesAvailable) : size;

    file.seekg(offset, std::ios::beg);
    std::string buffer(read_size, '\0');
    file.read(buffer.data(), read_size);

    if (!file) {
        throw std::runtime_error("Failed to read the requested amount of data");
    }

    return buffer;
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

int64_t StringToInt64(const std::string& str)
{
    return strtoll(str.c_str(), nullptr, 10);
}

int64_t GetFileSize(const std::string& utf8Filename)
{
    try {
        return std::filesystem::file_size(std::filesystem::u8path(utf8Filename));
    } catch (const std::filesystem::filesystem_error&) {
        return -1;
    }
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
    } else /*if (nBytes >= 1073741824)*/
    {
        postfix= "GB";
        precision = 1;
        number = (double)nBytes / 1073741824.0;
    }
    return ToString(number, precision) + " " + postfix;
}

std::string ToString(double value, int precision)
{
    std::ostringstream stream;
    stream << std::fixed;
    stream << std::setprecision(precision);
    stream << value;
    return stream.str();
}

std::string GetDefaultExtensionForMimeType(const std::string& mimeType) {
    return MimeTypeHelper::getDefaultExtensionForMimeType(mimeType);
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

void OnThreadExit(void (*func)()) {
    class ThreadExiter
    {
        std::set<ThreadExitFunctionPointer> exitFuncs_;
    public:
        ThreadExiter() = default;
        ThreadExiter(ThreadExiter const&) = delete;
        void operator=(ThreadExiter const&) = delete;
        ~ThreadExiter()
        {
            for(auto func: exitFuncs_) {
                func();
            }
            exitFuncs_.clear();
            /*while (!exit_funcs.empty())
            {
                exit_funcs.top()();
                exit_funcs.pop();
            }*/
        }
        void add(ThreadExitFunctionPointer func)
        {
            exitFuncs_.emplace(func);
        }
    };

    thread_local ThreadExiter exiter;
    exiter.add(func);
}

std::mutex xdgMimeMutex;
constexpr auto DefaultMimeType = "application/octet-stream";

std::string GetFileMimeType(const std::string& fileName) {
    std::lock_guard<std::mutex> lk(xdgMimeMutex);
    /* FILE* f = FopenUtf8(fileName.c_str(), "rb");
    if (!f) {
        return DefaultMimeType;
    }
    char buffer[256] {};
    size_t readBytes = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
    int resultPrio = 0;*/
   
    /*auto* mime = xdg_mime_get_mime_type_for_data(buffer, readBytes, &resultPrio);*/
    struct stat st;
   
    auto* mime = xdg_mime_get_mime_type_for_file(fileName.c_str(), &st);
    if (!mime) {
        return DefaultMimeType;
    }
    std::string result = mime;
    /* if (result == DefaultMimeType) {
        return GetFileMimeTypeByName(fileName)
    }*/

    /* if (result == "image/x-png") {
        result = "image/png";
    } else if (result == "image/pjpeg") {
        result = "image/jpeg";
    } else if (result == "application/octet-stream") {
        if (byBuff[0] == 'R' && byBuff[1] == 'I' && byBuff[2] == 'F' && byBuff[3] == 'F'
            && byBuff[8] == 'W' && byBuff[9] == 'E' && byBuff[10] == 'B' && byBuff[11] == 'P') {
            result = "image/webp";
        }
    }*/

    return result;
}

std::string GetFileMimeTypeByName(const std::string& fileName) {
    std::lock_guard<std::mutex> lk(xdgMimeMutex);
    struct stat st;
    auto* mime = xdg_mime_get_mime_type_for_file(fileName.c_str(), &st);
    if (!mime) {
        return DefaultMimeType;
    }
    return mime;
}

std::string GetFileMimeTypeByContents(const std::string& fileName) {
    FILE* f = FopenUtf8(fileName.c_str(), "rb");
    if (!f) {
        return DefaultMimeType;
    }
    char buffer[256] {};
    size_t readBytes = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
    int resultPrio = 0;
    std::lock_guard<std::mutex> lk(xdgMimeMutex);
    auto* mime = xdg_mime_get_mime_type_for_data(buffer, readBytes, &resultPrio);

    if (!mime) {
        return DefaultMimeType;
    }
    return mime;
}

std::string GenerateRandomFilename(const std::string& path, int suffixLen)
{
    if (!suffixLen) {
        suffixLen = 8;
    }
    int i, len = path.length();
    if (!len) {
        return IuStringUtils::RandomString(suffixLen);
    }
    for (i = len - 1; i >= 0; i--) {
        if (path[i] == '\\' || path[i] == '/') {
            break;
        }
    }
    std::string filename = path.substr(i + 1);
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        return path.substr(0, i + 1) + filename.substr(0, dotPos) + "_" + IuStringUtils::RandomString(suffixLen) + filename.substr(dotPos);
    }
    return path.substr(0, i + 1) + filename + "_" + IuStringUtils::RandomString(suffixLen);
}

} // end of namespace IuCoreUtils
