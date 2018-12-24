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
#ifndef IU_CORE_UPLOADENGINE_H
#define IU_CORE_UPLOADENGINE_H

#pragma once

#include <vector>
#include <string>
#include <map>

#include "Core/3rdpart/FastDelegate.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Network/NetworkClient.h"
#include "CommonTypes.h"
#include "Core/Scripting/API/UploadTaskWrappers.h"

class ServerSync;

struct LoginInfo
{
    std::string Login, Password, Cookies;
    bool DoAuth;
    LoginInfo() {
        DoAuth = false;
    }
};

struct ActionVariable
{
    std::string Name;
    int nIndex;
    ActionVariable(const std::string& name, int index) :Name(name), nIndex(index) {

    }
    ActionVariable() : nIndex(0) {
        
    }
};

struct ActionRegExp {
    std::string Pattern;
    std::string Data;
    std::string AssignVars;
    std::vector<ActionVariable> Variables;
    bool Required;

    ActionRegExp() : Required(true) {
        
    }
};
struct UploadAction
{
    int Index;
    bool IgnoreErrors;
    bool OnlyOnce;
    std::string Url;
    std::string Description;
    std::string Referer;
    std::string PostParams;
    std::string CustomHeaders;
    std::string Type;
   // std::string RegExp;

    std::vector<ActionRegExp> Regexes;
    int RetryLimit;
    int NumOfTries;

    UploadAction() {
        Index = 0; 
        IgnoreErrors = false;
        OnlyOnce = false;
        RetryLimit = 1;
        NumOfTries = 0;
    }
};

/**
CFolderItem class
*/
class CFolderItem
{
public:
    CFolderItem()
    {
        accessType = 0;
        itemCount = -1;
    }

    static const std::string NewFolderMark;

    /*! @cond PRIVATE */
    std::string title;
    std::string summary;
    std::string id;
    std::string parentid;
    std::string viewUrl;
    int accessType;
    int itemCount;
    /*! @endcond */
    std::string getTitle() const { return title;}
    std::string getSummary() const { return summary;}
    std::string getId() const { return (id);}
    std::string getParentId() const { return (parentid);}
    int getItemCount() const { return itemCount; }
    int getAccessType() const { return accessType; }
    std::string getViewUrl() const { return (viewUrl); }

    void setTitle(const std::string& str) { title = (str); }
    void setViewUrl(const std::string& str) { viewUrl = (str); }

    void setSummary(const std::string& str) { summary = (str); }
    void setId(const std::string& str) { id = (str); }
    void setParentId(const std::string& str) { parentid = (str); }
    void setAccessType(const int type) {  accessType=type; }
    void setItemCount(const int count) {  itemCount=count; }    
};

/**
ServerSettingsStruct
*/
class ServerSettingsStruct
{
public:
    /*! @cond PRIVATE */
    ServerSettingsStruct() : paramsMutex_(new std::mutex()){ authData.DoAuth = 0; }
    std::map<std::string, std::string> params;
    std::shared_ptr<std::mutex> paramsMutex_;
    LoginInfo authData;
    CFolderItem newFolder;
    CFolderItem defaultFolder;

    bool isEmpty() {
        return authData.Login.empty() && authData.Password.empty() && !authData.DoAuth && !params.size() && defaultFolder.getId().empty();
    }
    /*! @endcond */

    std::string getParam(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(*paramsMutex_);
        std::string result;
        std::string pname = name;
        if(pname == "Password" && authData.DoAuth)
            result = authData.Password;
        else if(pname == "Login" && authData.DoAuth)
            result = authData.Login;
        else {
            auto it = params.find(name);
            if (it != params.end()) {
                result = it->second;
            }        
        }
        return result;
    }

    void setParam(const std::string& name, const std::string& value)
    {
        std::lock_guard<std::mutex> lock(*paramsMutex_);
        params[name] = value;
    }
};

typedef std::map <std::string, std::map <std::string, ServerSettingsStruct>> ServerSettingsMap;

class CUploadEngineData
{
    public:
        enum ServerType { TypeImageServer = 1, TypeFileServer = 2 , TypeUrlShorteningServer = 4, TypeTextServer = 8};
        enum NeedAuthorizationEnum { naNotAvailable = 0, naAvailable, naObligatory };

        std::string Name;
        std::string PluginName;
        bool SupportsFolders;
        bool UsingPlugin;
        bool Debug;
        //bool ImageHost;
        bool SupportThumbnails;
        bool BeforehandAuthorization;
        int NeedAuthorization;
        bool NeedPassword;
        int64_t MaxFileSize;
        std::string RegistrationUrl;
        std::string CodedLogin;
        std::string CodedPassword;
        std::string ThumbUrlTemplate, ImageUrlTemplate, DownloadUrlTemplate, DeleteUrlTemplate, EditUrlTemplate;
        std::vector<UploadAction> Actions;
        std::string LoginLabel, PasswordLabel;
        std::string UserAgent;
        std::string Engine;
        int RetryLimit;
        int NumOfTries;
        int MaxThreads;
        int TypeMask;
        bool hasType(ServerType type) const;
        CUploadEngineData();
};
/** 
CIUUploadParams class
*/
class UploadParams
{
public:
    /*! @cond PRIVATE */
    int apiVersion;
    int thumbWidth;
    int thumbHeight;
    std::string serverName;
    std::string data;
    std::string folderId;
    std::string DirectUrl;

