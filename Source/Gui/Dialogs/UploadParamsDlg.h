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
#ifndef LOGINDLG_H
#define LOGINDLG_H

#pragma once
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/Upload/UploadEngine.h"
#include "3rdpart/ColorButton.h"
#include "Func/Settings.h"

// CUploadParamsDlg

class CUploadParamsDlg : public CDialogImpl<CUploadParamsDlg>	
{
	public:
		int ServerId;
		CUploadParamsDlg(ImageUploadParams params);
		~CUploadParamsDlg();
		enum { IDD = IDD_UPLOADPARAMSDLG };
		ImageUploadParams imageUploadParams();
	protected:
		BEGIN_MSG_MAP(CUploadParamsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER(IDC_CREATETHUMBNAILS, BN_CLICKED, OnClickedCreateThumbnailsCheckbox)
			COMMAND_HANDLER(IDC_DEFAULTSETTINGSCHECKBOX, BN_CLICKED, OnClickedDefaultSettingsCheckbox)
			COMMAND_HANDLER(IDC_DEFAULTTHUMBSETTINGSCHECKBOX, BN_CLICKED, OnClickedDefaultThumbSettingsCheckbox)
			COMMAND_HANDLER(IDC_THUMBTEXTCHECKBOX, BN_CLICKED, OnClickedThumbTextCheckbox)
			COMMAND_HANDLER(IDC_USESERVERTHUMBNAILS, BN_CLICKED, OnClickedUseServerThumbnailsCheckbox)

			
			
			COMMAND_HANDLER(IDC_PROCESSIMAGESCHECKBOX, BN_CLICKED, OnClickedProcessImagesCheckbox)
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCreateThumbnailsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedProcessImagesCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedDefaultSettingsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedDefaultThumbSettingsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedThumbTextCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedUseServerThumbnailsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		
		
		void createThumbnailsCheckboxChanged();
		void processImagesChanged();
		void defaultSettingsCheckboxChanged();
		void defaultThumbSettingsCheckboxChanged();
		void thumbTextCheckboxChanged();
		void useServerThumbnailsChanged();
		CUploadEngineData *m_UploadEngine;
		CColorButton ThumbBackground_;
		ImageUploadParams params_;
};

#endif // LOGINDLG_H


