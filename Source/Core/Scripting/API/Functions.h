#ifndef IU_CORE_SCRIPTAPI_FUNCTIONS_H
#define IU_CORE_SCRIPTAPI_FUNCTIONS_H

#include <string>
#include "../Squirrelnc.h"

class NetworkClient;

#undef random

/** @file Functions.h
@section Globals Global functions
@brief Global functions

Note that string type is UTF-8 encoded unless otherwise stated.

Standard functions can be found in the <a href="http://www.squirrel-lang.org/doc/squirrel3.html">documentation of the standard library</a>  
and <a href="http://www.squirrel-lang.org/doc/sqstdlib3.html">documentation of the Squirrel language</a>.
*/

//! Namespace containing functions and classes available in squirrel scripts
namespace ScriptAPI {
    /* @cond PRIVATE */
    Sqrat::Object  IncludeScript(const std::string& filename);
    void RegisterFunctions(Sqrat::SqratVM& vm);
    
    void RegisterShortTranslateFunctions(Sqrat::SqratVM& vm);
    void CleanUpFunctions();
    /* @endcond */
    
    // List of API functions (do not call them)

    /**
     * Returns path to squirrel scripts directory.
     * @since 1.3.1
     */
    const std::string GetScriptsDirectory();

    /**
    Returns name of the application's current language file (without .lng extension). For example: "English", "Russian".
    @since 1.3.1
    */
    const std::string GetAppLanguageFile();

    /**
    Returns application's current language. For example : "en", "ru".
    */
    const std::string GetAppLanguage();

    /**
     * Returns a table containing information about application's version. For example:
    @code
    {
        Major = 1,
        Minor = 3,
        Release = 1,
        Build = 4240,
        Gui = true
    }
    @endcode
    @since 1.3.1
     */
    Sqrat::Table GetAppVersion();
    
    /**
    Returns application's current locale. For example : "en_US", "ru_RU".
    */
    const std::string GetAppLocale();

    /**
     * Retrieves the thread identifier of the calling thread.
     */
    std::string GetCurrentThreadId();
    const std::string ExtractFileName(const std::string& path);
    const std::string GetFileExtension(const std::string& path);

    /**
     * Returns the directory for temporary files
     * @since 1.3.0
     */
    const std::string GetTempDirectory();

    /**
     * Displays a prompt in a dialog box, waits for the user to input text or click a button, 
     * and returns a String containing the contents of the text box.
     * @param defaultValue Default text displayed in text box
     */
    const std::string InputDialog(const std::string& text, const std::string& defaultValue);

    /**
     * Returns the MIME file type (for example, "text / html"), analyzing the contents of the file.
     * File must exist. 
     * (in Windows, the FindMimeFromData function is used. In unix, the command 'file -b --mime-type' is used.).
     */
    const std::string GetFileMimeType(const std::string& filename);

    /**
     * Opens a URL/file in a standard browser/associated application.
     * @since 1.2.9.4185
     */
    bool ShellOpenUrl(const std::string& url);

    /**
     * Shows a dialog box with a message
     * 
     *   <b>buttons</b> possible values: OK, YES_NO, YES_NO_CANCEL, ABORT_RETRY_IGNORE,CANCEL_TRY_CONTINUE, OK_CANCEL, RETRY_CANCEL
     *   
     *   <b>type</b> possible values: EXCLAMATION, WARNING, INFORMATION, QUESTION, ERROR
     *   
     *   @returns Title of the clicked button ( ABORT,CANCEL, CONTINUE, IGNORE, YES, NO, OK, TRY).
     */
    std::string MessageBox(const std::string& message, const std::string &title, const std::string& buttons, const std::string& type);

    /**
     * @brief Shows a dialog box with a diagnostic message.
     */
    void DebugMessage(const std::string& msg, bool isResponseBody);

    /**
     * Returns text translated into the currently selected language. 
     * Every user-visible string in the script must be wrapped into this function.
     * @ref Internalization
     */
    const std::string Translate(const std::string& key, const std::string& originalText);

    /**
     * @brief Shows the captcha input dialog. 
     * 
     * nm - network manager, 
     * url - address of the image with captcha. 
     * The return value is the text entered by the user.
     */
    const std::string AskUserCaptcha(NetworkClient* nm, const std::string& url);

    /**
     * Converts a string from ANSI encoding to UTF-8. 
     * 
     * You can find the codepage number you need here: 
     * 
     * https://raw.githubusercontent.com/zenden2k/image-uploader/master/Source/Core/3rdpart/CodePages.cpp
     *  
     * Available only in Windows.
     */
    const std::string AnsiToUtf8(const std::string& str, int codepage);

    /**
     * Converts a string from ANSI encoding to UTF-8.
     * 
     * You can find the codepage number you need here: 
     * 
     * https://raw.githubusercontent.com/zenden2k/image-uploader/master/Source/Core/3rdpart/CodePages.cpp
     *  
     * Available only in Windows.
     */
    const std::string Utf8ToAnsi(const std::string& str, int codepage);

