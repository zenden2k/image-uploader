/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
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
#ifndef THUMBSETTINGSPAGE_H
#define THUMBSETTINGSPAGE_H


#pragma once

#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
#include "SettingsPage.h"
#include "Func/Settings.h"
#include "3rdpart/ColorButton.h"
#include <atlcrack.h>
// CThumbSettingsPage

class CThumbSettingsPage : public CDialogImpl<CThumbSettingsPage>, 
                           public CSettingsPage	
{
public:
	CThumbSettingsPage();
	virtual ~CThumbSettingsPage();
	enum { IDD = IDD_THUMBSETTINGSPAGE };

    BEGIN_MSG_MAP(CThumbSettingsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_NEWTHUMBNAIL, BN_CLICKED, OnBnClickedNewThumbnail)
		COMMAND_HANDLER(IDC_THUMBSCOMBO, CBN_SELCHANGE, OnThumbComboChanged)
		COMMAND_HANDLER(IDC_EDITTHUMBNAILPRESET, BN_CLICKED, OnEditThumbnailPreset)
		COMMAND_HANDLER(IDC_THUMBTEXTCHECKBOX, BN_CLICKED,OnThumbTextCheckboxClick )
		COMMAND_HANDLER(IDC_THUMBTEXT, EN_CHANGE, OnThumbTextChange)
       COMMAND_HANDLER_EX(IDC_HEIGHTRADIO, BN_CLICKED, OnWidthEditChange) 
      COMMAND_HANDLER_EX(IDC_WIDTHRADIO, BN_CLICKED, OnWidthEditChange) 
      COMMAND_HANDLER_EX(IDC_HEIGHTEDIT, EN_CHANGE, OnWidthEditChange) 
      COMMAND_HANDLER_EX(IDC_WIDTHEDIT, EN_CHANGE, OnWidthEditChange)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	public:
		bool Apply();
	protected:
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnThumbTextChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		CMyImage img;
		std::string getSelectedThumbnailFileName();
		std::string getSelectedThumbnailName();
		LRESULT OnEditThumbnailPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnThumbTextCheckboxClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		void ThumbTextCheckboxChange();
		LRESULT OnThumbComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnBnClickedNewThumbnail(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		void showSelectedThumbnailPreview();
		LRESULT OnWidthEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl);
		ThumbCreatingParams params_;
		std::map<std::string, Thumbnail*> thumb_cache_;
      bool m_CatchFormChanges;
		CColorButton ThumbBackground;
   
};

#endif // THUMBSETTINGSPAGE_H

