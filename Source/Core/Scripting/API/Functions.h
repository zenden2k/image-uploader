#ifndef IU_CORE_SCRIPTAPI_FUNCTIONS_H
#define IU_CORE_SCRIPTAPI_FUNCTIONS_H

#include <string>
#include "../Squirrelnc.h"

class NetworkClient;
#ifdef random
#undef random
#endif

/** @file
@section Globals Global functions
@brief Global functions
Note that string type is UTF-8 encoded unless otherwise stated.
*/
namespace ScriptAPI {
    /* @cond PRIVATE */
    Sqrat::Object  IncludeScript(const std::string& filename);
    void RegisterFunctions(Sqrat::SqratVM& vm);
    
    void RegisterShortTranslateFunctions(Sqrat::SqratVM& vm);
    void CleanUpFunctions();
    /* @endcond */
    
    // List of API functions (do not call them)

    const std::string GetScriptsDirectory();
    /*!
    Returns name of the application's current language file (without .lng extension). For example: "English", "Russian".
    @since 1.3.1
    */
    const std::string GetAppLanguageFile();
    /**
    Returns application's current language. For example : "en", "ru".
    */
    const std::string GetAppLanguage();

    /**
    Returns application's current locale. For example : "en_US", "ru_RU".
    */
    const std::string GetAppLocale();
    std::string GetCurrentThreadId();
    const std::string ExtractFileName(const std::string& path);
    const std::string GetFileExtension(const std::string& path);
    const std::string GetTempDirectory();
    const std::string InputDialog(const std::string& text, const std::string& defaultValue);
    const std::string GetFileMimeType(const std::string& filename);
    bool ShellOpenUrl(const std::string& url);
    const std::string MessageBox(const std::string& message, const std::string &title, const std::string& buttons, const std::string& type);
    void DebugMessage(const std::string& msg, bool isResponseBody);
    const std::string Translate(const std::string& key, const std::string& originalText);
    const std::string AskUserCaptcha(NetworkClient* nm, const std::string& url);
    const std::string AnsiToUtf8(const std::string& str, int codepage);
    void sleep(int msec);
    const std::string JsonEscapeString(const std::string& src);
    const std::string ToJSON(Sqrat::Object  obj);
    Sqrat::Object ParseJSON(const std::string& json);

    int random();
    std::string HtmlEntitiesDecode(const std::string& src);
    const std::string GetFileContents(const std::string& filename);
    const std::string url_encode(const std::string &value);
    const std::string md5(const std::string& data);

    // fake functions, just for docs (not implemented)

    const std::string md5_file(const std::string& filename);
    const std::string sha1(const std::string& data);
    const std::string sha1_file(const std::string& filename);
    const std::string hmac_sha1(const std::string& key, const std::string& data, bool base64);
    const std::string ExtractFileNameNoExt(const std::string& fileName);
    const std::string ExtractFilePath(const std::string& fileName);
    bool CopyFile(const std::string& src, const std::string & dest, bool overwrite);
    bool CreateDirectory(const std::string& path_, unsigned int mode);
    bool FileExists(const std::string& fileName);
    bool MoveFileOrFolder(const std::string& from, const std::string& to);
    bool PutFileContents(const std::string& utf8Filename, const std::string& content);
    bool DeleteFile(const std::string& utf8Filename);
    Sqrat::Object include(const std::string& filename);
    void print(std::string arg);
    std::string Base64Encode(const std::string& data);
    std::string Base64Decode(const std::string& data);

    /** 
    Execute shell command. Parameter cmd is using default operating system locale (so it won't work with utf-8 strings on windows).
    Use Process class instead
    */
    int system(std::string cmd);
}

#endif