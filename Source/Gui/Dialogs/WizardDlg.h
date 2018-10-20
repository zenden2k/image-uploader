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
#ifndef WIZARDDLG_H
#define WIZARDDLG_H


#pragma once

#include "atlheaders.h"
#include "statusdlg.h"

#include "Func/MyEngineList.h"
#include "HotkeySettings.h"
#include "Core/ScreenCapture.h"
#include "resource.h"       // main symbols
#include "screenshotdlg.h"
#include "Gui/Dialogs/UpdateDlg.h"
#include "Core/Settings.h"
#include "Core/ProgramWindow.h"
#include "Core/TaskDispatcher.h"
#include "FolderAddDlg.h"
#include "Gui/HwndScopedWrapper.h"
#include "Gui/Controls/IconButton.h"
#include "Gui/CommonDefines.h"

#define ID_PASTE 9888
#define ID_HOTKEY_BASE 10000
#define WM_MY_ADDIMAGE WM_USER + 222
#define WM_MY_SHOWPAGE WM_USER + 223
#define WM_MY_EXIT WM_USER + 224
#define WM_TASKDISPATCHERMSG WM_USER + 225



// CWizardDlg
class CFolderAdd;
class CWizardPage;
class CWizardDlg;
class CUpdateDlg;

class CMyFolderDialog : public CFolderDialogImpl<CMyFolderDialog> {
public:
    void OnInitialized();
    static BOOL CALLBACK DialogProc(

        HWND hwndDlg,    // handle to dialog box
        UINT uMsg,    // message
        WPARAM wParam,    // first message parameter
        LPARAM lParam     // second message parameter
        );
    DLGPROC OldProc;
public:
    CMyFolderDialog(HWND hWnd);
    bool m_bSubdirs;
    HWND SubdirsCheckbox;

};
extern TCHAR MediaInfoDllPath[MAX_PATH];
class UploadManager;
class UploadEngineManager;
class ScriptsManager;
class Win7JumpList;
class CWizardDlg : 
    public CDialogImpl<CWizardDlg>    , public CUpdateUI<CWizardDlg>,
        public CMessageFilter, public CIdleHandler, public IDropTarget, public CRegionSelectCallback,
        public CUpdateDlg::CUpdateDlgCallback,
        public IProgramWindow,
        public ITaskDispatcher
{
public:
    struct AddImageStruct
    {
        CString RealFileName, VirtualFileName;
        bool show;
    };

    void runInGuiThread(TaskDispatcherTask&& task, bool async) override;
    CWizardDlg();
    virtual ~CWizardDlg();
    CStringList m_Paths;
    enum { IDD = IDD_WIZARDDLG };
    enum { IDM_OPENSCREENSHOTS_FOLDER = 9889 };
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnIdle();
    CString m_bCurrentFunc;
    BEGIN_UPDATE_UI_MAP(CWizardDlg)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CWizardDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_ENABLE, OnEnable)
        MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
        MESSAGE_HANDLER(WM_MY_ADDIMAGE, OnAddImages)
        MESSAGE_HANDLER(WM_MY_SHOWPAGE, OnWmShowPage)
        MESSAGE_HANDLER(WM_MY_EXIT, OnWmMyExit)
        MESSAGE_HANDLER(WM_TASKDISPATCHERMSG, OnTaskDispatcherMsg)
        MESSAGE_HANDLER(MYWM_ENABLEDROPTARGET, OnEnableDropTarget)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
        COMMAND_HANDLER(IDC_UPDATESLABEL, BN_CLICKED, OnUpdateClicked)
        COMMAND_HANDLER(IDM_OPENSCREENSHOTS_FOLDER, BN_CLICKED, OnOpenScreenshotFolderClicked)
        
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
    LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnWmShowPage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTaskDispatcherMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEnable(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnUpdateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnLocalHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnWmMyExit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDocumentation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnShowLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOpenScreenshotFolderClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnEnableDropTarget(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void CloseDialog(int nVal);
    bool DragndropEnabled;
    enum WizardPageId { wpWelcomePage = 0, wpVideoGrabberPage = 1, wpMainPage = 2, wpUploadSettingsPage = 3, wpUploadPage = 4 };
    int CurPage;
    int PrevPage,NextPage;
    bool CreatePage(WizardPageId PageID);
    CWizardPage* Pages[5];
    int screenshotIndex; 
    void setSessionImageServer(ServerProfile server);
    void setSessionFileServer(ServerProfile server);
    ServerProfile getSessionImageServer() const;
    ServerProfile getSessionFileServer() const;
    void setServersChanged(bool changed) override;
    bool serversChanged() const;
    virtual WindowHandle getHandle() override;
    virtual WindowNativeHandle getNativeHandle() override;
    virtual void ShowUpdateMessage(const CString& msg) override;
protected:
    ServerProfile sessionImageServer_, sessionFileServer_;
    bool serversChanged_;
    void settingsChanged(BasicSettings* settings);
    bool pasteFromClipboard();
public:
    bool ShowPage(WizardPageId idPage, int prev = -1, int next = -1);
    bool AddImage(const CString &FileName, const CString &VirtualFileName, bool Show=true);
    LRESULT OnPrevBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnNextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    HBITMAP GenHeadBitmap(int PageID);
    void PasteBitmap(HBITMAP);
    int m_StartingThreadId;
    LRESULT OnBnClickedAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void Exit();
    LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    //CSavingOptions SavingOptions;
    bool LoadUploadEngines(const CString &filename, CString &Error);
    bool ParseCmdLine();
    CIcon hIcon;
    CIcon hIconSmall;
    CHotkeyList m_hotkeys;
    CFolderAdd FolderAdd;
    CMyEngineList m_EngineList;
    UploadManager* uploadManager_;
    UploadEngineManager* uploadEngineManager_;
    ScriptsManager* scriptsManager_;
	std::unique_ptr<Win7JumpList> win7JumpList_;
    HwndScopedWrapper aboutButtonToolTip_;
    CIconButton helpButton_;
    CIcon helpButtonIcon_;
    long m_lRef;
    bool QuickUploadMarker;
    CString LastVideoFile;
    bool m_bShowAfter;
    bool CommonScreenshot(CaptureMode mode);
    // functions
    bool funcAddImages(bool AnyFiles = false);
    bool funcImportVideo();
    bool funcScreenshotDlg();
    bool funcRegionScreenshot(bool ShowAfter=true);
    bool funcFullScreenshot();
    bool funcWindowHandleScreenshot();
    bool funcFreeformScreenshot();
    bool funcWindowScreenshot(bool Delay=false);
    bool funcAddFolder();
    bool funcPaste();
    bool funcSettings();
    bool funcMediaInfo();
    bool funcAddFiles();
    bool funcDownloadImages();
    bool funcReuploadImages();
    bool funcShortenUrl();
    bool funcOpenScreenshotFolder();
    bool funcFromClipboard();
    // end of functions
    bool executeFunc(CString funcName);

	bool importVideoFile(const CString& fileName, int prevPage = 0);

    bool HandleDropFiledescriptors(IDataObject *pDataObj);
    bool HandleDropHDROP(IDataObject *pDataObj);
    bool HandleDropBitmap(IDataObject *pDataObj);

    public:
    CUpdateDlg *updateDlg;
    bool CanShowWindow() override;
    void UpdateAvailabilityChanged(bool Available) override;
    //CSpecialHyperLink m_UpdateLink;
    HACCEL hLocalHotkeys;
    //    IUnknown methods
         STDMETHODIMP_(ULONG) AddRef();

         STDMETHODIMP_(ULONG) Release();
         STDMETHODIMP QueryInterface( REFIID riid, void** ppv );
    //    IDropTarget methods
         STDMETHODIMP DragEnter( 
              /* [unique][in] */ IDataObject *pDataObj,
              /* [in] */ DWORD grfKeyState,
              /* [in] */ POINTL pt,
              /* [out][in] */ DWORD *pdwEffect);
         STDMETHODIMP DragOver( 
              /* [in] */ DWORD grfKeyState,
              /* [in] */ POINTL pt,
              /* [out][in] */ DWORD *pdwEffect);

         STDMETHODIMP DragLeave( void);
         STDMETHODIMP Drop( 
              /* [unique][in] */ IDataObject *pDataObj,
              /* [in] */ DWORD grfKeyState,
              /* [in] */ POINTL pt,
              /* [out][in] */ DWORD *pdwEffect);

    void AddFolder(LPCTSTR szFolder, bool SubDirs = false);
    
    //CRegionSelectCallback
    void OnScreenshotFinished(int Result) override;
    void OnScreenshotSaving(LPTSTR FileName, Gdiplus::Bitmap* Bm) override;
    void CloseWizard();
    bool RegisterLocalHotkeys();
    bool UnRegisterLocalHotkeys();
    bool m_bShowWindow;
    bool m_bHandleCmdLineFunc;
    void CreateUpdateDlg();
    INT m_bScreenshotFromTray;
    bool IsClipboardDataAvailable();

    LRESULT OnBnClickedHelpbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};



#endif // WIZARDDLG_H