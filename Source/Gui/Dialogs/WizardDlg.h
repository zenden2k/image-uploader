/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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
#ifndef WIZARDDLG_H
#define WIZARDDLG_H


#pragma once

#include <boost/signals2.hpp>

#include "atlheaders.h"
#include "Func/MyEngineList.h"
#include "HotkeySettings.h"
#include "Core/ScreenCapture.h"
#include "resource.h"       // main symbols
#include "ScreenshotDlg.h"
#include "Gui/Dialogs/UpdateDlg.h"
#include "Core/ProgramWindow.h"
#include "Core/TaskDispatcher.h"
#include "FolderAddDlg.h"
#include "Core/Upload/ServerProfileGroup.h"
#include "Gui/HwndScopedWrapper.h"
#include "Gui/Controls/IconButton.h"
#include "Gui/CommonDefines.h"
#include "Gui/Controls/DialogIndirect.h"
#include "Gui/Components/DragndropOverlay.h"

constexpr int ID_PASTE = 9888;
constexpr int ID_HOTKEY_BASE = 10000;
constexpr int WM_MY_ADDIMAGE = WM_USER + 222;
constexpr int WM_MY_SHOWPAGE = WM_USER + 223;
constexpr int WM_MY_EXIT = WM_USER + 224;
constexpr int WM_TASKDISPATCHERMSG = WM_USER + 225;

// Forward declarations
class CFolderAdd;
class CWizardPage;
class CWizardDlg;
class CUpdateDlg;
class CStatusDlg;
class DefaultLogger;
class UploadManager;
class UploadEngineManager;
class ScriptsManager;
class Win7JumpList;
class WtlGuiSettings;
class CFloatingWindow;