    std::string ThumbUrl;
    std::string ViewUrl; 
    std::string EditUrl;
    std::string DeleteUrl;

    std::string ServerFileName;
    std::string temp_;
    ScriptAPI::UploadTaskWrapper task_;

    UploadParams() {
        apiVersion = 0;
        thumbWidth = 0;
        thumbHeight = 0;
    }
    /*! @endcond */

    const std::string getParam(const std::string& name)
    {
        temp_.clear();
        if(name == "THUMBWIDTH")
            temp_= IuCoreUtils::toString(thumbWidth);
        else if (name == "THUMBHEIGHT")
            temp_ =  IuCoreUtils::toString(thumbHeight);
        return temp_;
    }

    const std::string getFolderID() { return folderId; }
    void setDirectUrl(const std::string& url) { DirectUrl = url;}
    void setThumbUrl(const std::string& url) { ThumbUrl = url;}
    void setViewUrl(const std::string& url) { ViewUrl = url;}
    /**
    @since version 1.3.2
    */
    void setEditUrl(const std::string& url) { EditUrl = url;}
    /**
    @since version 1.3.2
    */
    void setDeleteUrl(const std::string& url) { DeleteUrl = url;}

    /**
    @since version 1.3.2
    */
    std::string getDirectUrl() const
    {
        return DirectUrl;
    }
    /**
    @since version 1.3.2
    */
    std::string getThumbUrl() const
    {
        return ThumbUrl;
    }
    /**
    @since version 1.3.2
    */
    std::string getViewUrl() const
    {
        return ViewUrl;
    }
    /**
    @since version 1.3.2
    */
    std::string getEditUrl() const
    {
        return EditUrl;
    }
    /**
    @since version 1.3.2
    */
    std::string getDeleteUrl() const
    {
        return DeleteUrl;
    }

    const std::string getServerFileName() const { return ServerFileName; }
    ScriptAPI::UploadTaskWrapper getTask() { return task_; }
};

class CUploadEngineListBase
{
public:
    CUploadEngineListBase();
    CUploadEngineData* byIndex(size_t index);
    CUploadEngineData* byName(const std::string& name);
    CUploadEngineData* firstEngineOfType(CUploadEngineData::ServerType type);
    int count() const;
    int getRandomImageServer() const;
    int getRandomFileServer() const;
    int getUploadEngineIndex(const std::string& Name) const;
    std::vector<CUploadEngineData>::const_iterator begin() const;
    std::vector<CUploadEngineData>::const_iterator end() const;
protected:
    std::vector<CUploadEngineData> m_list;
private:
    DISALLOW_COPY_AND_ASSIGN(CUploadEngineListBase);
};

class UploadTask;
class CUploader;

class CAbstractUploadEngine
{
    public:
        CAbstractUploadEngine(ServerSync* serverSync);
        virtual ~CAbstractUploadEngine();
        virtual int doUpload(std::shared_ptr<UploadTask> task, UploadParams& params) = 0;
        void setServerSettings(ServerSettingsStruct* settings);
        ServerSettingsStruct * serverSettings() const;
        virtual int RetryLimit()=0;
        virtual void setNetworkClient(INetworkClient* nm);
        void setUploadData(CUploadEngineData* data);
        void setServerSync(ServerSync* sync);
        void setCurrentUploader(CUploader *uploader);
        CUploader * currentUploader() const;
        virtual void stop();
        ServerSync* serverSync() const;
        CUploadEngineData* getUploadData() const;
        // Events
        fastdelegate::FastDelegate0<bool> onNeedStop;
        fastdelegate::FastDelegate1<InfoProgress> onProgress;
        fastdelegate::FastDelegate3<StatusType, int, std::string> onStatusChanged;
        fastdelegate::FastDelegate2< const std::string&, bool> onDebugMessage;
        typedef fastdelegate::FastDelegate1<const ErrorInfo&> ErrorMessageCallback;
        ErrorMessageCallback onErrorMessage;
        DISALLOW_COPY_AND_ASSIGN(CAbstractUploadEngine);
    protected:
        bool m_bShouldStop;
        INetworkClient * m_NetworkClient;
        CUploadEngineData * m_UploadData;
        CUploader * currUploader_;
        ServerSettingsStruct* m_ServersSettings;
        std::shared_ptr<UploadTask> currentTask_;
        ServerSync* serverSync_;
        bool DebugMessage(const std::string& message, bool isServerResponseBody = false);
        bool ErrorMessage(ErrorInfo);
        virtual bool needStop();
        virtual void SetStatus(StatusType status, const std::string& param = "");        
};

#endif // IU_CORE_UPLOADENGINE_H