/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

