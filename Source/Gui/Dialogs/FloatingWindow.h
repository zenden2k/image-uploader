/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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

#ifndef IU_GUI_DIALOGS_FLOATINGWINDOW_H
#define IU_GUI_DIALOGS_FLOATINGWINDOW_H

// This file was generated by WTL subclass control wizard
// FloatingWindow.h : Declaration of the FloatingWindow

#pragma once
#include <memory>
#include "atlheaders.h"

#include "Gui/Components/trayicon.h"
#include "SettingsDlg.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Settings/WtlGuiSettings.h"

// FloatingWindow

constexpr int IDM_UPLOADFILES = 20001;
constexpr int IDM_IMPORTVIDEO = IDM_UPLOADFILES + 1;
constexpr int IDM_SCREENSHOT = IDM_UPLOADFILES + 2;
constexpr int IDM_SCREENSHOTDLG = IDM_UPLOADFILES + 3;
constexpr int IDM_REGIONSCREENSHOT = IDM_UPLOADFILES + 4;
constexpr int IDM_FULLSCREENSHOT = IDM_UPLOADFILES + 5;
constexpr int IDM_ACTIVEWINDOWSCREENSHOT = IDM_UPLOADFILES + 6;
constexpr int IDM_WINDOWHANDLESCREENSHOT = IDM_UPLOADFILES + 7;
constexpr int IDM_FREEFORMSCREENSHOT = IDM_UPLOADFILES + 8;
constexpr int IDM_ADDFOLDERS = IDM_UPLOADFILES + 9;
constexpr int IDM_SHOWAPPWINDOW = IDM_UPLOADFILES + 10;
constexpr int IDM_SETTINGS = IDM_UPLOADFILES + 11;
constexpr int IDM_EXIT = IDM_UPLOADFILES + 12;
constexpr int IDM_CONTEXTMENU = IDM_UPLOADFILES + 13;
constexpr int IDM_PASTEFROMCLIPBOARD = IDM_UPLOADFILES + 14;
constexpr int IDM_MEDIAINFO = IDM_UPLOADFILES + 15;
constexpr int IDM_UPLOADIMAGES = IDM_UPLOADFILES + 16;
constexpr int IDM_SCREENTSHOTACTION_UPLOAD = IDM_UPLOADFILES + 17;
constexpr int IDM_SCREENTSHOTACTION_TOCLIPBOARD = IDM_UPLOADFILES + 18;
constexpr int IDM_SCREENTSHOTACTION_SHOWWIZARD = IDM_UPLOADFILES + 19;
constexpr int IDM_SCREENTSHOTACTION_ADDTOWIZARD = IDM_UPLOADFILES + 20;
constexpr int IDM_SCREENTSHOTACTION_OPENINEDITOR = IDM_UPLOADFILES + 21;
constexpr int IDM_PASTEFROMWEB = IDM_UPLOADFILES + 22;
constexpr int IDM_STOPUPLOAD = IDM_UPLOADFILES + 23;
constexpr int IDM_REUPLOADIMAGES = IDM_UPLOADFILES + 24;
constexpr int IDM_SHORTENURL = IDM_REUPLOADIMAGES + 25;
constexpr int IDM_SHORTENURLCLIPBOARD = IDM_REUPLOADIMAGES + 26;
constexpr int IDM_OPENSCREENSHOTSFOLDER = IDM_REUPLOADIMAGES + 27;
constexpr int IDM_SHOWLASTUPLOADRESULTS = IDM_REUPLOADIMAGES + 28;
constexpr int IDM_MONITOR_ALLMONITORS = IDM_REUPLOADIMAGES + 29;
constexpr int IDM_MONITOR_CURRENTMONITOR = IDM_REUPLOADIMAGES + 30;
constexpr int IDM_QUICKUPLOADFROMCLIPBOARD = IDM_UPLOADFILES + 31;
constexpr int IDM_LASTREGIONSCREENSHOT = IDM_UPLOADFILES + 32;
constexpr int IDM_TOPWINDOWSCREENSHOT = IDM_UPLOADFILES + 33;
constexpr int IDM_SCREENRECORDINGDIALOG = IDM_UPLOADFILES + 34;
constexpr int IDM_SCREENRECORDINGSTART = IDM_UPLOADFILES + 35;
constexpr int IDM_SCREENRECORDINGSTOP = IDM_UPLOADFILES + 36;
constexpr int IDM_MONITOR_SELECTEDMONITOR_FIRST = IDM_REUPLOADIMAGES + 50;
constexpr int IDM_MONITOR_SELECTEDMONITOR_LAST = IDM_REUPLOADIMAGES + 50 + 25;

constexpr int WM_CLOSETRAYWND = WM_USER + 2;
constexpr int WM_RELOADSETTINGS = WM_USER + 3;

