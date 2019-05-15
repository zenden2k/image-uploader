/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "Functions.h"

#include "Core/AppParams.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Scripting/Squirrelnc.h"
#include "Core/Logging.h"
#include <json/json.h>
#include "Core/Network/NetworkClient.h"
#ifdef _WIN32
    #include <windows.h>
    #ifndef IU_CLI
        #include "Func/IuCommonFunctions.h"
        #include "Core/i18n/Translator.h"
        #include "Func/WinUtils.h"
    #endif
#include "Core/Images/Utils.h"
#endif

#include "Core/Scripting/DialogProvider.h"

#include "Core/Utils/StringUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include <json/json.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "Core/3rdpart/base64.h"
#include "Core/3rdpart/codepages.h"
#include "Core/Utils/TextUtils.h"
#include "ScriptAPI.h"
#include "Core/ServiceLocator.h"
#include "Core/Upload/UploadErrorHandler.h"
#include "Core/Utils/DesktopUtils.h"
#include "Core/i18n/Translator.h"

using namespace Sqrat;

namespace ScriptAPI {
/*
SquirrelObject* RootTable = 0;*/

const std::string GetScriptsDirectory()
{
    return AppParams::instance()->settingsDirectory() + "Scripts/";
}

const std::string GetAppLanguageFile()
{
    std::string languageFile = AppParams::instance()->languageFile();
    if ( languageFile.empty() ) {
        return "English";
    }
    return IuCoreUtils::ExtractFileNameNoExt(languageFile);
}

Sqrat::Table GetAppVersion() {
    Sqrat::Table res(GetCurrentThreadVM());
    auto version = AppParams::instance()->GetAppVersion();
    if (version) {
        res.SetValue("Major", static_cast<SQInteger>(version->Major));
        res.SetValue("Minor", static_cast<SQInteger>(version->Minor));
        res.SetValue("Release", static_cast<SQInteger>(version->Release));
        res.SetValue("Build", static_cast<SQInteger>(version->Build));
    }
    res.SetValue("Gui", AppParams::instance()->isGui());
    return res;
}

const std::string GetCurrentScriptFileName()
{
    return GetScriptName(GetCurrentThreadVM());
}

Sqrat::Object IncludeScript(const std::string& filename)
{    
    if ( filename.empty() ) {
        LOG(ERROR) << "include() failed: empty file name";
        return Sqrat::Object();
    }
    std::string absolutePath;
    if ( filename[0] == '/' || (filename.length()>1 && filename[1]==':' ) ) {
        absolutePath = filename;
    } else {
        std::string dir = IuCoreUtils::ExtractFilePath(GetCurrentScriptFileName());
        if (dir.empty())
        {
            dir = GetScriptsDirectory();
        }
        absolutePath = dir  + filename;
    }

    if ( !IuCoreUtils::FileExists(absolutePath) ) {
        LOG(ERROR) << "include() failed: file \"" + absolutePath + "\" not found.";
        return Sqrat::Object();
    }
    std::string scriptText;
    if ( !IuCoreUtils::ReadUtf8TextFile(absolutePath, scriptText) ) {
        LOG(ERROR) << "include() failed: could not read file \"" + absolutePath + "\".";
        return Sqrat::Object();
    }
    Sqrat::Script squirrelScript(GetCurrentThreadVM());
    squirrelScript.CompileString(scriptText.c_str(),IuCoreUtils::ExtractFileName(absolutePath).c_str());
    squirrelScript.Run();
    return squirrelScript;
}

Json::Value* translationRoot = 0;

void CleanUpFunctions() {
    delete translationRoot;
    translationRoot = 0;
}

bool LoadScriptTranslation() {
    if ( !translationRoot ) {
        translationRoot = new Json::Value();
        std::string absolutePath = GetScriptsDirectory() + "Lang/" + GetAppLanguageFile() + ".json";
        std::string jsonText;
        if ( !IuCoreUtils::ReadUtf8TextFile(absolutePath, jsonText) ) {
            return false;
        }
        
        Json::Reader reader;
        if ( reader.parse(jsonText, *translationRoot, false) ) {
            return true;
        }
        return false;
    } else {
        return true;
    }
}

const std::string Translate(const std::string& key, const std::string& originalText) {
    if ( LoadScriptTranslation() ) {
        std::vector<std::string> tokens;
        IuStringUtils::Split(key, ".", tokens, -1);
        Json::Value root = *translationRoot;
        int count = tokens.size();
        for ( int i = 0; i < count; i++ ) {
            std::string token = tokens[i];
            if ( !root.isMember(token) ) {
                break;
            }
            root = root[token];
            if ( root.type() != Json::objectValue && i+1 != count ) {
                break;
            }
            if ( i+1 == count && root.type() == Json::stringValue  ) {
                return root.asString();
            }
        }
    }

    auto translator = ServiceLocator::instance()->translator();
    if (translator) {
        return translator->translate(originalText.c_str());
    } 
    return originalText;
}

const std::string AskUserCaptcha(NetworkClient* nm, const std::string& url)
{
    return ServiceLocator::instance()->dialogProvider()->askUserCaptcha(nm, url);
}

const std::string InputDialog(const std::string& text, const std::string& defaultValue) {
    return ServiceLocator::instance()->dialogProvider()->inputDialog(text, defaultValue);
}


const std::string GetFileMimeType(const std::string& filename)
{
    return IuCoreUtils::GetFileMimeType(filename);
}

const std::string AnsiToUtf8(const std::string& str, int codepage)
{
#ifdef _WIN32
    return IuCoreUtils::ConvertToUtf8(str, NameByCodepage(codepage));
#else
    LOG(WARNING) << "AnsiToUtf8 not implemented";
    return str; // FIXME
#endif
}

const std::string scriptUtf8ToAnsi(const std::string& str, int codepage )
{
#ifdef _WIN32
    return IuCoreUtils::Utf8ToAnsi(str, codepage);
#else
    LOG(WARNING) << "Utf8ToAnsi not implemented";
    return str; // FIXME
#endif
}

void WriteLog(const std::string& type, const std::string& message) {
    ILogger::LogMsgType msgType = ILogger::logWarning;
    if ( type == "error" ) {
        msgType = ILogger::logError;
    } else if (type == "info") {
        msgType = ILogger::logInformation;
    }
    ServiceLocator::instance()->logger()->write(msgType, "Script Engine", message, "Script: " + IuCoreUtils::ExtractFileName(GetCurrentScriptFileName()), GetCurrentTopLevelFileName());
}

const std::string md5(const std::string& data)
{
    return IuCoreUtils::CryptoUtils::CalcMD5HashFromString(data);
}

void sleep(int msec) {
#ifdef _WIN32
    ::Sleep(msec);
#else
    ::sleep(ceil(msec/1000.0));
#endif
}

const std::string JsonEscapeString( const std::string& src) {
    return Json::valueToQuotedString(src.data());
}

const std::string GetTempDirectory() {
    return AppParams::instance()->tempDirectory();
}

const std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex << std::uppercase;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << '%' << std::setw(2) << int((unsigned char) c);
    }

    return escaped.str();
}