class CWizardDlg : 
    public CCustomDialogIndirectImpl<CWizardDlg>, public CUpdateUI<CWizardDlg>,
        public CMessageFilter, public CIdleHandler, public IDropTarget, public CRegionSelectCallback,
        public CUpdateDlg::CUpdateDlgCallback,
        public IProgramWindow,
        public ITaskRunner
{
public:
    enum { IDD = IDD_WIZARDDLG };
    enum { IDM_OPENSCREENSHOTS_FOLDER = 9889, IDM_OPENSERVERSCHECKER };
    enum { kNewFilesTimer = 1 };
    static const WPARAM kWmMyExitParam = 5;

    enum WizardPageId { wpWelcomePage = 0, wpVideoGrabberPage = 1, wpMainPage = 2, wpUploadSettingsPage = 3, wpUploadPage = 4 };

    struct AddImageStruct
    {
        CString RealFileName, VirtualFileName;
        bool show; 
    };

    CWizardDlg(std::shared_ptr<DefaultLogger> logger, CMyEngineList* enginelist, 
        UploadEngineManager* uploadEngineManager, UploadManager* uploadManager, 
        ScriptsManager* scriptsManager, WtlGuiSettings* settings);
    ~CWizardDlg() override;

    void setFloatWnd(std::shared_ptr<CFloatingWindow> floatWnd);

    BOOL PreTranslateMessage(MSG* pMsg) override;
    BOOL OnIdle() override;

    BEGIN_UPDATE_UI_MAP(CWizardDlg)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CWizardDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_ENABLE, OnEnable)
        MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
        MESSAGE_HANDLER(WM_MY_ADDIMAGE, OnAddImages)
        MESSAGE_HANDLER(WM_MY_SHOWPAGE, OnWmShowPage)
        MESSAGE_HANDLER(WM_MY_EXIT, OnWmMyExit)
        MESSAGE_HANDLER(WM_TASKDISPATCHERMSG, OnTaskDispatcherMsg)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(MYWM_ENABLEDROPTARGET, OnEnableDropTarget)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
        COMMAND_HANDLER(IDC_UPDATESLABEL, BN_CLICKED, OnUpdateClicked)
        COMMAND_HANDLER(IDM_OPENSCREENSHOTS_FOLDER, BN_CLICKED, OnOpenScreenshotFolderClicked)
        COMMAND_HANDLER(IDM_OPENSERVERSCHECKER, BN_CLICKED, OnServersCheckerClicked)
        
        COMMAND_HANDLER(ID_PASTE, 1, OnPaste)
        COMMAND_RANGE_HANDLER(ID_HOTKEY_BASE, ID_HOTKEY_BASE +100, OnLocalHotkey);
        COMMAND_HANDLER_EX(IDC_PREV, BN_CLICKED, OnPrevBnClicked)
        COMMAND_HANDLER_EX(IDC_NEXT, BN_CLICKED, OnNextBnClicked)
        COMMAND_HANDLER(IDC_ABOUT, BN_CLICKED, OnBnClickedAbout)
        COMMAND_HANDLER(IDC_HELPBUTTON, BN_CLICKED, OnBnClickedHelpbutton)
        COMMAND_ID_HANDLER(IDC_DOCUMENTATION, OnDocumentation)
        COMMAND_ID_HANDLER(IDC_SHOWLOG, OnShowLog)
        REFLECT_NOTIFICATIONS()

    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAddImages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnWmShowPage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTaskDispatcherMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEnable(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnUpdateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnLocalHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnWmMyExit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDocumentation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnShowLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOpenScreenshotFolderClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnServersCheckerClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnEnableDropTarget(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnBnClickedHelpbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void CloseDialog(int nVal);
    bool CreatePage(WizardPageId PageID);
    void setSessionImageServer(const ServerProfileGroup& server);
    void setSessionFileServer(const ServerProfileGroup& server);
    ServerProfileGroup getSessionImageServer() const;
    ServerProfileGroup getSessionFileServer() const;

    // IProgramWindow methods
    void setServersChanged(bool changed) override;
    bool serversChanged() const;
    WindowHandle getHandle() override;
    WindowNativeHandle getNativeHandle() override;

    void ShowUpdateMessage(const CString& msg) override;

    void runInGuiThread(TaskRunnerTask&& task, bool async) override;

    template<class T> T* getPage(WizardPageId id) {
        if (id < 0 || id >= ARRAY_SIZE(Pages)) {
            return nullptr;
        }
        return dynamic_cast<T*>(Pages[id]);
    }
    void settingsChanged(BasicSettings* settings);
    bool pasteFromClipboard();
    bool ShowPage(WizardPageId idPage, int prev = -1, int next = -1);
    bool AddImage(const CString &FileName, const CString &VirtualFileName, bool Show=true);
    bool AddImageAsync(const CString &FileName, const CString &VirtualFileName, bool show);
    LRESULT OnPrevBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnNextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    HBITMAP GenHeadBitmap(WizardPageId PageID) const;
    void PasteBitmap(HBITMAP);
    LRESULT OnBnClickedAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void Exit();
    LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    //CSavingOptions SavingOptions;
    bool LoadUploadEngines(const CString &filename, CString &Error);
    bool ParseCmdLine();
    bool CommonScreenshot(ScreenCapture::CaptureMode mode);
    // functions
    bool funcAddImages(bool AnyFiles = false);
    bool funcImportVideo();
    bool funcScreenshotDlg();
    bool funcRegionScreenshot(bool ShowAfter = true);
    bool funcFullScreenshot();
    bool funcWindowHandleScreenshot();
    bool funcFreeformScreenshot();
    bool funcWindowScreenshot(bool Delay = false);
    bool funcLastRegionScreenshot();
    bool funcAddFolder();
    //bool funcPaste();
    bool funcSettings();
#ifdef IU_ENABLE_MEDIAINFO
    bool funcMediaInfo();
#endif
    bool funcAddFiles();
    bool funcDownloadImages();
    bool funcReuploadImages();
    bool funcShortenUrl();
    bool funcOpenScreenshotFolder();
    bool funcFromClipboard(bool fromCmdLine = false);
    bool funcExit();
    // end of functions
    bool executeFunc(CString funcName, bool fromCmdLine = false);

    bool importVideoFile(const CString& fileName, int prevPage = 0);

    bool HandleDropFiledescriptors(IDataObject *pDataObj);
    bool HandleDropHDROP(IDataObject *pDataObj);
    bool HandleDropBitmap(IDataObject *pDataObj);
    void setIsFirstRun(bool isFirstRun);

    bool CanShowWindow() override;
    void UpdateAvailabilityChanged(bool Available) override;

    //    IUnknown methods
    STDMETHODIMP_(ULONG) AddRef() override;

    STDMETHODIMP_(ULONG) Release() override;
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    //    IDropTarget methods
    STDMETHODIMP DragEnter(
        /* [unique][in] */ IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect) override;
    STDMETHODIMP DragOver(
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect) override;

    STDMETHODIMP DragLeave(void) override;
    STDMETHODIMP Drop(
        /* [unique][in] */ IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect) override;

    void AddFolder(LPCTSTR szFolder, bool SubDirs = false);

    //CRegionSelectCallback
    void OnScreenshotFinished(int Result) override;
    void OnScreenshotSaving(LPTSTR FileName, Gdiplus::Bitmap* Bm) override;
    void CloseWizard();
    bool RegisterLocalHotkeys();
    bool UnRegisterLocalHotkeys();
    void CreateUpdateDlg();
    bool IsClipboardDataAvailable();
    void showLogWindowForFileName(CString fileName);
    bool hasLastScreenshotRegion() const;
    void setLastScreenshotRegion(std::shared_ptr<ScreenCapture::CScreenshotRegion> region, HMONITOR monitor);
    void addLastRegionAvailabilityChangeCallback(std::function<void(bool)> cb);
    bool getQuickUploadMarker() const;
    void setQuickUploadMarker(bool val);
    CString getLastVideoFile() const;
    void setLastVideoFile(CString fileName);
    bool isShowWindowSet() const;
    void beginAddFiles();
    void endAddFiles();
protected:
    bool acceptsDragnDrop() const;
    CIcon hIcon;
    CIcon hIconSmall;
    CHotkeyList m_hotkeys;
    CFolderAdd FolderAdd;
   
    UploadManager* uploadManager_;
    UploadEngineManager* uploadEngineManager_;
    ScriptsManager* scriptsManager_;
	std::unique_ptr<Win7JumpList> win7JumpList_;
    HwndScopedWrapper aboutButtonToolTip_;
    CButton helpButton_;
    CIcon helpButtonIcon_;
    long m_lRef = 0;
    bool QuickUploadMarker;
    CString LastVideoFile;
    bool m_bShowAfter;
    bool isFirstRun_;
    WtlGuiSettings& Settings;

    std::map<CString, CLogWindow*> logWindowsByFileName_;
    std::vector<AddImageStruct> newImages_;
    std::mutex newImagesMutex_;
    std::shared_ptr<ScreenCapture::CScreenshotRegion> lastScreenshotRegion_;
    HMONITOR lastScreenshotMonitor_;
    std::vector<std::function<void(bool)>> lastRegionAvailabilityChangeCallbacks_;
    std::shared_ptr<DefaultLogger> logger_;
    std::unique_ptr<CStatusDlg> statusDlg_;
    std::vector<TaskRunnerTask> scheduledTasks_;
    std::mutex scheduledTasksMutex_;
    std::shared_ptr<CFloatingWindow> floatWnd_;
    DWORD mainThreadId_;
    std::unique_ptr<CUpdateDlg> updateDlg;
    HACCEL hLocalHotkeys;
    INT m_bScreenshotFromTray;
    bool m_bShowWindow;
    bool m_bHandleCmdLineFunc;
    ServerProfileGroup sessionImageServer_, sessionFileServer_;
    bool serversChanged_;
    int CurPage;
    int PrevPage, NextPage;
    bool DragndropEnabled;
    CStringList m_Paths;
    CWizardPage* Pages[5];
    int screenshotIndex;
    CString m_bCurrentFunc;
    CMyEngineList* enginelist_;
    boost::signals2::connection settingsChangedConnection_;
    CDragndropOverlay dragndropOverlay_;
    bool enableDragndropOverlay_ = false;
    CDragndropOverlay::ITEM_ID dragndropOverlaySelectedItem_ = CDragndropOverlay::kInvalid;
};



#endif // WIZARDDLG_H
