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

#pragma once

#include "resource.h"       // main symbols
#include "maindlg.h"
#include "logosettings.h"
#include "logindlg.h"
#include "atlctrlx.h"
#include "Gui/WizardCommon.h"
#include "Gui/Controls/IconButton.h"
#include <atlcrack.h>
#include "Gui/Controls/PercentEdit.h"
#include <Func/Settings.h>
#define IDC_SELECTFOLDER 4050
#define IDC_SERVERBUTTON 4000
#define IDC_IMAGETOOLBAR 4010
#define IDC_FILETOOLBAR 4011

#define IDC_NEWFOLDER 4012

#define IDC_OPENINBROWSER 4014

#define IDC_SERVERPARAMS 4016
#define IDC_OPENREGISTERURL 4018
#define IDC_LOGINTOOLBUTTON 4020

#define IDC_TOOLBARSEPARATOR1 4002
#define IDC_TOOLBARSEPARATOR2 4003

#define IDC_IMAGESERVER_FIRST_ID 14000
#define IDC_IMAGESERVER_LAST_ID 15000

#define IDC_FILESERVER_FIRST_ID 16000
#define IDC_FILESERVER_LAST_ID 17000

#define IDC_RESIZEPRESETMENU_FIRST_ID 18000
#define IDC_RESIZEPRESETMENU_LAST_ID 18100

class CMyEngineList;
class IconBitmapUtils;

class CUploadSettings : 
	public CDialogImpl<CUploadSettings>	, public CWizardPage
{
	public:
		CUploadSettings(CMyEngineList * EngineList);
		~CUploadSettings();
		enum { IDD = IDD_UPLOADSETTINGS };

    BEGIN_MSG_MAP(CUploadSettings)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(IDC_KEEPASIS, BN_CLICKED, OnBnClickedKeepasis)
		COMMAND_HANDLER(IDC_CREATETHUMBNAILS, BN_CLICKED, OnBnClickedCreatethumbnails)
		COMMAND_HANDLER(IDC_USETHUMBTEMPLATE, BN_CLICKED, OnBnClickedUseThumbTemplate)
		COMMAND_HANDLER(IDC_USESERVERTHUMBNAILS, BN_CLICKED, OnBnClickedUseServerThumbnails)
		COMMAND_HANDLER(IDC_LOGINTOOLBUTTON, BN_CLICKED, OnBnClickedLogin)
		COMMAND_HANDLER(IDC_LOGINTOOLBUTTON+1, BN_CLICKED, OnBnClickedLogin)
		COMMAND_HANDLER(IDC_SELECTFOLDER, BN_CLICKED, OnBnClickedSelectFolder)	
		COMMAND_HANDLER(IDC_NEWFOLDER, BN_CLICKED, OnNewFolder)	
		COMMAND_HANDLER(IDC_NEWFOLDER+1, BN_CLICKED, OnNewFolder)
		COMMAND_HANDLER(IDC_OPENINBROWSER, BN_CLICKED, OnOpenInBrowser)	
		COMMAND_HANDLER(IDC_OPENINBROWSER+1, BN_CLICKED, OnOpenInBrowser)
		COMMAND_HANDLER(IDC_OPENREGISTERURL, BN_CLICKED, OnOpenSignupPage)	
		COMMAND_HANDLER(IDC_OPENREGISTERURL+1, BN_CLICKED, OnOpenSignupPage)
		COMMAND_HANDLER(IDC_SERVERPARAMS, BN_CLICKED, OnServerParamsClicked)	
		COMMAND_HANDLER(IDC_SERVERPARAMS+1, BN_CLICKED, OnServerParamsClicked)
		NOTIFY_HANDLER(IDC_IMAGETOOLBAR, TBN_DROPDOWN, OnServerDropDown);
		NOTIFY_HANDLER(IDC_FILETOOLBAR, TBN_DROPDOWN, OnServerDropDown);
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
      COMMAND_HANDLER(IDC_FORMATLIST, CBN_SELCHANGE, OnProfileEditedCommand)
      COMMAND_HANDLER(IDC_QUALITYEDIT, EN_CHANGE, OnProfileEditedCommand)
      COMMAND_HANDLER(IDC_IMAGEWIDTH, EN_CHANGE, OnProfileEditedCommand)
		COMMAND_HANDLER(IDC_IMAGEHEIGHT, EN_CHANGE, OnProfileEditedCommand)
		MESSAGE_HANDLER( WM_CTLCOLOREDIT, OnCtlColorEdit )
		MESSAGE_HANDLER( WM_CTLCOLORBTN, OnCtlColorEdit )
		//MESSAGE_HANDLER( WM_CTLCOLORSTATIC, OnCtlColorEdit )
     
		COMMAND_RANGE_HANDLER(IDC_IMAGESERVER_FIRST_ID, IDC_IMAGESERVER_LAST_ID, OnImageServerSelect);
		COMMAND_RANGE_HANDLER(IDC_FILESERVER_FIRST_ID, IDC_FILESERVER_LAST_ID, OnFileServerSelect);
      COMMAND_RANGE_HANDLER(IDC_RESIZEPRESETMENU_FIRST_ID, IDC_RESIZEPRESETMENU_LAST_ID, OnResizePresetMenuItemClick);
      
      COMMAND_HANDLER_EX(IDC_RESIZEPRESETSBUTTON, BN_CLICKED, OnResizePresetButtonClicked)
      COMMAND_ID_HANDLER_EX(IDC_EDITPROFILE, OnEditProfileClicked)
      COMMAND_HANDLER_EX(IDC_PROFILECOMBO, CBN_SELCHANGE, OnProfileComboSelChange)
	   REFLECT_NOTIFICATIONS()
    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedKeepasis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSelectFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedCreatethumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedUseThumbTemplate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedUseServerThumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLogin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnImageServerSelect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileServerSelect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNewFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOpenInBrowser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnServerParamsClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnServerDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnOpenSignupPage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnQualityEditKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnResizePresetMenuItemClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   LRESULT OnProfileEditedCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCtlColorEdit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   
	int m_nImageServer, m_nFileServer;
	ServerProfile imageServer, fileServer;
	void ShowParams();
	CToolBarCtrl Toolbar;
	CToolBarCtrl FileServerSelectBar;
	bool OnNext();
	bool OnShow();
   virtual bool OnHide();
	void UpdateAllPlaceSelectors();
	void UpdatePlaceSelector(bool ImageServer);
	void UpdateToolbarIcons();
	CImageList m_PlaceSelectorImageList;
	int nImageIndex;
	int nFileIndex;
	void OnFolderButtonContextMenu(POINT pt, bool isImageServerToolbar);
	void OnServerButtonContextMenu(POINT pt, bool isImageServerToolbar);
protected:
	CMyEngineList * m_EngineList;
	IconBitmapUtils* iconBitmapUtils_;
	void TranslateUI();
   CIconButton m_ResizePresetIconButton;
   CIconButton m_ProfileEditButton;
   CToolBarCtrl m_ProfileEditToolbar;
	CPercentEdit m_ThumbSizeEdit;
    void UpdateProfileList();
public:
   LRESULT OnResizePresetButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   LRESULT OnEditProfileClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    std::map<CString, ImageConvertingParams>& ñonvert_profiles_;
     void ShowParams(const ImageConvertingParams& params);
   void ShowParams(const CString profileName);
   bool SaveParams(ImageConvertingParams& params);
    CString CurrentProfileName;
   void ProfileChanged();
   bool m_CatchChanges;
   bool m_ProfileChanged;
   void SaveCurrentProfile();
   CString CurrentProfileOriginalName;
   LRESULT OnProfileComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl);
};



