#ifndef IU_CORE_SETTINGS_BASICSETTINGS_H
#define IU_CORE_SETTINGS_BASICSETTINGS_H

#pragma once
#include "Core/3rdpart/FastDelegate.h"
#include "Core/SettingsManager.h"
#include "Core/Upload/UploadEngine.h"
#include "StringConvert.h"
#include "EncodedPassword.h"

typedef std::map <std::string, std::map <std::string, ServerSettingsStruct>> ServerSettingsMap;

struct ConnectionSettingsStruct {
    enum UseProxyEnum { kNoProxy = 0, kUserProxy, kSystemProxy };
    int UseProxy;
    std::string ServerAddress;
    int ProxyPort;
    bool NeedsAuth;
    std::string ProxyUser;
    CEncodedPassword ProxyPassword;
    int ProxyType;
};

class BasicSettings {
public:
    BasicSettings();
    virtual ~BasicSettings();
    void setEngineList(CUploadEngineListBase* engineList);
    bool LoadSettings(const std::string& szDir = "", const std::string& fileName = "", bool LoadFromRegistry = true);
    bool SaveSettings();
    void notifyChange();
    typedef fastdelegate::FastDelegate1<BasicSettings*> ChangeCallback;
    std::vector<ChangeCallback> changeCallbacks_;
    unsigned int LastUpdateTime;

    void addChangeCallback(const ChangeCallback& callback);
    void removeChangeCallback(const ChangeCallback& callback);

    // Fields
    int FileRetryLimit;
    int ActionRetryLimit;
    std::mutex serverSettingsMutex_;
    ServerSettingsMap ServersSettings;
    std::string ScriptFileName;
    bool ExecuteScript;
    bool DeveloperMode;

    int MaxThreads;
    bool AutoShowLog;

    int UploadBufferSize;
    std::string SettingsFolder;

    ConnectionSettingsStruct ConnectionSettings;

    ServerSettingsStruct* getServerSettings(const ServerProfile& profile, bool create = false);
protected:
    SettingsManager mgr_;
    std::string fileName_;
  
    CUploadEngineListBase* engineList_;
    bool loadFromRegistry_;
    bool LoadAccounts(SimpleXmlNode root);
    bool SaveAccounts(SimpleXmlNode root);
    void BindToManager();
    virtual bool PostLoadSettings(SimpleXml &xml);
    virtual bool PostSaveSettings(SimpleXml &xml);
};
#endif