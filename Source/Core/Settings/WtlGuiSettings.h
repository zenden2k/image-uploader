#ifndef IU_CORE_SETTINGS_WTLSETTINGS_H
#define IU_CORE_SETTINGS_WTLSETTINGS_H

#pragma once

#include "atlheaders.h"
#include "CommonGuiSettings.h"
#include "Func/WinUtils.h"
#include "Gui/Dialogs/HotkeySettings.h"
#include "Core/SearchByImage.h"
#include "3rdpart/GdiplusH.h" 
#include "Core/Images/Utils.h"

#define TRAY_SCREENSHOT_UPLOAD 0
#define TRAY_SCREENSHOT_CLIPBOARD 1
#define TRAY_SCREENSHOT_SHOWWIZARD 2
#define TRAY_SCREENSHOT_ADDTOWIZARD 3
#define TRAY_SCREENSHOT_OPENINEDITOR 4


class CFloatingWindow;

struct ImageReuploaderSettingsStruct {
    bool PasteHtmlOnCtrlV;
};

struct ImageEditorSettingsStruct {
    Gdiplus::Color ForegroundColor, BackgroundColor, StepForegroundColor, StepBackgroundColor;
    int PenSize;
    int RoundingRadius;
    LOGFONT Font;
    bool AllowAltTab;
    bool AllowEditingInFullscreen;
    SearchByImage::SearchEngine SearchEngine;
    bool FillTextBackground;
    int ArrowType;
};

struct TrayIconSettingsStruct {
    CString LeftDoubleClickCommandStr, LeftClickCommandStr, RightClickCommandStr, MiddleClickCommandStr;
    int TrayScreenshotAction;
    bool DontLaunchCopy;
};

struct ScreenshotSettingsStruct {
    int Format;
    int Quality, Delay;
    int WindowHidingDelay;
    bool ShowForeground;
    bool CopyToClipboard;
    COLORREF brushColor;
    CString FilenameTemplate;
    CString Folder;
    bool RemoveCorners;
    bool AddShadow;
    bool RemoveBackground;
    bool OpenInEditor; // only from screenshot dlg
    bool UseOldRegionScreenshotMethod;
    int MonitorMode;
};

inline std::string myToString(const CHotkeyList& value) {
    return IuCoreUtils::WstringToUtf8((LPCTSTR)value.toString());
}

inline void myFromString(const std::string& text, CHotkeyList& value) {
    value.DeSerialize(IuCoreUtils::Utf8ToWstring(text).c_str());
}

/* LOGFONT serialization support */
inline std::string myToString(const LOGFONT& value) {
    CString res;
    WinUtils::FontToString(&value, res);
    return IuCoreUtils::WstringToUtf8((LPCTSTR)res);
}

inline void myFromString(const std::string& text, LOGFONT& value) {
    CString wide_text = IuCoreUtils::Utf8ToWstring(text).c_str();
    LOGFONT font;
    WinUtils::StringToFont(wide_text, &font);
    value = font;
}

inline std::string myToString(const Gdiplus::Color& value)
{
    char buffer[30];
    sprintf(buffer, "rgba(%d,%d,%d,%1.4f)", (int)value.GetR(), (int)value.GetG(), (int)value.GetB(), (float)(value.GetA() / 255.0));
    return buffer;
}

inline void myFromString(const std::string& text, Gdiplus::Color& value)
{
    value = ImageUtils::StringToColor(text);
}

class WtlGuiSettings : public CommonGuiSettings {
public:
    WtlGuiSettings();
    ~WtlGuiSettings();

    void setFloatWnd(CFloatingWindow* floatWnd);
    ImageEditorSettingsStruct ImageEditorSettings;

    bool RememberImageServer;
    bool RememberFileServer;
    ScreenshotSettingsStruct ScreenshotSettings;
    ImageReuploaderSettingsStruct ImageReuploaderSettings;
    TrayIconSettingsStruct TrayIconSettings;
    CString ImageEditorPath;
    
    bool ExplorerContextMenu;
    bool ExplorerContextMenu_changed;
    bool ExplorerVideoContextMenu;
    bool DropVideoFilesToTheList;
    bool ConfirmOnExit;
    bool AutomaticallyCheckUpdates;

    bool AutoCopyToClipboard;
    bool WatchClipboard;
    bool ParseSubDirs;
    bool ExplorerCascadedMenu;
    CHotkeyList Hotkeys;
    bool Hotkeys_changed;

    static COLORREF DefaultLinkColor;
    bool QuickUpload;
    bool UseTxtTemplate;
    bool GroupByFilename;

    bool SendToContextMenu;
    bool SendToContextMenu_changed;

    std::string testFileName, testUrl;

    CString getServerName();
    CString getQuickServerName();
    CString getFileServerName();
    CString getSettingsFileName() const;

    static CString getShellExtensionFileName();

    void ApplyRegSettingsRightNow();
    void Uninstall();

    CString prepareVideoDialogFilters();

    ServerSettingsStruct& ServerByName(CString name);
    ServerSettingsStruct& ServerByUtf8Name(const std::string& name);
    void FindDataFolder();
	void fixInvalidServers();
protected:
    void EnableAutostartup(bool enable);
    bool LoadConvertProfiles(SimpleXmlNode root);
    bool LoadConvertProfile(const CString& name, SimpleXmlNode profileNode);
    bool SaveConvertProfiles(SimpleXmlNode root);
    void BindConvertProfile(SettingsNode& mgr, ImageConvertingParams &params);
    bool LoadServerProfiles(SimpleXmlNode root);
    bool SaveServerProfiles(SimpleXmlNode root);

    void LoadServerProfile(SimpleXmlNode root, ServerProfile& profile);
    bool LoadServerProfileGroup(SimpleXmlNode root, ServerProfileGroup& group);
    bool SaveServerProfileGroup(SimpleXmlNode root, ServerProfileGroup& group);
    bool PostLoadSettings(SimpleXml &xml) override;
    bool PostSaveSettings(SimpleXml &xml) override;
    void RegisterShellExtension(bool Register);

    void PostLoadServerProfile(ServerProfile& profile);
    void PostLoadServerProfileGroup(ServerProfileGroup& profile);

    void BindToManager();
private:
    TCHAR m_Directory[MAX_PATH];
    CFloatingWindow* floatWnd_;
};
#endif