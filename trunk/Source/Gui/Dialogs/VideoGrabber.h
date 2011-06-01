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
#include "Gui/Dialogs/maindlg.h"
#include "Gui/Dialogs/videograbberparams.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Gui/Controls/thumbsview.h"


#define WM_MYADDIMAGE (WM_USER+22)
// CVideoGrabber
class CVideoGrabber;
// Структура, предназначенная для передачи данных из дочернего потока главному
struct SENDPARAMS
{
	BYTE *pBuffer;
	CString szTitle;
	//LPTSTR szTitle;
	BITMAPINFO bi;
	long BufSize;
	CVideoGrabber *vg;
};

class CImgSavingThread: public CThreadImpl<CImgSavingThread>
{
public:
	SENDPARAMS m_sp;
	CVideoGrabber *vg;
	CAutoCriticalSection DataCriticalSection;
	CEvent StopEvent, SavingEvent, ImageProcessEvent;
	CImgSavingThread();
	DWORD Run();
	void Save(SENDPARAMS sp);
	void Stop();
	~CImgSavingThread();
	void Reset();
};

class CVideoGrabber : 
	public CWizardPage,  public CDialogImpl<CVideoGrabber>	,public CThreadImpl<CVideoGrabber>
{
public:
	CVideoGrabber();
	~CVideoGrabber();
	enum { IDD = IDD_VIDEOGRABBER };

    BEGIN_MSG_MAP(CVideoGrabber)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//MESSAGE_HANDLER(WM_MYADDIMAGE, OnAddImage)
		//MESSAGE_HANDLER(WM_MOUSEWHEEL/*0x020A*/, OnMouseWheel)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
        
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(IDC_GRAB, BN_CLICKED, OnBnClickedGrab)
		COMMAND_HANDLER(IDC_GRABBERPARAMS, BN_CLICKED, OnBnClickedGrabberparams)
		COMMAND_HANDLER(IDC_MULTIPLEFILES, BN_CLICKED, OnBnClickedMultiplefiles)
		COMMAND_HANDLER(IDC_SAVEASONE, BN_CLICKED, OnBnClickedMultiplefiles)
		COMMAND_HANDLER(IDC_SELECTVIDEO, BN_CLICKED, OnBnClickedButton1)
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
	LRESULT OnBnClickedGrab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	CImgSavingThread SavingThread;
	CString ErrorStr;
	bool SetFileName(LPCTSTR FileName);
	TCHAR m_szFileName[MAX_PATH];
	int GrabBitmaps(TCHAR * szFile );
	DWORD Run();
	bool CanceledByUser;
	CMainDlg* MainDlg;
	bool GrabInfo(LPCTSTR String);
	bool Terminated,IsStopTimer;
	int NumOfFrames;
	int TimerInc;
	int ThreadTerminated();

	LRESULT OnBnClickedGrabberparams(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnBnClickedMultiplefiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	bool OnAddImage(SENDPARAMS *sp);
	void SavingMethodChanged(void);

	int GenPicture(CString &outFileName);
	TCHAR szBufferFileName[256];

	LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	bool OnShow();
	CThumbsView ThumbsView;
	void CheckEnableNext();
	bool OnNext();
	LRESULT OnBnClickedFileinfobutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};



#endif // VIDEOGRABBER_H