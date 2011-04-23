/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
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

#include "../../resource.h"       // main symbols
#include "../../Core/Upload/Uploader.h"
#include "maindlg.h"
#include "../../3rdpart/thread.h"
#include "LogoSettings.h"
#include "../Controls/HyperLinkControl.h"
#include "SizeExceed.h"
#include "welcomedlg.h"
#include <Shobjidl.h>
#include "ResultsWindow.h"

struct CUploadDlgProgressInfo
{
	InfoProgress ip;
	CAutoCriticalSection CS;
	std::deque<DWORD> Bytes;
};
class CUploadDlg : 
	public CDialogImpl<CUploadDlg>,public CThreadImpl<CUploadDlg>, public CWizardPage	
{
public:
	CUploadDlg(CWizardDlg *dlg);
	~CUploadDlg();
	enum { IDD = IDD_UPLOADDLG };
	int TimerInc;
	bool IsStopTimer;
	bool Terminated;
	//TCHAR ProgressBuffer[256];
	CUploadDlgProgressInfo PrInfo;

    BEGIN_MSG_MAP(CUploadDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)

	END_MSG_MAP()

	
	
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	#if  WINVER	>= 0x0601
		ITaskbarList3* ptl;
	#endif
	 DWORD Run();
	 CMainDlg *MainDlg;
	 CResultsWindow *ResultsWindow;
	int ThreadTerminated(void);
//	int GenerateImages(LPCTSTR szFileName, LPTSTR szBufferImage, LPTSTR szBufferThumb,int thumbwidth, ImageConvertingParams &iss);
	CAtlArray<CUrlListItem> UrlList;
	CUploader *m_CurrentUploader;
	bool OnShow();
	bool OnNext();
	bool OnHide();
	//CToolBarCtrl Toolbar;
	void ShowProgress(bool Show=true);
	DWORD LastUpdate;
	void FileProgress(const CString Text, bool ShowPrefix=true);
	void GenThumb(LPCTSTR szImageFileName, Image *bm, int ThumbWidth,int newwidth,int newheight,LPTSTR szBufferThumb, int fileformat);
	bool CancelByUser;
	void GenerateOutput();
	void UploadProgress(int CurPos, int Total,int FileProgress=0);
	int progressCurrent, progressTotal;
	CMyEngineList *m_EngineList;
	CString m_StatusText;
	bool OnUploaderNeedStop();
	void OnUploaderProgress(InfoProgress pi);
	void OnUploaderStatusChanged(StatusType status, int actionIndex, std::string text);
	void OnUploaderConfigureNetworkClient(NetworkManager *nm);	
};