    /**
     * The script falls asleep for the specified number of milliseconds. 
     */
    void sleep(int msec);

    /**
     * Escapes a string for use in JSON. Returns a string with quotes. 
     * @deprecated Instead, use the ToJSON function.
     */
    const std::string JsonEscapeString(const std::string& src);
    const std::string ToJSON(const Sqrat::Object&  obj);

    /**
     * Converts text containing a JSON document, into a table 
     * (associative container made in the form of key / value pairs) or an array.
    *  For example, the original JSON document:
        @code
        {
            "result": {
                "success": true,
                "links": [
                    {
                        "href": "http://example.com/thumb.jpg"
                    },
                    {
                        "href": "http://example.com/image.jpg"
                    }
                ]
            }
        } 
        @endcode
        Squirrel script:
    @code
    local t = ParseJSON(text);
    print(t.result.links[1].href); // will print "http://example.com/image.jpg"
    @endcode
    @return In case of error function retuns null.
    @since 1.3.0
    */
    Sqrat::Object ParseJSON(const std::string& json);

    /**
     * Returns a random number.
     */
    int random();
    std::string HtmlEntitiesDecode(const std::string& src);

    /**
     * @since 1.3.0
     */
    const std::string GetFileContents(const std::string& filename);

    /**
     *  URL-encodes string. This function is convenient when encoding a string to be used in a query part of a URL.
     *
     *  Use NetworkClient's nm.urlEncode() when possible instead.
     */
    const std::string url_encode(const std::string &value);

    /**
     * @brief Calculates the md5 hash of a string
     */
    const std::string md5(const std::string& data);

    // fake functions, just for docs (not implemented)
    /**
     * @brief Calculates the md5 hash of a given file
     * @since 1.2.7.4176
     */
    const std::string md5_file(const std::string& filename);

    /**
     *  Calculates the sha1-hash of a given string
     *  @since 1.2.7.4176
     */
    const std::string sha1(const std::string& data);

    /**
     *  Calculates the sha1-hash of a given file
     *  @since 1.2.7.4176
     */
    const std::string sha1_file(const std::string& filename);

    /**
     *  Calculates the sha1-hash of a given file, 
     *  prepeding file with prefix and adding postfix at the end while calculating hash
     *  @since 1.3.3
     */
    const std::string sha1_file_prefix(const std::string& filename, const std::string& prefix, const std::string& postfix);

    /**
     * Generate a keyed hash value using the HMAC method and sha1 hashing algorythm
     * 
     * @param data Message to be hashed.
     * @param key Shared secret key used for generating the HMAC variant of the message digest.
     * @param base64 Generate in base64 format
     */
    const std::string hmac_sha1(const std::string& key, const std::string& data, bool base64);

    /**
     * Extract the file name without extension from the path.
     * @since 1.3.0
     */
    const std::string ExtractFileNameNoExt(const std::string& fileName);

    /**
     * Extract directory name from path.
     * @since 1.3.0
     */
    const std::string ExtractFilePath(const std::string& fileName);

    
    bool CopyFile(const std::string& source, const std::string & destination, bool overwrite);
    /**
     * Creates a directory (recursively). The <b>mode</b> parameter is ignored in Windows.
     */
    bool CreateDirectory(const std::string& path_, unsigned int mode);
    /**
     * Checks if a file exists.
     * @since 1.3.0
     */
    bool FileExists(const std::string& fileName);

    /**
     * Renames/moves a file or folder. 
     * @since 1.3.0
     */
    bool MoveFileOrFolder(const std::string& from, const std::string& to);

    /**
     * Write data to a file
     * @since 1.3.0
     */
    bool PutFileContents(const std::string& utf8Filename, const std::string& content);
    bool DeleteFile(const std::string& utf8Filename);

    /**
     * Includes and runs the script from file. 
     * <b>filename</b> should be relative to the scripts root directory.
     */
    Sqrat::Object include(const std::string& filename);

    /**
     * Standard squirrel function for writing text to standard output. It's results can be seen in the log window.
     */
    void print(std::string arg);
    std::string Base64Encode(const std::string& data);
    std::string Base64Decode(const std::string& data);

    /** 
    Execute shell command. Parameter cmd is using default operating system locale (so it won't work with utf-8 strings on windows).
    Use Process class instead
    */
    int system(std::string cmd);

    /**
     * Returns the size of the file in bytes.
     * @since 1.3.0
     */
    int GetFileSize(const std::string& filename);

    /**
    * Returns the size of the file in bytes (for files greater than 2 GB).
    * @since 1.3.0
    * @deprecated
    */
    double GetFileSizeDouble(const std::string& filename);
    /**
     * Returns a table containing information about image. For example:
    @code
    {
        Width = 640,
        Height = 480
    }
    @endcode
    @since 1.3.2.4616
     */
    Sqrat::Table GetImageInfo(const std::string& fileName);

    std::string GetDeviceId();
    std::string GetDeviceName();
}

#endif