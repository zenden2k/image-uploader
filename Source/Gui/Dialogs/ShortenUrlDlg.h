#ifndef GUI_SHORTENURL_DIALOG_H
#define GUI_SHORTENURL_DIALOG_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       
#include "Core/FileDownloader.h"
#include "WizardDlg.h"
#include <Core/Upload/FileQueueUploader.h>
#include <Gui/Controls/PictureExWnd.h>
#include <Gui/Controls/CustomEditControl.h>

// CShortenUrlDlg
class CShortenUrlDlg:	public CDialogImpl <CShortenUrlDlg>,
                           public CDialogResize <CShortenUrlDlg>
{
	public:
		enum { IDD = IDD_SHORTENURL };
		CShortenUrlDlg(CWizardDlg *wizardDlg, CMyEngineList * engineList, UploadManager* uploadManager, const CString &initialBuffer);
		~CShortenUrlDlg();
		
	protected:	
		BEGIN_MSG_MAP(CShortenUrlDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			MSG_WM_DRAWCLIPBOARD(OnDrawClipboard)
			MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCbChain)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MSG_WM_CTLCOLORSTATIC(OnCtlColorMsgDlg)
			CHAIN_MSG_MAP(CDialogResize<CShortenUrlDlg>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CShortenUrlDlg)
			//DLGRESIZE_CONTROL(IDC_FILEINFOEDIT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_CENTER_X)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_INPUTEDIT, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_RESULTSEDIT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_RESULTSLABEL, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_ANIMATIONSTATIC, DLSZ_MOVE_X)
			
			//DLGRESIZE_CONTROL(IDC_DOWNLOADFILESPROGRESS, DLSZ_SIZE_X|DLSZ_MOVE_Y)
			//DLGRESIZE_CONTROL(IDC_IMAGEDOWNLOADERTIP, DLSZ_SIZE_X)
		END_DLGRESIZE_MAP()
		
		HWND PrevClipboardViewer;
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		void OnDrawClipboard();
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnCtlColorMsgDlg(HDC hdc, HWND hwndChild);
		void OnFileFinished(std::shared_ptr<UploadTask> task, bool ok);
		virtual bool OnQueueFinished(CFileQueueUploader* queueUploader);
		bool OnConfigureNetworkClient(CFileQueueUploader*, NetworkClient* nm);
		bool ParseBuffer(const CString& text);
		void OnClose();

		bool StartProcess();
		void ProcessFinished();

		CFileDownloader m_FileDownloader;
		CString m_InitialBuffer;
		CWizardDlg *m_WizardDlg;
		UploadManager* uploadManager_;
		CMyEngineList *engineList_;
		int serverId_;
		CPictureExWnd wndAnimation_;
		std::vector<CUploadEngineData*> servers_;
		CCustomEditControl outputEditControl_;
		CBrush backgroundBrush_;
		CBrush greenBrush_;
		std::vector<std_tr::shared_ptr<CUploadEngineData>> uploadEngineDataVector;
};



#endif // IMAGEDOWNLOADERDLG_H