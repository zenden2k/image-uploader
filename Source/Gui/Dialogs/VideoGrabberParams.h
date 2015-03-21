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
#ifndef VIDEOGRABBERPARAMS_H
#define VIDEOGRABBERPARAMS_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "settingspage.h"
#include <atlcrack.h>
#include "3rdpart/ColorButton.h"
// CVideoGrabberParams

class CVideoGrabberParams : 
	public CDialogImpl<CVideoGrabberParams>, public CSettingsPage	
{
public:
	CVideoGrabberParams();
	~CVideoGrabberParams();
	enum { IDD = IDD_VIDEOGRABBERPARARAMS};

    BEGIN_MSG_MAP(CVideoGrabberParams)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_MEDIAINFOFONT, OnMediaInfoFontClicked)
		COMMAND_HANDLER(IDC_MEDIAINFOONIMAGE,BN_CLICKED, OnShowMediaInfoTextBnClicked)
		COMMAND_HANDLER(IDC_VIDEOSNAPSHOTSFOLDERBUTTON, BN_CLICKED, OnVideoSnapshotsFolderButtonClicked);
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//CSavingOptions *so;
	bool Apply();
	LOGFONT m_Font;
	CColorButton Color1;
	LRESULT OnMediaInfoFontClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnShowMediaInfoTextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnVideoSnapshotsFolderButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};



#endif // VIDEOGRABBERPARAMS_H