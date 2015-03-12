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

#ifndef LANGSELECT_H
#define LANGSELECT_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
// CLangSelect


class CLangSelect : public CDialogImpl<CLangSelect>	
{
	public:
		CLangSelect();
		~CLangSelect();
	
		enum { IDD = IDD_LANGSELECT };
	protected:
		BEGIN_MSG_MAP(CLangSelect)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
 		int GetNextLngFile(LPTSTR szBuffer, int nLength);
		TCHAR Language[128];

	private:
		WIN32_FIND_DATA wfd;
		HANDLE findfile;
		CMyImage LogoImage;

		void SelectLang(LPCTSTR Lang);
};


#endif // LANGSELECT_H