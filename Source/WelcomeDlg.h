/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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
// CWelcomeDlg


class CWelcomeDlg ;
#include "resource.h"       // main symbols
#include "wizarddlg.h"
#include "Screenshotdlg.h"
#include "hyperlinkcontrol.h"
#include "settingsdlg.h"
#include "mediainfodlg.h"
#include "regionselect.h"
#include <atlcrack.h>

#define ID_VIEWHISTORY 36220
class CWelcomeDlg : 
	public CDialogImpl<CWelcomeDlg>, 
	public CWinDataExchange<CWelcomeDlg>, 
	public CWizardPage
{
public:
	 CWelcomeDlg();
	virtual ~CWelcomeDlg();
	enum { IDD = IDD_WELCOMEDLG };

    BEGIN_MSG_MAP(CWelcomeDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_CTLCOLORDLG(OnCtlColorMsgDlg)
		MSG_WM_CTLCOLORSTATIC(OnCtlColorMsgDlg)
		MSG_WM_DRAWCLIPBOARD(OnDrawClipboard)
		///	MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBkg)
		COMMAND_ID_HANDLER(IDC_CLIPBOARD,  OnClipboardClick)
		COMMAND_ID_HANDLER(IDC_ADDFOLDER,  OnAddFolderClick)
		COMMAND_ID_HANDLER(ID_VIEWHISTORY, OnViewHistoryClick)
		COMMAND_HANDLER(IDC_SCREENSHOT, BN_CLICKED, OnBnClickedScreenshot)
		COMMAND_HANDLER(IDC_SETTINGS, BN_CLICKED, OnBnClickedSettings)
		COMMAND_HANDLER(IDC_ADDIMAGES, BN_CLICKED, OnBnClickedAddimages)
		COMMAND_HANDLER(IDC_ADDVIDEO, BN_CLICKED, OnBnClickedAddvideo)
		COMMAND_HANDLER(IDC_REGIONPRINT, BN_CLICKED, OnBnClickedRegionPrint)
		COMMAND_HANDLER(IDC_MEDIAFILEINFO, BN_CLICKED, OnBnClickedMediaInfo)
		COMMAND_HANDLER(IDC_DOWNLOADIMAGES, BN_CLICKED, OnBnClickedDownloadImages)
		COMMAND_HANDLER(IDC_ADDFILES, BN_CLICKED, OnBnClickedAddFiles)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCbChain)
		REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

	BEGIN_DDX_MAP(WelcomeDlg)
		DDX_CONTROL(IDC_LISTBOX/*IDC_LINKSCONTROL*/, ListBox)
		DDX_CONTROL(IDC_LEFTBITMAP/*IDC_LINKSCONTROL*/, LeftImage)
		DDX_CONTROL(IDC_STATICLOGO/*IDC_LINKSCONTROL*/, LogoImage)
    END_DDX_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT OnBnClickedScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedAddvideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedAddimages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRegionPrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDownloadImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClipboardClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAddFolderClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewHistoryClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnDrawClipboard();
	LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	CHyperLinkControl ListBox;
	LRESULT OnCtlColorMsgDlg(HDC hdc, HWND hwndChild);
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	HWND PrevClipboardViewer;
	HBRUSH br; 
	bool OnShow();
	bool QuickRegionPrint;
	CMyImage LeftImage;CMyImage LogoImage;
};


