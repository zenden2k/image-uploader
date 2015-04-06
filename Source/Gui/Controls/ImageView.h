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

#pragma once

#include "resource.h"       // main symbols
#include "myimage.h"
#include <atlcrack.h>
// CImageView

class CImageView : 
	public CDialogImpl<CImageView>	
{
	public:
		CImageView();
		~CImageView();
		enum { IDD = IDD_IMAGEVIEW };

		 BEGIN_MSG_MAP(CImageView)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MSG_WM_KILLFOCUS(OnKillFocus)
			MSG_WM_ACTIVATE(OnActivate)
			MSG_WM_KEYDOWN(OnKeyDown)
			  COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			  COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedOK)
		 END_MSG_MAP()
		 // Handler prototypes:
		 //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		 //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		 //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		CMyImage Img;
		LRESULT OnKillFocus(HWND hwndNewFocus);
		LRESULT OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags);
	public:
		bool ViewImage(LPCTSTR FileName, HWND Parent=NULL);
		LRESULT OnActivate(UINT state, BOOL fMinimized, HWND hwndActDeact);
};