void DebugMessage(const std::string& msg, bool isResponseBody) {
    ServiceLocator::instance()->uploadErrorHandler()->DebugMessage(msg,isResponseBody);
}

const std::string MessageBox( const std::string& message, const std::string &title,const std::string& buttons , const std::string& type) {
#if defined(_WIN32) && !defined(IU_CLI)
    UINT uButtons = MB_OK;
    if ( buttons == "ABORT_RETRY_IGNORE") {
        uButtons = MB_ABORTRETRYIGNORE;
    } else if ( buttons == "CANCEL_TRY_CONTINUE") {
        uButtons = MB_CANCELTRYCONTINUE;
    } else if ( buttons == "OK_CANCEL") {
        uButtons = MB_OKCANCEL;
    } else if ( buttons == "RETRY_CANCEL") {
        uButtons = MB_RETRYCANCEL;
    } else if ( buttons == "YES_NO") {
        uButtons = MB_YESNO;
    }else if ( buttons == "YES_NO_CANCEL") {
        uButtons = MB_YESNOCANCEL;
    }
    UINT icon = 0;
    if ( type == "EXCLAMATION") {
        icon = MB_ICONEXCLAMATION;
    } else if ( type == "WARNING") {
        icon = MB_ICONWARNING;
    } else if ( type == "INFORMATION") {
        icon = MB_ICONINFORMATION;
    } else if ( type == "QUESTION") {
        icon = MB_ICONQUESTION;
    } else if ( type == "ERROR") {
        icon = MB_ICONERROR;
    } 
    int res = ::MessageBox(GetActiveWindow(), IuCoreUtils::Utf8ToWstring(message).c_str(),  IuCoreUtils::Utf8ToWstring(title).c_str(), uButtons |icon );
    if ( res == IDABORT ) {
        return "ABORT";
    } else if ( res == IDCANCEL ) {
        return "CANCEL";
    } else if ( res == IDCONTINUE ) {
        return "CONTINUE";
    } else if ( res == IDIGNORE ) {
        return "IGNORE";
    } else if ( res == IDNO ) {
        return "NO";
    } else if ( res == IDOK ) {
        return "OK";
    } else if ( res == IDYES ) {
        return "YES";
    }
    else if ( res == IDRETRY ) {
        return "TRY";
    } else if ( res == IDTRYAGAIN ) {
        return "TRY";
    } 
    return "";
    
#else
    std::cerr<<"----------";
#ifdef _WIN32
    std::wcerr<<IuCoreUtils::Utf8ToWstring(title);
#else
    std::cerr<<IuCoreUtils::Utf8ToSystemLocale(title);
#endif
    std::cerr<<"----------"<<std::endl;
#ifdef _WIN32
    std::wcerr<<IuCoreUtils::Utf8ToWstring(message)<<std::endl;;
#else
    std::cerr<<IuCoreUtils::Utf8ToSystemLocale(message)<<std::endl;;
#endif
    if ( buttons.empty() || buttons == "OK") {
            getc(stdin);
            return "OK";
    } else {
        

        std::vector<std::string> tokens;
        std::map<char,std::string> buttonsMap;
        IuStringUtils::Split(buttons,"_", tokens,10);
        for(int i = 0; i< tokens.size(); i++ ) {
            if ( i !=0 ) {
                std::cerr<< "/";
            }
            buttonsMap[tokens[i][0]] = tokens[i];
            std::cerr<< "("<<tokens[i][0]<<")"<<IuStringUtils::toLower(tokens[i]).c_str()+1;
        }
        std::cerr<<": ";
        char res;
        std::cin >> res;
        res = toupper(res);
        return buttonsMap[res];
    }

#endif
}

