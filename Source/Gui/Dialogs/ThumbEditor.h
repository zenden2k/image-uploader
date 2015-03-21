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

#ifndef THUMBEDITOR_H
#define THUMBEDITOR_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include <atlframe.h>
#include "3rdpart/ColorButton.h"

class Thumbnail;
// CThumbEditor

class CThumbEditor : 
	public CDialogImpl<CThumbEditor>
{
public:
	CThumbEditor(Thumbnail *thumb);
	~CThumbEditor();
	enum { IDD = IDD_THUMBEDITOR };

    BEGIN_MSG_MAP(CThumbEditor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		  COMMAND_HANDLER(IDC_ADDFILESIZE, BN_CLICKED, OnShowTextCheckboxClicked)
		  COMMAND_HANDLER(IDC_DRAWFRAME, BN_CLICKED, OnDrawFrameCheckboxClicked)
		  COMMAND_HANDLER(IDC_THUMBFONT, BN_CLICKED, OnFontSelect);
			REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

	protected:
		 // Handler prototypes:
		 //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		 //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		 //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnShowTextCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnDrawFrameCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnFontSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		
		void LoadParams();
		void SaveParams();
		CColorButton FrameColor;
		LOGFONT ThumbFont;
		Thumbnail * thumb_;
		CColorButton Color1,Color2,ThumbTextColor,TextColor, StrokeColor;
		void ShowTextCheckboxChanged();
		void DrawFrameCheckboxChanged();
};



#endif // THUMBEDITOR_H