#ifndef IU_CORE_SETTINGS_BASICSETTINGS_H
#define IU_CORE_SETTINGS_BASICSETTINGS_H

#pragma once
#include "Core/3rdpart/FastDelegate.h"
#include "Core/SettingsManager.h"
#include "Core/Upload/UploadEngine.h"
#include "StringConvert.h"

typedef std::map <std::string, std::map <std::string, ServerSettingsStruct>> ServerSettingsMap;

class BasicSettings {
public:
    BasicSettings();
    virtual ~BasicSettings();
    void setEngineList(CUploadEngineList_Base* engineList);
    bool LoadSettings(std::string szDir = "", std::string fileName = "", bool LoadFromRegistry = true);
    bool SaveSettings();
    void notifyChange();
    typedef fastdelegate::FastDelegate1<BasicSettings*> ChangeCallback;
    std::vector<ChangeCallback> changeCallbacks_;
    unsigned int LastUpdateTime;

    void addChangeCallback(const ChangeCallback& callback);

    // Fields
    int FileRetryLimit;
    int ActionRetryLimit;
    std::mutex serverSettingsMutex_;
    ServerSettingsMap ServersSettings;
    std::string ScriptFileName;
    bool ExecuteScript;

    int UploadBufferSize;
    std::string SettingsFolder;
protected:
    SettingsManager mgr_;
    std::string fileName_;
  
    CUploadEngineList_Base* engineList_;
    bool loadFromRegistry_;
    bool LoadAccounts(SimpleXmlNode root);
    bool SaveAccounts(SimpleXmlNode root);
    void BindToManager();
    virtual bool PostLoadSettings(SimpleXml &xml);
    virtual bool PostSaveSettings(SimpleXml &xml);
};
#endif