void parseJSONObj(const Json::Value& root, Sqrat::Array& obj);
void parseJSONObj(const Json::Value& root, Sqrat::Table& obj);

template<class T,class V> void setObjValues(T key, Json::ValueIterator it, V &obj) {
    using namespace Json;
    Sqrat::Array newArr;
    Sqrat::Table newObj;

    try {
        switch (it->type()) {
        case nullValue:
            obj.SetValue(key, Sqrat::Object());
            break;
        case intValue:      ///< signed integer value
            obj.SetValue(key, static_cast<SQInteger>(it->asInt64()));
            break;
        case uintValue:     ///< unsigned integer value
            obj.SetValue(key, static_cast<SQInteger>(it->asInt64()));
            break;
        case realValue:
            obj.SetValue(key, it->asFloat());
            break;    ///< double value
        case stringValue:   ///< UTF-8 string value
            obj.SetValue(key, it->asString().data());
            break;
        case booleanValue:  ///< bool value
            obj.SetValue(key, it->asBool());
            break;
        case arrayValue:    ///< array value (ordered list)
            parseJSONObj(*it, newArr);
            obj.SetValue(key, newArr);
            break;
        case objectValue:
            parseJSONObj(*it, newObj);
            obj.SetValue(key, newObj);
            break;
        }
    } catch (std::logic_error & ex) {
        LOG(WARNING) << "setObjValue()" << std::endl << ex.what();
    }
}

void parseJSONObj(const Json::Value& root, Sqrat::Array& obj) {
    Json::ValueIterator it;
    obj = Sqrat::Array (GetCurrentThreadVM(), root.size());
    for(it = root.begin(); it != root.end(); ++it) {
        int key = it.key().asInt();
        setObjValues(key, it, obj);
    }
}

void parseJSONObj(const Json::Value& root, Sqrat::Table& obj) {
    Json::ValueIterator it;
    obj = Sqrat::Table (GetCurrentThreadVM());
    for(it = root.begin(); it != root.end(); ++it) {
        std::string key = it.key().asString();
        setObjValues(key.data(), it, obj);
    }
}

