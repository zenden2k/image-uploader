/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

class CWizardDlg;

class CWizardPage;
class CFolderAdd;
#include "screenshotdlg.h"
#include "welcomedlg.h"
#include "maindlg.h"
#include "videograbber.h"
#include "uploadsettings.h"
#include "uploaddlg.h"
#include "aboutdlg.h"
#include "statusdlg.h"

#include "resource.h"       // main symbols
#include <atlcrack.h>
#define ID_PASTE 9888
#define ID_HOTKEY_BASE 10000
#define WM_MY_ADDIMAGE WM_USER + 222
#define WM_MY_SHOWPAGE WM_USER + 223
// CWizardDlg

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
class CMyFolderDialog: public CFolderDialogImpl<CFolderDialog>
{
	
	virtual void OnInitialized();
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
{
public:
	CWizardDlg();
	~CWizardDlg();
	CStringList m_Paths;
	enum { IDD = IDD_WIZARDDLG };
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CWizardDlg)
	END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CWizardDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
		MESSAGE_HANDLER(WM_MY_ADDIMAGE, OnAddImages)
		MESSAGE_HANDLER(WM_MY_SHOWPAGE, OnWmShowPage)
		MESSAGE_HANDLER(WM_ENABLE,OnEnable)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(ID_PASTE, 1, OnPaste)
		COMMAND_RANGE_HANDLER(ID_HOTKEY_BASE, ID_HOTKEY_BASE +100, OnLocalHotkey);
		COMMAND_HANDLER_EX(IDC_PREV, BN_CLICKED, OnPrevBnClicked)
		COMMAND_HANDLER_EX(IDC_NEXT, BN_CLICKED, OnNextBnClicked)
		COMMAND_HANDLER(IDC_ABOUT, BN_CLICKED, OnBnClickedAbout)

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
	
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnLocalHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void CloseDialog(int nVal);
	bool DragndropEnabled;
	int CurPage;
	int PrevPage,NextPage;
bool CreatePage(int PageID);
	CWizardPage* Pages[5];

public:
	bool ShowPage(int idPage,int prev=-1,int next=-1);
	bool AddImage(LPCTSTR FileName, bool Show=true);
	LRESULT OnPrevBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
LRESULT OnNextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
HBITMAP GenHeadBitmap(int PageID);
	void PasteBitmap(HBITMAP);
public:
	LRESULT OnBnClickedAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void Exit();
	LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//CSavingOptions SavingOptions;
	bool LoadUploadEngines(CString &Error);
	bool ParseCmdLine();

	CHotkeyList m_hotkeys;
	CFolderAdd FolderAdd;
	long m_lRef;
	bool QuickUploadMarker;
	TCHAR LastVideoFile[MAX_PATH];

	// functions
	bool funcAddImages();
	bool funcImportVideo();
	bool funcScreenshotDlg();
	bool funcRegionScreenshot();
	bool funcFullScreenshot();
	bool funcWindowScreenshot();
	bool funcAddFolder();
	bool funcPaste();
	bool funcSettings();
	bool funcMediaInfo();
	// end of functions
	bool executeFunc(CString funcName);

	bool HandleDropFiledescriptors(IDataObject *pDataObj);
	bool HandleDropHDROP(IDataObject *pDataObj);
	bool HandleDropBitmap(IDataObject *pDataObj);

	public:
			CScreenshotDlg _screenShotdlg;
	HACCEL hAccel;
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
	void OnScreenshotSaving(LPTSTR FileName, Bitmap* Bm);
	void CloseWizard();
	bool RegisterLocalHotkeys();
	bool UnRegisterLocalHotkeys();
	bool m_bShowWindow;
};


