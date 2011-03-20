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
struct LogoParams;
#include "../../resource.h"       // main symbols
#include "../../myimage.h"
#include "../../colorpicker.h"
#include "../../settingspage.h"
#include "../../Settings.h"
// CThumbSettingsPage



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


class CThumbSettingsPage : 
	public CDialogImpl<CThumbSettingsPage>, public CSettingsPage	
{
public:
	CThumbSettingsPage();
	virtual ~CThumbSettingsPage();
	enum { IDD = IDD_THUMBSETTINGSPAGE };

    BEGIN_MSG_MAP(CThumbSettingsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
      COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(IDC_LOGOBROWSE, BN_CLICKED, OnBnClickedLogobrowse)
		COMMAND_HANDLER(IDC_SELECTFONT, BN_CLICKED, OnBnClickedSelectfont)
		COMMAND_HANDLER(IDC_THUMBFONT, BN_CLICKED, OnBnClickedThumbfont)
		COMMAND_HANDLER(IDC_NEWTHUMBNAIL, BN_CLICKED, OnBnClickedNewThumbnail)
		COMMAND_HANDLER(IDC_THUMBSCOMBO, CBN_SELCHANGE, OnThumbComboChanged)
		COMMAND_HANDLER(IDC_EDITTHUMBNAILPRESET, BN_CLICKED, OnEditThumbnailPreset)
		COMMAND_HANDLER(IDC_THUMBTEXTCHECKBOX, BN_CLICKED,OnThumbTextCheckboxClick )
		COMMAND_HANDLER(IDC_THUMBTEXT, EN_CHANGE, OnThumbTextChange);

/*		COLOR_BUTTON(IDC_SELECTCOLOR,params->TextColor);
		COLOR_BUTTON(IDC_STROKECOLOR,params->StrokeColor);
		COLOR_BUTTON(IDC_FRAMECOLOR,params->FrameColor);
		COLOR_BUTTON(IDC_THUMBTEXTCOLOR,params->ThumbTextColor);
		COLOR_BUTTON(IDC_COLOR1,params->ThumbColor1);
		COLOR_BUTTON(IDC_COLOR2,params->ThumbColor2);*/
		REFLECT_NOTIFICATIONS()
		END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnThumbTextChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	CMyImage img;
	LRESULT OnBnClickedLogobrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSelectfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedThumbfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LogoParams *params;
	LOGFONT lf,ThumbFont;
	bool Apply();
	std::string getSelectedThumbnailFileName();
	LRESULT OnEditThumbnailPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnThumbTextCheckboxClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void ThumbTextCheckboxChange();
	LRESULT OnThumbComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClickedNewThumbnail(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void showSelectedThumbnailPreview();
	ThumbCreatingParams params_;
	std::map<std::string, Thumbnail*> thumb_cache_;
};