Sqrat::Object parseJSONObj(const Json::Value& root) {
    Json::ValueIterator it;
    //SquirrelObject obj;
    bool isArray = root.isArray();
    
    if ( isArray ) {
        Sqrat::Array obj(GetCurrentThreadVM(), root.size());
        for(it = root.begin(); it != root.end(); ++it) {
            int key = it.key().asInt();
            setObjValues(key, it, obj);
        }
        return obj;
    } else {
        Sqrat::Table obj(GetCurrentThreadVM());
        for(it = root.begin(); it != root.end(); ++it) {
            std::string key = it.key().asString();
            setObjValues(key.data(), it, obj);
        }
        return obj;
    }
}

Sqrat::Object ParseJSON(const std::string& json) {
    Json::Value root;
    Json::Reader reader;
    Sqrat::Object sq;
    if ( reader.parse(json, root, false) ) {
        return parseJSONObj(root);
    }
    return sq;
}

Json::Value sqValueToJson(const Sqrat::Object& obj ) {
    switch ( obj.GetType() ) {
        case OT_NULL:
            return Json::Value(Json::nullValue);
        case OT_INTEGER:
            return SQINT_TO_JSON_VALUE(obj.Cast<int>());
            break;
        case OT_FLOAT:
            return obj.Cast<float>();
            break;

        case OT_BOOL:
            return obj.Cast<bool>();
            break;

        case OT_STRING:
            return obj.Cast<std::string>();
            break;
    }
    return Json::Value(Json::nullValue);
}
Json::Value sqObjToJson(const Sqrat::Object& obj ) {
    HSQUIRRELVM vm = GetCurrentThreadVM();
    Json::Value res;
    Sqrat::Object::iterator it;
    switch ( obj.GetType() ) {
            case OT_NULL:
            case OT_INTEGER:
            case OT_FLOAT:
            case OT_BOOL:
            case OT_STRING:
                return sqValueToJson(obj);
                break;
            case OT_TABLE:
                while(obj.Next(it) ) {
                    res[it.getName()] = sqObjToJson(Sqrat::Object(it.getValue(), vm));
                }
                return res;
                break;
            case OT_ARRAY: 
                while(obj.Next(it) ) {
                    res[Sqrat::Object(it.getKey(), vm).Cast<int>()] = sqObjToJson(Sqrat::Object(it.getValue(), vm));
                }
                return res;
                break;                
    }
    return Json::Value(Json::nullValue);
}

const std::string ToJSON(const Sqrat::Object&  obj) {
    Json::Value root = sqObjToJson(obj);
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";  // or whatever you like

    return Json::writeString(builder, root);
}




int64_t ScriptGetFileSize(const std::string& filename) {
    return IuCoreUtils::getFileSize(filename);
}

const std::string GetAppLanguage() {
#ifndef IU_CLI
    ITranslator* translator = ServiceLocator::instance()->translator();
    if ( !translator ) {
        LOG(ERROR) << "No translator set";
    } else {
        return translator->getCurrentLanguage();
    }
#endif 
    return "en";
}

const std::string GetAppLocale() {
#ifndef IU_CLI
    ITranslator* translator = ServiceLocator::instance()->translator();
    if (!translator) {
        LOG(ERROR) << "No translator set";
    } else {
        return ServiceLocator::instance()->translator()->getCurrentLocale();
    }
#endif 
    return "en_US";
}

std::string GetCurrentThreadId()
{
    std::thread::id treadId = std::this_thread::get_id();
    return IuCoreUtils::ThreadIdToString(treadId);
}
#undef random
// older versions of Squirrel Standart Library have broken srand() function
int random()
{
    return rand();
}

const std::string ExtractFileName(const std::string& path)
{
    std::string res = IuCoreUtils::ExtractFileName(path);
    return res;
}

const std::string GetFileExtension(const std::string& path)
{
    std::string res = IuCoreUtils::ExtractFileExt(path);
    return res;
}

Sqrat::Table GetImageInfo(const std::string& fileName) {
    Sqrat::Table obj(GetCurrentThreadVM());
    int width = 0, height = 0;
#ifdef _WIN32    
    ImageUtils::ImageInfo ii = ImageUtils::GetImageInfo(U2W(fileName));
    width = ii.width;
    height = ii.height;
#endif
    obj.SetValue("Width", width);
    obj.SetValue("Height", height);
    return obj;
}

