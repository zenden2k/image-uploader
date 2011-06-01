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
#ifndef LOGOSETTINGS_H
#define LOGOSETTINGS_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/myimage.h"
#include "settingspage.h"
#include "3rdpart/ColorButton.h"
#include "Core/ImageConverter.h"
#include <atlcrack.h>

// CLogoSettings
#define IDC_NEWPROFILE 10001
#define IDC_SAVEPROFILE 10002
#define IDC_DELETEPROFILE 10003
struct LogoParams
{
	LOGFONT Font;
	LOGFONT ThumbFont;
	int LogoPosition;
	int LogoBlend;
	int TextPosition;
	TCHAR FileName[256];
	TCHAR FontName[256];
	TCHAR Text[256];
	COLORREF TextColor,StrokeColor,FrameColor,ThumbColor1,ThumbColor2/*TextBackground ,*/,ThumbTextColor;
	int ThumbAlpha;
	BOOL TextOverThumb;
};

#define COLOR_BUTTON(id, member) \
	if (uMsg == WM_COMMAND && BN_CLICKED == HIWORD(wParam) && id == LOWORD(wParam)) \
	{ \
		SetMsgHandled(TRUE); \
		CColorDialog ColorDialog(member);\
	if(ColorDialog.DoModal(m_hWnd)==IDOK)\
	{\
		member=ColorDialog.GetColor();\
	}\
			return TRUE; \
	}


class CLogoSettings : 
	public CDialogImpl<CLogoSettings>, public CSettingsPage	
{
public:
	CLogoSettings();
	virtual ~CLogoSettings();
	enum { IDD = IDD_LOGOSETTINGS };

    BEGIN_MSG_MAP(CLogoSettings)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER(IDC_LOGOBROWSE, BN_CLICKED, OnBnClickedLogobrowse)
			COMMAND_HANDLER(IDC_SELECTFONT, BN_CLICKED, OnBnClickedSelectfont)
			COMMAND_HANDLER(IDC_THUMBFONT, BN_CLICKED, OnBnClickedThumbfont)
			COMMAND_HANDLER_EX(IDC_YOURLOGO, BN_CLICKED, OnYourLogoCheckboxClicked)
			COMMAND_HANDLER_EX(IDC_YOURTEXT, BN_CLICKED, OnAddTextChecboxClicked)
			COMMAND_ID_HANDLER_EX(IDC_NEWPROFILE, OnNewProfile)
			COMMAND_ID_HANDLER_EX(IDC_SAVEPROFILE, OnSaveProfile)
			COMMAND_ID_HANDLER_EX(IDC_DELETEPROFILE, OnDeleteProfile)
			COMMAND_HANDLER_EX(IDC_PROFILECOMBO, CBN_SELCHANGE, OnProfileComboSelChange)
			NOTIFY_HANDLER( IDC_SELECTCOLOR, CPN_SELCHANGE, OnProfileEditedNotification)
			NOTIFY_HANDLER( IDC_STROKECOLOR, CPN_SELCHANGE, OnProfileEditedNotification)
			COMMAND_HANDLER(IDC_FORMATLIST, CBN_SELCHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_QUALITYEDIT, EN_CHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_IMAGEWIDTH, EN_CHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_IMAGEHEIGHT, EN_CHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_RESIZEMODECOMBO, CBN_SELCHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_SMARTCONVERTING, BN_CLICKED, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_LOGOPOSITION, CBN_SELCHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_EDITYOURTEXT, EN_CHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_TEXTPOSITION, CBN_SELCHANGE, OnProfileEditedCommand)
			COMMAND_HANDLER(IDC_LOGOEDIT, EN_CHANGE, OnProfileEditedCommand)
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
	CMyImage img;
   LRESULT OnProfileEditedCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProfileEditedNotification(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
	LRESULT OnBnClickedLogobrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSelectfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedThumbfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	CColorButton FrameColor;
	CColorButton Color1,Color2,ThumbTextColor,TextColor, StrokeColor;
	LogoParams *params;
	LOGFONT lf,ThumbFont;
   CString CurrentProfileOriginalName;
	bool Apply();
   void ShowParams(const ImageConvertingParams& params);
   void ShowParams(const CString profileName);
   bool SaveParams(ImageConvertingParams& params);
   std::map<CString, ImageConvertingParams> ñonvert_profiles_;
   LRESULT OnYourLogoCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   LRESULT OnAddTextChecboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   CToolBarCtrl m_ProfileEditToolbar;
   void UpdateProfileList();
   LRESULT OnSaveProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnNewProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnDeleteProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   CString CurrentProfileName;
   LRESULT OnProfileComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   void ProfileChanged();
   bool m_CatchChanges;
   bool m_ProfileChanged;
	public:
		void TranslateUI();
	
};

#endif // LOGOSETTINGS_H


