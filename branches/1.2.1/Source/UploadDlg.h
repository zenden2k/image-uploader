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

#include "resource.h"       // main symbols
#include "uploader.h"
#include "maindlg.h"
#include "thread.h"
#include "LogoSettings.h"
#include "hyperlinkcontrol.h"
#include "SizeExceed.h"
#include "welcomedlg.h"




#include "ResultsPanel.h"
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
	TCHAR ProgressBuffer[256];
	InfoProgress PrInfo;

    BEGIN_MSG_MAP(CUploadDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		NOTIFY_HANDLER(IDC_RESULTSTAB, TCN_SELCHANGE, OnTabChanged)

	END_MSG_MAP()

	LRESULT OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	#if  WINVER	>= 0x0700
		ITaskbarList3* ptl;
	#endif
	 DWORD Run();
	 CMainDlg *MainDlg;
	 CResultsPanel ResultsPanel;
	int ThreadTerminated(void);
	int GenerateImages(LPTSTR szFileName, LPTSTR szBufferImage, LPTSTR szBufferThumb,int thumbwidth, ImageSettingsStruct &iss);
	CAtlArray<CUrlListItem> UrlList;
	CUploader *m_CurrentUploader;
	bool OnShow();
	bool OnNext();
	bool OnHide();
	void ShowProgress(bool Show=true);
	DWORD LastUpdate;
	void FileProgress(const CString Text, bool ShowPrefix=true);
	void GenThumb(LPCTSTR szImageFileName, Image *bm, int ThumbWidth,int newwidth,int newheight,LPTSTR szBufferThumb, int fileformat);
	CToolBarCtrl Toolbar;
	bool CancelByUser;
	void GenerateOutput();
	void UploadProgress(int CurPos, int Total,int FileProgress=0);
	int progressCurrent, progressTotal;
};