/*
void DebugMessage(const std::string& message, bool isServerResponseBody)
{
#ifdef IU_CLI
    #ifdef _WIN32
        std::wcerr << IuCoreUtils::Utf8ToWstring(message);
    #else
        std::cerr << IuCoreUtils::Utf8ToSystemLocale(message);
    #endif
#else
    DefaultErrorHandling::DebugMessage(message, isServerResponseBody);
#endif
}
*/
void RegisterFunctions(Sqrat::SqratVM& vm)
{
    Sqrat::RootTable& root = vm.GetRootTable();
    /*RootTable = rootTable;*/
    root
        .Func("GetScriptsDirectory", GetScriptsDirectory)
        .Func("GetAppLanguageFile", GetAppLanguageFile)
        .Func("include", IncludeScript)
        .Func("Translate", Translate)
        .Func("GetAppVersion", GetAppVersion)

        .Func("random", random)
        .Func("sleep", sleep)
        .Func("md5", md5)
        .Func("AnsiToUtf8", AnsiToUtf8)
        .Func("Utf8ToAnsi", scriptUtf8ToAnsi)
        .Func("ExtractFileName", ExtractFileName)
        .Func("GetFileExtension", GetFileExtension)
        .Func("AskUserCaptcha", AskUserCaptcha)
        .Func("InputDialog", InputDialog)
        .Func("GetFileMimeType", GetFileMimeType)
        .Func("JsonEscapeString", JsonEscapeString)
        .Func("ShellOpenUrl", DesktopUtils::ShellOpenUrl)
        .Func("ParseJSON", ParseJSON)
        .Func("ToJSON", ToJSON)
        .Func("GetFileContents", IuCoreUtils::GetFileContents)
        .Func("GetTempDirectory", GetTempDirectory)
        .Func("ExtractFileNameNoExt", IuCoreUtils::ExtractFileNameNoExt)
        .Func("ExtractFilePath", IuCoreUtils::ExtractFilePath)
        .Func("CopyFile", IuCoreUtils::copyFile)
        .Func("CreateDirectory", IuCoreUtils::createDirectory)
        .Func("FileExists", IuCoreUtils::FileExists)
        .Func("MoveFileOrFolder", IuCoreUtils::MoveFileOrFolder)
        .Func("PutFileContents", IuCoreUtils::PutFileContents)
        .Func("DeleteFile", IuCoreUtils::RemoveFile)
        .Func("GetAppLanguage", GetAppLanguage)
        .Func("GetAppLocale", GetAppLocale)
        .Func("HtmlEntitiesDecode", IuTextUtils::DecodeHtmlEntities)

        .Func("GetFileSize", ScriptGetFileSize)    
        .Func("GetFileSizeDouble", ScriptGetFileSize)
        .Func("GetImageInfo", GetImageInfo)
        .Func("WriteLog", WriteLog)    
        .Func("GetCurrentScriptFileName", GetCurrentScriptFileName)
        .Func("GetCurrentThreadId", GetCurrentThreadId)
        .Func("MessageBox", MessageBox);    

    using namespace IuCoreUtils;
    root
        .Func("md5_file", &CryptoUtils::CalcMD5HashFromFile)
        .Func("sha1", &CryptoUtils::CalcSHA1HashFromString)
        .Func("sha1_file", &CryptoUtils::CalcSHA1HashFromFile)
        .Func("hmac_sha1", &CryptoUtils::CalcHMACSHA1HashFromString)
        .Func("Base64Decode", &CryptoUtils::Base64Decode)
        .Func("Base64Encode", &CryptoUtils::Base64Encode)
        .Func("url_encode", url_encode)
        .Func("DebugMessage", DebugMessage);
    srand(static_cast<unsigned int>(time(0)));

    
}
//    atexit(&CleanUpFunctions);


void RegisterShortTranslateFunctions(Sqrat::SqratVM& vm) {
    Sqrat::RootTable& root = vm.GetRootTable();

    Function func(root, "tr");
    if ( func.IsNull() ) {
        root.Func("tr", Translate);
    }
}

}
