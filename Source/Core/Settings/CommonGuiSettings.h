#ifndef IU_CORE_SETTINGS_COMMONGUISETTINGS_H
#define IU_CORE_SETTINGS_COMMONGUISETTINGS_H

#pragma once


#include <map>
#include "Core/SettingsManager.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/Upload/ServerProfile.h"
#include "BasicSettings.h"
#include "Core/Upload/ServerProfileGroup.h"

#ifdef IU_QT
    #include <QString>
    typedef QString SettingsString;
    #define Utf8ToSettingsString(s) QString::fromStdString(s)
    #define SettingsStringToUtf8(s) s.toStdString()

#else
    #include "atlheaders.h"
    typedef CString SettingsString;
    #define Utf8ToSettingsString(s) Utf8ToWCstring(s)
    #define SettingsStringToUtf8(s) WCstringToUtf8(s)
#endif

#ifndef _WIN32
    #define TCHAR char
    #define _T(a) a
#endif

#include "EncodedPassword.h"

#include "Core/Images/ImageConverter.h"

typedef std::map<SettingsString, ServerProfile> ServerProfilesMap;


struct VideoSettingsStruct {
    int Columns;
    int TileWidth;
    int GapWidth;
    int GapHeight;
    int NumOfFrames;
    int JPEGQuality;
    std::string Engine;
#ifndef IU_QT
    bool ShowMediaInfo;
    CString Font;
    COLORREF TextColor;
    CString SnapshotsFolder;
    CString SnapshotFileTemplate;
#endif
};

#ifndef IU_QT
struct MediaInfoSettingsStruct {
    int InfoType; // 0 - short summary, 1 - full info
    bool EnableLocalization;
};

struct HistorySettingsStruct {
    bool EnableDownloading;
    bool HistoryConverted;
};
#endif

class CommonGuiSettings : public BasicSettings {
    public:   
        CommonGuiSettings();
        ~CommonGuiSettings();

        bool UseDirectLinks;

        ServerProfile urlShorteningServer, temporaryServer;
        ServerProfileGroup imageServer, fileServer, quickScreenshotServer, contextMenuServer;
        ServerProfilesMap ServerProfiles;
        VideoSettingsStruct VideoSettings;
#ifndef IU_QT
        CString Language;
        CString DataFolder;
        CString m_SettingsDir;
        bool ShowTrayIcon = false;
        bool ShowTrayIcon_changed = false;
        bool AutoStartup;
        bool AutoStartup_changed;
        int ThumbsPerLine;
        TCHAR m_szLang[64];

        MediaInfoSettingsStruct MediaInfoSettings;

        HistorySettingsStruct HistorySettings;

        int CodeLang;
        int CodeType;

        bool UseProxyServer;
        bool IsPortable;

        CString VideoFolder, ImagesFolder;

        std::map<CString, ImageConvertingParams> ConvertProfiles;

    protected:
        CString CurrentConvertProfileName;
#endif
    protected:
        void BindToManager();
        bool PostSaveSettings(SimpleXml &xml) override;
        bool PostLoadSettings(SimpleXml &xml) override;
        bool LoadServerProfiles(SimpleXmlNode root);
        bool SaveServerProfiles(SimpleXmlNode root);
        void LoadServerProfile(SimpleXmlNode root, ServerProfile& profile);
        bool LoadServerProfileGroup(SimpleXmlNode root, ServerProfileGroup& group);
        bool SaveServerProfileGroup(SimpleXmlNode root, ServerProfileGroup& group);
        void PostLoadServerProfileGroup(ServerProfileGroup& profile);
        virtual void PostLoadServerProfile(ServerProfile& profile);
    public:
        static const char VideoEngineDirectshow[];
        static const char VideoEngineDirectshow2[];
        static const char VideoEngineFFmpeg[];
        static const char VideoEngineAuto[];
        static bool IsFFmpegAvailable();
    };
#endif
