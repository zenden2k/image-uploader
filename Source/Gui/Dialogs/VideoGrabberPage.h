/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>

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
#ifndef IU_GUI_DIALOGS_VIDEOGRABBERPAGE_H
#define IU_GUI_DIALOGS_VIDEOGRABBERPAGE_H

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/WizardCommon.h"
#include "Gui/Controls/ThumbsView.h"
#include "Gui/HwndScopedWrapper.h"

class AbstractImage;
#define WM_MYADDIMAGE (WM_USER + 22)

class VideoGrabber;
class CVideoGrabberPage;
class UploadEngineManager;
class CMainDlg;

struct SENDPARAMS
{
	BYTE* pBuffer;
	CString szTitle;
	BITMAPINFO bi;
	long BufSize;
	CVideoGrabberPage* vg;
};

class CVideoGrabberPage : public CWizardPage, public CDialogImpl<CVideoGrabberPage>, public CWinDataExchange <CVideoGrabberPage>
{ 
	public:
		CVideoGrabberPage(UploadEngineManager * uploadEngineManager);
		~CVideoGrabberPage();
		enum { IDD = IDD_VIDEOGRABBER };

	protected:
		BEGIN_MSG_MAP(CVideoGrabberPage)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER(IDC_GRAB, BN_CLICKED, OnBnClickedGrab)
			COMMAND_HANDLER(IDC_GRABBERPARAMS, BN_CLICKED, OnBnClickedGrabberparams)
			COMMAND_HANDLER(IDC_MULTIPLEFILES, BN_CLICKED, OnBnClickedMultiplefiles)
			COMMAND_HANDLER(IDC_SAVEASONE, BN_CLICKED, OnBnClickedMultiplefiles)
			COMMAND_HANDLER(IDC_SELECTVIDEO, BN_CLICKED, OnBnClickedBrowseButton)
			COMMAND_ID_HANDLER(IDC_OPENFOLDER, OnOpenFolder)
			NOTIFY_HANDLER(IDC_THUMBLIST, LVN_DELETEITEM, OnLvnItemDelete)
			COMMAND_HANDLER(IDC_FILEINFOBUTTON, BN_CLICKED, OnBnClickedFileinfobutton)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()
        
        BEGIN_DDX_MAP(CVideoGrabberPage)
            DDX_CONTROL_HANDLE(IDC_FILEEDIT, fileEdit_)
            DDX_CONTROL_HANDLE(IDC_VIDEOENGINECOMBO, videoEngineCombo_)
        END_DDX_MAP()

		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
		LRESULT OnBnClickedGrab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedGrabberparams(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedMultiplefiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedFileinfobutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnOpenFolder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
		int GrabBitmaps(const CString& szFile );
		DWORD Run();
		bool SetGrabbingStatusText(LPCTSTR String);
		int ThreadTerminated();
		bool OnAddImage(Gdiplus::Bitmap *bm, CString title);
		void SavingMethodChanged(void);
		int GenPicture(CString& outFileName);
		CString GenerateFileNameFromTemplate(const CString& templateStr, int index, const CPoint& size, const CString& originalName);
		CThumbsView ThumbsView;
		void CheckEnableNext();
		bool OnNext() override; // Reimplemented function of CWizardPage
		bool OnShow() override; // Reimplemented function of CWizardPage
		void OnFrameGrabbed(const std::string&, int64_t, AbstractImage*);
		void OnFrameGrabbingFinished();

		CHyperLink openInFolderLink_;
		std::unique_ptr<VideoGrabber> videoGrabber_ ;

//		CImgSavingThread SavingThread;
		CString ErrorStr;
		CString snapshotsFolder;
		bool Terminated, IsStopTimer;
		int originalGrabInfoLabelWidth_;
		int grabbedFramesCount;
		int NumOfFrames;
		int TimerCounter;
		bool SetFileName(LPCTSTR FileName);
		CString m_szFileName;
		bool CanceledByUser;
		CMainDlg* MainDlg;
        CEdit fileEdit_;
        CComboBox videoEngineCombo_;
		UploadEngineManager * uploadEngineManager_;
        HwndScopedWrapper engineComboToolTip_, deinterlacingCheckboxTooltip_;
};

#endif // VIDEOGRABBER_H
