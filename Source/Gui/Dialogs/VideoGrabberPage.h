/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

 */
#ifndef IU_GUI_DIALOGS_VIDEOGRABBERPAGE_H
#define IU_GUI_DIALOGS_VIDEOGRABBERPAGE_H

#include <random>
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/WizardCommon.h"
#include "Gui/Controls/ThumbsView.h"

class AbstractImage;

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
		enum { IDD = IDD_VIDEOGRABBER };

	protected:
		BEGIN_MSG_MAP(CVideoGrabberPage)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
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
            DDX_CONTROL_HANDLE(IDC_UPDOWN, frameCountUpDownCtrl_)
        END_DDX_MAP()

		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
		LRESULT OnBnClickedGrab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedGrabberparams(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedMultiplefiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedFileinfobutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnBnClickedBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnOpenFolder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
		int GrabBitmaps(const CString& szFile );
		bool SetGrabbingStatusText(LPCTSTR String);
		int ThreadTerminated();
		bool OnAddImage(Gdiplus::Bitmap *bm, CString title);
		void SavingMethodChanged(void);
		int GenPicture(CString& outFileName);
		static CString GenerateFileNameFromTemplate(const CString& templateStr, int index, const CPoint& size, const CString& originalName);
		CThumbsView ThumbsView;
		void CheckEnableNext();
		bool OnNext() override; // Reimplemented function of CWizardPage
		bool OnShow() override; // Reimplemented function of CWizardPage
		void OnFrameGrabbed(const std::string&, int64_t, std::shared_ptr<AbstractImage>);
		void OnFrameGrabbingFinished(bool success);

		CHyperLink openInFolderLink_;
		std::unique_ptr<VideoGrabber> videoGrabber_ ;

//		CImgSavingThread SavingThread;
		CString ErrorStr;
		CString snapshotsFolder;
		bool Terminated;
		int originalGrabInfoLabelWidth_;
		int grabbedFramesCount;
		int NumOfFrames;
		bool SetFileName(LPCTSTR FileName);
		CString fileName_;
		bool CanceledByUser;
		CMainDlg* MainDlg;
        CEdit fileEdit_;
        CComboBox videoEngineCombo_;
		UploadEngineManager * uploadEngineManager_;
        CToolTipCtrl toolTipCtrl_;
		CUpDownCtrl frameCountUpDownCtrl_;
};

#endif // VIDEOGRABBER_H
