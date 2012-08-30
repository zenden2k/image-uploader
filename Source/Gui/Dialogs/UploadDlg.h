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
#include <Func/HistoryManager.h>
#include <Func/CommonTypes.h>

class CTempFilesDeleter;

class CUploadDlg : public CDialogImpl<CUploadDlg>,
                   public CThreadImpl<CUploadDlg>, 
						 public CFileQueueUploader::Callback,
						 public CWizardPage	
{
	public:
		CUploadDlg(CWizardDlg *dlg);
		~CUploadDlg();
		enum { IDD = IDD_UPLOADDLG };
		enum {  IDC_UPLOADPROCESSTAB = WM_USER +100, IDC_UPLOADRESULTSTAB = IDC_UPLOADPROCESSTAB+1};
		int TimerInc;
		bool IsStopTimer;
		bool Terminated;
		
		 BEGIN_MSG_MAP(CUploadDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			COMMAND_HANDLER(IDC_UPLOADPROCESSTAB, BN_CLICKED, OnUploadProcessButtonClick)
			COMMAND_HANDLER(IDC_UPLOADRESULTSTAB, BN_CLICKED, OnUploadResultsButtonClick)
			NOTIFY_HANDLER(IDC_UPLOADTABLE, LVN_ITEMCHANGED, OnLvnItemchangedUploadtable)
			COMMAND_HANDLER(IDC_MEDIAFILEINFO, BN_CLICKED, OnBnClickedMediaInfo)
			COMMAND_HANDLER(IDC_VIEWLOG, BN_CLICKED, OnBnClickedViewLog)
		 END_MSG_MAP()

		 // Handler prototypes:
		 //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		 //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		 //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnUploadProcessButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnUploadResultsButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
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
		//void UploadProgress(int CurPos, int Total,int FileProgress=0);
		int progressCurrent, progressTotal;
		CMyEngineList *m_EngineList;
		CString m_StatusText;
		bool OnUploaderNeedStop();
		void OnUploaderProgress(InfoProgress pi);
		void OnUploaderStatusChanged(StatusType status, int actionIndex, std::string text);
	   bool OnConfigureNetworkManager(NetworkManager* nm);
		bool OnUploadProgress(UploadProgress progress, UploadTask* task, NetworkManager* nm);

//upload stuff
		bool OnFileFinished(bool ok, CFileQueueUploader::FileListItem& result);
		bool OnQueueFinished(CFileQueueUploader*);
		void SetUploadProgress(int CurPos, int Total, int FileProgress);
		void createToolbar();
		void EnableMediaInfo(bool Enable);
		CToolBarCtrl Toolbar;
	protected:
		struct CUploadDlgProgressInfo
		{
			InfoProgress ip;
			CAutoCriticalSection CS;
			std::deque<DWORD> Bytes;
		};
		CUploadDlgProgressInfo PrInfo;
		#if  WINVER	>= 0x0601
				ITaskbarList3* ptl;
		#endif
		CFileQueueUploader queueUploader_;
		CTempFilesDeleter*  tempFileDeleter_;
		//CHistorySession session_;
		ZThread::Mutex *mutex_;
		int filesFinished_;
		int currentTab_;

		struct UploadItem {
			UploadItem() {
				fileUploaded = false;
				thumbUploaded = false;
				needThumb = false;
				tableIndex = -1;
			}
			std::string fileName;
			bool fileUploaded;
			bool thumbUploaded;
			bool needThumb;
			CFileQueueUploader::FileListItem fileResult;
			CFileQueueUploader::FileListItem thumbnailResult;
			int tableIndex;
			CUploadEngineData *uploadEngineData;
		};

		struct FileProcessingStruct {
			std::string fileName;
			CUploadEngineData *uploadEngineData;
			int tableRow;
		};

		struct UploadTaskData {
			UploadTaskData(UploadItem * item, bool isThumb) {
				uploadItem = item;
				this->isThumb = isThumb;
			}
			UploadItem *uploadItem;
			bool isThumb;
		};
		
		std::deque<UploadItem*> uploadItems_;
		std::deque<FileProcessingStruct> fileProcessingQueue_;
		CListViewCtrl uploadListView_;
		void showUploadResultsTab();
		void showUploadProgressTab();
		bool getNextUploadItem();
		bool addNewFilesToUploadQueue();
		
public:
	LRESULT OnLvnItemchangedUploadtable(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};



#endif // IU_GUI_DIALOGS_UPLOADDLG_H