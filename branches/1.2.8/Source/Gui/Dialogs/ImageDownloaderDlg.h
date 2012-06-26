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

// ImageDownloaderDlg.h : Declaration of the CImageDownloaderDlg
// 
// This dialog window shows technical information 
// about  video/audio file that user had selected
#ifndef IMAGEDOWNLOADERDLG_H
#define IMAGEDOWNLOADERDLG_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       
#include "Core/FileDownloader.h"
#include "WizardDlg.h"

// CImageDownloaderDlg
class CImageDownloaderDlg:	public CDialogImpl <CImageDownloaderDlg>,
                           public CDialogResize <CImageDownloaderDlg>
{
	public:
		enum { IDD = IDD_IMAGEDOWNLOADER };
		CImageDownloaderDlg(CWizardDlg *wizardDlg,const CString &initialBuffer);
		~CImageDownloaderDlg();

	protected:	
		BEGIN_MSG_MAP(CImageDownloaderDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			MSG_WM_DRAWCLIPBOARD(OnDrawClipboard)
			MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCbChain)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			CHAIN_MSG_MAP(CDialogResize<CImageDownloaderDlg>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CImageDownloaderDlg)
			DLGRESIZE_CONTROL(IDC_FILEINFOEDIT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_DOWNLOADFILESPROGRESS, DLSZ_SIZE_X|DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_IMAGEDOWNLOADERTIP, DLSZ_SIZE_X)
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
		bool BeginDownloading();
		static bool LinksAvailableInText(const CString &text);
		void ParseBuffer(const CString& text, bool OnlyImages);
		void OnQueueFinished();
		bool OnFileFinished(bool ok, CFileDownloader::DownloadFileListItem it);
		void OnConfigureNetworkManager(NetworkManager* nm);

		CString m_FileName;
		CFileDownloader m_FileDownloader;
		CWizardDlg * m_WizardDlg;
		int m_nFilesCount;
		int m_nFileDownloaded;
		CString m_InitialBuffer;
};



#endif // IMAGEDOWNLOADERDLG_H