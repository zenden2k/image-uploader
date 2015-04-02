/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "Func/Settings.h"

#define ID_PASTE 9888
#define ID_HOTKEY_BASE 10000
#define WM_MY_ADDIMAGE WM_USER + 222
#define WM_MY_SHOWPAGE WM_USER + 223
#define WM_MY_EXIT WM_USER + 224
// CWizardDlg

class CFolderAdd;
class CWizardPage;
class CWizardDlg;
class CUpdateDlg;

class CFolderAdd: public CThreadImpl<CFolderAdd>
{
	public:
		CFolderAdd(CWizardDlg *WizardDlg);
		void Do(CStringList &Paths, bool ImagesOnly, bool SubDirs  = false);
		DWORD Run();
	private:
		int count;
		CStringList m_Paths;
		bool m_bSubDirs;
		bool m_bImagesOnly;
		CWizardDlg *m_pWizardDlg;
		TCHAR m_szPath[MAX_PATH];
		WIN32_FIND_DATA wfd;
		HANDLE findfile;
			CStatusDlg dlg;
		int GetNextImgFile(LPTSTR szBuffer, int nLength);
		int ProcessDir( CString currentDir, bool bRecursive /* = true  */ );
};
class CMyFolderDialog: public CFolderDialogImpl<CMyFolderDialog>
{
public:
	void OnInitialized();
	static BOOL CALLBACK DialogProc(

    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   );
	DLGPROC OldProc;
	public:
		CMyFolderDialog(HWND hWnd);
		bool m_bSubdirs;
		HWND SubdirsCheckbox;

};
extern CWizardDlg * pWizardDlg;
extern TCHAR MediaInfoDllPath[MAX_PATH];


class CWizardDlg : 
	public CDialogImpl<CWizardDlg>	, public CUpdateUI<CWizardDlg>,
		public CMessageFilter, public CIdleHandler, public IDropTarget, public CRegionSelectCallback
		, public CUpdateDlg::CUpdateDlgCallback
{
public:
	struct AddImageStruct
	{
		CString RealFileName, VirtualFileName;
		bool show;
	};

	CWizardDlg();
	~CWizardDlg();
	CStringList m_Paths;
	enum { IDD = IDD_WIZARDDLG };
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
		
		MESSAGE_HANDLER(WM_ENABLE,OnEnable)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(IDC_UPDATESLABEL, BN_CLICKED, OnUpdateClicked)
		
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
	void CloseDialog(int nVal);
	bool DragndropEnabled;
	int CurPage;
	int PrevPage,NextPage;
	bool CreatePage(int PageID);
	CWizardPage* Pages[5];
	int screenshotIndex; 
	void setSessionImageServer(ServerProfile server);
	void setSessionFileServer(ServerProfile server);
	ServerProfile getSessionImageServer() const;
	ServerProfile getSessionFileServer() const;
	void setServersChanged(bool changed);
	bool serversChanged() const;
protected:
	ServerProfile sessionImageServer_, sessionFileServer_;
	bool serversChanged_;
public:
	bool ShowPage(int idPage,int prev=-1,int next=-1);
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
	// end of functions
	bool executeFunc(CString funcName);

	bool HandleDropFiledescriptors(IDataObject *pDataObj);
	bool HandleDropHDROP(IDataObject *pDataObj);
	bool HandleDropBitmap(IDataObject *pDataObj);

	public:
	CUpdateDlg *updateDlg;
	bool CanShowWindow();
	void UpdateAvailabilityChanged(bool Available);
	HACCEL hAccel;
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
	void OnScreenshotFinished(int Result);
	void OnScreenshotSaving(LPTSTR FileName, Gdiplus::Bitmap* Bm);
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