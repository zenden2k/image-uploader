#ifndef IU_CORE_SETTINGS_COMMONGUISETTINGS_H
#define IU_CORE_SETTINGS_COMMONGUISETTINGS_H

#pragma once


#include <map>
#include <string>
#include "Core/SettingsManager.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/Upload/ServerProfile.h"
#include "BasicSettings.h"

#ifdef IU_QT
    #include <QString>
    typedef QString SettingsString;
#else
    #include "atlheaders.h"
    typedef CString SettingsString;
#endif

#include "EncodedPassword.h"

#include "Core/Images/ImageConverter.h"

struct UploadProfileStruct {
    bool GenThumb;
    bool KeepAsIs;
    int ServerID, QuickServerID;
};

typedef std::map<CString, ServerProfile> ServerProfilesMap;

struct FullUploadProfile {
    ServerProfile upload_profile;
    ImageConvertingParams convert_profile;
};

struct ThumbSettingsStruct : public ThumbCreatingParams {
    TCHAR FontName[256];
    BOOL UseServerThumbs;
    bool CreateThumbs;
};

struct VideoSettingsStruct {
    int Columns;
    int TileWidth;
    int GapWidth;
    int GapHeight;
    int NumOfFrames;
    int JPEGQuality;
    BOOL UseAviInfo;
    BOOL ShowMediaInfo;
    LOGFONT Font;
    COLORREF TextColor;
    CString Engine;
    CString SnapshotsFolder;
    CString SnapshotFileTemplate;
};

struct HistorySettingsStruct {
    bool EnableDownloading;
};
 
class CommonGuiSettings : public BasicSettings {
    public:   
        CommonGuiSettings();
        ~CommonGuiSettings();

        UploadProfileStruct UploadProfile;
        bool UseDirectLinks;
        CString DataFolder;
        CString m_SettingsDir;
        CString Language;

#ifndef IU_SERVERLISTTOOL   
        bool ShowTrayIcon;
        bool ShowTrayIcon_changed;
        bool AutoStartup;
        bool AutoStartup_changed;
        int ThumbsPerLine;
        TCHAR m_szLang[64];
        VideoSettingsStruct VideoSettings;
        ServerProfile imageServer, fileServer, quickScreenshotServer, contextMenuServer, urlShorteningServer, temporaryServer;
        HistorySettingsStruct HistorySettings;

        int CodeLang;
        int CodeType;

        bool UseProxyServer;
        bool IsPortable;

        CString VideoFolder, ImagesFolder;

        std::map<CString, ImageConvertingParams> ConvertProfiles;
        ServerProfilesMap ServerProfiles;
    protected:
        CString CurrentConvertProfileName;
    public:

#endif
    public:
        static const TCHAR VideoEngineDirectshow[];
        static const TCHAR VideoEngineFFmpeg[];
        static const TCHAR VideoEngineAuto[];
        static bool IsFFmpegAvailable();
    };
#endif