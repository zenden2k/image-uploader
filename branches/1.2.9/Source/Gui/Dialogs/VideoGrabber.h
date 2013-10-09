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
#ifndef VIDEOGRABBER_H
#define VIDEOGRABBER_H

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "3rdpart/thread.h"
#include "Gui/WizardCommon.h"
#include "Gui/Dialogs/MainDlg.h"
#include "Gui/Dialogs/videograbberparams.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Gui/Controls/ThumbsView.h"

#define __AFX_H__ // little hack for avoiding __POSITION type redefinition
#include <objbase.h>
#include <streams.h>
#undef __AFX_H__
#include <qedit.h>

#define WM_MYADDIMAGE (WM_USER + 22)

class CVideoGrabber;

// Структура, предназначенная для передачи данных из дочернего потока главному
struct SENDPARAMS
{
	BYTE* pBuffer;
	CString szTitle;
	BITMAPINFO bi;
	long BufSize;
	CVideoGrabber* vg;
};

class CImgSavingThread : public CThreadImpl<CImgSavingThread>
{
	public:
		SENDPARAMS m_sp;
		CVideoGrabber* vg;
		CAutoCriticalSection DataCriticalSection;
		CEvent StopEvent, SavingEvent, ImageProcessEvent;
		CImgSavingThread();
		DWORD Run();
		void Save(SENDPARAMS sp);
		void Stop();
		~CImgSavingThread();
		void Reset();
};

class CSampleGrabberCB : public ISampleGrabberCB
{
public:
	SENDPARAMS sp;
	CImgSavingThread* SavingThread;
	CVideoGrabber* vg;
	// Эти параметры устанавливаются главным потоком
	long Width;
	long Height;
	bool Grab; // для избавления от дубликатов
	CEvent ImageProcessEvent;
	HANDLE BufferEvent;
	LONGLONG prev, step; // не используется

	// Fake out any COM ref counting
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// Fake out any COM QI'ing
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP SampleCB( double SampleTime, IMediaSample* pSample );


	STDMETHODIMP BufferCB( double SampleTime, BYTE* pBuffer, long BufferSize );
	
};

class CVideoGrabber :
	public CWizardPage,  public CDialogImpl<CVideoGrabber>, public CThreadImpl<CVideoGrabber>
{
	public:
		CVideoGrabber();
		~CVideoGrabber();
		enum { IDD = IDD_VIDEOGRABBER };

	protected:
		BEGIN_MSG_MAP(CVideoGrabber)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER(IDC_GRAB, BN_CLICKED, OnBnClickedGrab)
			COMMAND_HANDLER(IDC_GRABBERPARAMS, BN_CLICKED, OnBnClickedGrabberparams)
			COMMAND_HANDLER(IDC_MULTIPLEFILES, BN_CLICKED, OnBnClickedMultiplefiles)
			COMMAND_HANDLER(IDC_SAVEASONE, BN_CLICKED, OnBnClickedMultiplefiles)
			COMMAND_HANDLER(IDC_SELECTVIDEO, BN_CLICKED, OnBnClickedButton1)
			COMMAND_ID_HANDLER(IDC_OPENFOLDER, OnOpenFolder)
			NOTIFY_HANDLER(IDC_THUMBLIST, LVN_DELETEITEM, OnLvnItemDelete)
			COMMAND_HANDLER(IDC_FILEINFOBUTTON, BN_CLICKED, OnBnClickedFileinfobutton)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnLvnKeydownThumblist(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
		LRESULT OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
		LRESULT OnBnClickedGrab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedGrabberparams(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedMultiplefiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedFileinfobutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnOpenFolder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
		int GrabBitmaps(TCHAR* szFile );
		DWORD Run();
		bool GrabInfo(LPCTSTR String);
		int ThreadTerminated();
		bool OnAddImage(SENDPARAMS* sp);
		void SavingMethodChanged(void);
		int GenPicture(CString& outFileName);
		CString GenerateFileNameFromTemplate(const CString& templateStr, int index, const CPoint size, const CString& originalName);
		CThumbsView ThumbsView;
		void CheckEnableNext();
		bool OnNext(); // Reimplemented function of CWizardPage
		bool OnShow(); // Reimplemented function of CWizardPage

		CHyperLink openInFolderLink_;

		TCHAR szBufferFileName[256];
		CImgSavingThread SavingThread;
		CString ErrorStr;
		CString snapshotsFolder;
		bool Terminated, IsStopTimer;
		int originalGrabInfoLabelWidth_;
		int grabbedFramesCount;
		int NumOfFrames;
		int TimerInc;
		bool SetFileName(LPCTSTR FileName);
		TCHAR m_szFileName[MAX_PATH];
		bool CanceledByUser;
		CMainDlg* MainDlg;
};

#endif // VIDEOGRABBER_H