namespace Uptooda::Core::OutputGenerator {
class XmlTemplateList;
class XmlTemplateGenerator;
struct UploadObject;
}

class UrlShorteningTask;
class IconBitmapUtils;

class CFloatingWindow :
    public CWindowImpl<CFloatingWindow>,
    public CTrayIconImpl<CFloatingWindow>
{
public:
    HANDLE hMutex;
    HWND m_ActiveWindow;
    HMENU m_hTrayIconMenu;
    UINT WM_TASKBARCREATED;
    bool EnableClicks;
    HWND m_PrevActiveWindow;
    CHotkeyList m_hotkeys;
    CIcon m_hIconSmall;
    CIcon activeIcon_;
    bool m_bStopCapturingWindows;
    bool m_bIsUploading;
    std::unique_ptr<Uptooda::Core::OutputGenerator::UploadObject> lastUploadedItem_;
    std::shared_ptr<UrlShorteningTask> lastUrlShorteningTask_;
    CString imageUrlShortened_;
    CString downloadUrlShortened_;

    CFloatingWindow(CWizardDlg* wizardDlg, UploadManager* uploadManager, UploadEngineManager* uploadEngineManager);
    ~CFloatingWindow();

    DECLARE_WND_CLASS(_T("CFloatingWindow"))

    BEGIN_MSG_MAP(CFloatingWindow)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_CREATE(OnCreate)
        COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)
        COMMAND_ID_HANDLER_EX(IDM_SETTINGS, OnMenuSettings)
        COMMAND_ID_HANDLER_EX(IDM_IMPORTVIDEO, OnImportvideo)
        COMMAND_ID_HANDLER_EX(IDM_UPLOADFILES, OnUploadFiles)
        COMMAND_ID_HANDLER_EX(IDM_UPLOADIMAGES, OnUploadImages)
        COMMAND_ID_HANDLER_EX(IDM_REUPLOADIMAGES, OnReUploadImages)
        COMMAND_ID_HANDLER_EX(IDM_SHORTENURL, OnShortenUrl)
        COMMAND_ID_HANDLER_EX(IDM_SHORTENURLCLIPBOARD, OnShortenUrlClipboard)
        COMMAND_ID_HANDLER_EX(IDM_SCREENSHOTDLG, OnScreenshotDlg)
        COMMAND_ID_HANDLER_EX(IDM_REGIONSCREENSHOT, OnRegionScreenshot)
        COMMAND_ID_HANDLER_EX(IDM_FULLSCREENSHOT, OnFullScreenshot)
        COMMAND_ID_HANDLER_EX(IDM_WINDOWHANDLESCREENSHOT, OnWindowHandleScreenshot)
        COMMAND_ID_HANDLER_EX(IDM_FREEFORMSCREENSHOT, OnFreeformScreenshot)
        COMMAND_ID_HANDLER_EX(IDM_ACTIVEWINDOWSCREENSHOT, OnActiveWindowScreenshot)
        COMMAND_ID_HANDLER_EX(IDM_TOPWINDOWSCREENSHOT, OnWindowScreenshot)
        COMMAND_ID_HANDLER_EX(IDM_LASTREGIONSCREENSHOT, OnLastRegionScreenshot)
        COMMAND_ID_HANDLER_EX(IDM_ADDFOLDERS, OnAddFolder)
        COMMAND_ID_HANDLER_EX(IDM_SHOWAPPWINDOW, OnShowAppWindow)
        COMMAND_ID_HANDLER_EX(IDM_OPENSCREENSHOTSFOLDER, OnOpenScreenshotsFolder)
        COMMAND_ID_HANDLER_EX(IDM_PASTEFROMWEB, OnPasteFromWeb)
        COMMAND_ID_HANDLER_EX(IDM_CONTEXTMENU, OnContextMenu)
        COMMAND_ID_HANDLER_EX(IDM_PASTEFROMCLIPBOARD, OnPaste)
        COMMAND_ID_HANDLER_EX(IDM_QUICKUPLOADFROMCLIPBOARD, OnQuickUploadFromClipboard)
        COMMAND_ID_HANDLER_EX(IDM_STOPUPLOAD, OnStopUpload)
        COMMAND_ID_HANDLER_EX(IDM_MEDIAINFO, OnMediaInfo)
        COMMAND_ID_HANDLER_EX(IDM_SHOWLASTUPLOADRESULTS, OnShowLastUploadResults)
        COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_UPLOAD, OnScreenshotActionChanged)
        COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_TOCLIPBOARD, OnScreenshotActionChanged)
        COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_SHOWWIZARD, OnScreenshotActionChanged)
        COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_ADDTOWIZARD, OnScreenshotActionChanged)
        COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_OPENINEDITOR, OnScreenshotActionChanged)
        COMMAND_ID_HANDLER_EX(IDM_MONITOR_ALLMONITORS, OnMonitorAllMonitors)
        COMMAND_ID_HANDLER_EX(IDM_MONITOR_CURRENTMONITOR, OnMonitorCurrentMonitor)
        COMMAND_RANGE_HANDLER(IDM_MONITOR_SELECTEDMONITOR_FIRST, IDM_MONITOR_SELECTEDMONITOR_LAST, OnMonitorSelectedMonitor)
        COMMAND_ID_HANDLER_EX(IDM_SCREENRECORDINGDIALOG, OnScreenRecordingDialog)
        COMMAND_ID_HANDLER_EX(IDM_SCREENRECORDINGSTART, OnScreenRecordingStart)
        COMMAND_ID_HANDLER_EX(IDM_SCREENRECORDINGSTOP, OnScreenRecordingStop)
        MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
        MESSAGE_HANDLER_EX(WM_CLOSETRAYWND, OnCloseTray)
        MESSAGE_HANDLER_EX(WM_RELOADSETTINGS, OnReloadSettings)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_HOTKEY(OnHotKey)
        MESSAGE_HANDLER_EX(WM_TASKBARCREATED, OnTaskbarCreated)

        //CHAIN_MSG_MAP(CTrayIconImpl<CFloatingWindow>)
    END_MSG_MAP()

    enum {
        kStatusTimer = 3,
        kIconAnimationTimer = 4,
        kStatusHideTimeout = 20000 // 20 sec
    };
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
     LRESULT OnClose(void);
     LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
     LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
     LRESULT OnMenuSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnCloseTray(UINT uMsg, WPARAM wParam, LPARAM lParam);
     LRESULT OnReloadSettings(UINT uMsg, WPARAM wParam, LPARAM lParam);
     LRESULT OnImportvideo(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnUploadFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnUploadImages(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnReUploadImages(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnScreenshotDlg(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnRegionScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnFullScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnActiveWindowScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnFreeformScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnWindowScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnWindowHandleScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnLastRegionScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnAddFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnShowAppWindow(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnOpenScreenshotsFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnContextMenu(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnMediaInfo(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnScreenshotActionChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnStopUpload(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnShortenUrl(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnShortenUrlClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnShowLastUploadResults(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam);
     LRESULT OnMonitorAllMonitors(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnMonitorCurrentMonitor(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnMonitorSelectedMonitor(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL&);
     LRESULT OnScreenRecordingDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnScreenRecordingStart(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnScreenRecordingStop(WORD wNotifyCode, WORD wID, HWND hWndCtl);

     LRESULT OnTimer(UINT id);
     void CreateTrayIcon();
     void RegisterHotkeys();
     void UnRegisterHotkeys();
     LRESULT OnHotKey(int HotKeyID, UINT flags, UINT vk);
     LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnPasteFromWeb(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     LRESULT OnQuickUploadFromClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl);
     void UploadScreenshot(const CString& realName, const CString &displayName);

     // Text displayed in tray icon tooltip
     void setStatusText(const CString& text, int timeoutMs = 0);
     void removeStatusText();
     void ShowBaloonTip(const CString& text, const CString& title, unsigned int timeout, const std::function<void()>& onClick=std::function<void()>());
     void startIconAnimation();
     void stopIconAnimation();
     void showLastUploadedCode();
     void onUploadSessionFinished(UploadSession* session);
     static BOOL IsRunningFloatingWnd();

     bool animationEnabled_;

     CString fileName, realFileName;

     // Text displayed in tray icon tooltip
     CString statusText_;
     UploadManager* uploadManager_;
     UploadEngineManager* uploadEngineManager_;
     bool m_bFromHotkey;
     void OnFileFinished(UploadTask*  task, bool ok);
     void ShowImageUploadedMessage(UploadTask* task, const CString& url);
     void ShowScreenshotCopiedToClipboardMessage();
     bool canExitApp();

     int iconAnimationCounter_;
     std::shared_ptr<UploadSession> currentUploadSession_;

     struct UploadTaskUserData {
        CString linkTypeToShorten;
     };

     enum UploadType {
         utImage, utUrl, utShorteningImageUrl
     };
    CWizardDlg* wizardDlg_;
    std::function<void()> balloonClickFunction_;
    std::unique_ptr<Uptooda::Core::OutputGenerator::XmlTemplateList> templateList_;
    protected:
    std::unique_ptr<IconBitmapUtils> iconBitmapUtils_;
    CBitmap addFilesBitmap_, screenshotBitmap_, regionScreenshotBitmap_, screenRecordingBitmap_, startRecordingBitmap_, settingsBitmap_;
};

#endif

