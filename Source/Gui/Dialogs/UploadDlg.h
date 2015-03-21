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
#ifndef IU_GUI_DIALOGS_UPLOADDLG_H
#define IU_GUI_DIALOGS_UPLOADDLG_H


#pragma once

#include <deque>
#include <Shobjidl.h>
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "3rdpart/thread.h"

#include "Core/Upload/Uploader.h"
#include "Gui/Controls/HyperLinkControl.h"
#include "Gui/Dialogs/maindlg.h"
#include "Gui/Dialogs/LogoSettings.h"
#include "Gui/Dialogs/SizeExceed.h"
#include "Gui/Dialogs/ResultsWindow.h"
#include <Core/Upload/FileQueueUploader.h>
#include "Func/Settings.h"

class CUploadDlg : public CDialogImpl<CUploadDlg>,
                   public CThreadImpl<CUploadDlg>, 
						 public CWizardPage,
						 public CFileQueueUploader::Callback
{
	public:
		CUploadDlg(CWizardDlg *dlg);
		~CUploadDlg();
		enum { IDD = IDD_UPLOADDLG };
		int TimerInc;
		bool IsStopTimer;
		bool Terminated;
		
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
		 DWORD Run();
		 CMainDlg *MainDlg;
		 CResultsWindow *ResultsWindow;
		int ThreadTerminated(void);
		CAtlArray<CUrlListItem> UrlList;
		CUploader *m_CurrentUploader;
		bool OnShow();
		bool OnNext();
		bool OnHide();
		void ShowProgress(bool Show=true);
		DWORD LastUpdate;
		void FileProgress(const CString& Text, bool ShowPrefix=true);
		void GenThumb(LPCTSTR szImageFileName, Gdiplus::Image *bm, int ThumbWidth,int newwidth,int newheight,LPTSTR szBufferThumb, int fileformat);
		bool CancelByUser;
		void GenerateOutput();
		void UploadProgress(int CurPos, int Total,int FileProgress=0);
		int progressCurrent, progressTotal;
		CMyEngineList *m_EngineList;
		CString m_StatusText;
		CFileQueueUploader* queueUploader_;
		bool OnUploaderNeedStop();
		void OnUploaderProgress(CUploader* uploader, InfoProgress pi);
		void OnUploaderStatusChanged(StatusType status, int actionIndex, std::string text);
		void OnUploaderConfigureNetworkClient(NetworkManager *nm);	
		void onShortenUrlChanged(bool shortenUrl);
		void AddShortenUrlTask(CUrlListItem* item);
		void AddShortenUrlTask(CUrlListItem* item, CString linkType);
		virtual bool OnFileFinished(bool ok, CFileQueueUploader::FileListItem& result);
		virtual bool OnConfigureNetworkManager(CFileQueueUploader*, NetworkManager* nm);
	protected:
		struct CUploadDlgProgressInfo
		{
			InfoProgress ip;
			CAutoCriticalSection CS;
			std::deque<DWORD> Bytes;
		};
		struct ShortenUrlUserData {
			CUrlListItem* item;
			CString linkType;
		};
		CUploadDlgProgressInfo PrInfo;
		bool alreadyShortened_;
		ServerProfile sessionImageServer_, sessionFileServer_;
		
		#if  WINVER	>= 0x0601
				ITaskbarList3* ptl;
		#endif
};



#endif // IU_GUI_DIALOGS_UPLOADDLG_H