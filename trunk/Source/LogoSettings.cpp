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

#include "stdafx.h"

#include "LogoSettings.h"
#include <uxtheme.h>
#include "LogWindow.h"
#include "LangClass.h"
#include "Settings.h"

#pragma comment( lib, "uxtheme.lib" )
// CLogoSettings
CLogoSettings::CLogoSettings()
{
	ZeroMemory(&lf, sizeof(lf));
}

CLogoSettings::~CLogoSettings()
{
		
}

LRESULT CLogoSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TabBackgroundFix(m_hWnd);
	// Translating controls
	TRC(IDCANCEL, "������");
	TRC(IDC_FILENAMELABEL, "��� �����:");
	TRC(IDC_LOGOPOSITIONLABEL, "������� ��������:");
	TRC(IDC_LOGOGROUP, "������� ����");
	TRC(IDC_TEXTONIMAGEGROUP, "����� �� ��������");
	TRC(IDC_ENTERYOURTEXTLABEL, "������� �����:");
	TRC(IDC_TEXTCOLORLABEL, "���� ������:");
	TRC(IDC_TEXTSTROKECOLOR, "���� �������:");
	TRC(IDC_SELECTFONT, "�����...");
	TRC(IDC_TEXTPOSITIONLABEL, "������� ��������:");
	TRC(IDC_THUMBPARAMS, "��������� �������");
	TRC(IDC_FRAMECOLORLABEL, "���� �����");
	TRC(IDC_THUMBTEXTCOLORLABEL, "���� ������:");
	TRC(IDC_GRADIENTCOLOR1LABEL, "���� ��������� 1:");
	TRC(IDC_GRADIENTCOLOR2LABEL, "���� ��������� 2:");
	TRC(IDC_THUMBFONT, "�����...");
	TRC(IDC_THUMBTEXTLABEL, "������� �� ������:");
	TRC(IDC_TEXTOVERTHUMB2, "������� ������ ������");
	SetWindowText(TR("�������������� ���������"));	

	RECT rc = {13, 20, 290, 95};
	img.Create(GetDlgItem(IDC_LOGOGROUP), rc);
	img.LoadImage(0);

	SendDlgItemMessage(IDC_TRANSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)0) );
	
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������� ����� ����"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������ ����������"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������ ������� ����"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("����� ������ ����"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("����� ����������"));
	SendDlgItemMessage(IDC_LOGOPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������ ������ ����"));

	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������� ����� ����"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������ ����������"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������ ������� ����"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("����� ������ ����"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("����� ����������"));
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_ADDSTRING, 0, (LPARAM)TR("������ ������ ����"));

	SendDlgItemMessage(IDC_LOGOPOSITION, CB_SETCURSEL, Settings.ImageSettings.LogoPosition);
	SendDlgItemMessage(IDC_TEXTPOSITION, CB_SETCURSEL, Settings.ImageSettings.TextPosition);
	SetDlgItemText(IDC_LOGOEDIT, Settings.ImageSettings.LogoFileName);
	
	if(*Settings.ImageSettings.LogoFileName) 
		img.LoadImage(Settings.ImageSettings.LogoFileName);

	SetDlgItemText(IDC_EDITYOURTEXT, Settings.ImageSettings.Text);
	SetDlgItemText(IDC_THUMBTEXT, Settings.ThumbSettings.Text);
	
	FrameColor.SubclassWindow(GetDlgItem(IDC_FRAMECOLOR));
	FrameColor.SetColor(Settings.ThumbSettings.FrameColor);
	Color1.SubclassWindow(GetDlgItem(IDC_COLOR1));
	Color1.SetColor(Settings.ThumbSettings.ThumbColor1);
	

	Color2.SubclassWindow(GetDlgItem(IDC_COLOR2));
	Color2.SetColor(Settings.ThumbSettings.ThumbColor2);
	
	ThumbTextColor.SubclassWindow(GetDlgItem(IDC_THUMBTEXTCOLOR));
	ThumbTextColor.SetColor(Settings.ThumbSettings.ThumbTextColor);
	
	TextColor.SubclassWindow(GetDlgItem(IDC_SELECTCOLOR));
	TextColor.SetColor(Settings.ImageSettings.TextColor);
	
	StrokeColor.SubclassWindow(GetDlgItem(IDC_STROKECOLOR));
	StrokeColor.SetColor(Settings.ImageSettings.StrokeColor);
	SendDlgItemMessage(IDC_TEXTOVERTHUMB2, BM_SETCHECK, Settings.ThumbSettings.TextOverThumb);


	lf = Settings.ImageSettings.Font;
	ThumbFont = Settings.ThumbSettings.ThumbFont;

	return 1;  // Let the system set the focus
}

LRESULT CLogoSettings::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CLogoSettings::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CLogoSettings::OnBnClickedLogobrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("�����������"))+ _T(" (jpeg, bmp, png, gif ...)"),
		_T("*.jpg;*.gif;*.png;*.bmp;*.tiff"),
		TR("��� �����"),
		_T("*.*"));

	CFileDialog fd(true, 0, 0, 4|2, Buf, m_hWnd);
	
	CString s;
	s = GetAppFolder();
	fd.m_ofn.lpstrInitialDir = s;
	if ( fd.DoModal() != IDOK || !fd.m_szFileName ) return 0;

	LPTSTR FileName = fd.m_szFileName;

	SetDlgItemText(IDC_LOGOEDIT, FileName);
	img.LoadImage(FileName);
	img.Invalidate();

	return 0;
}

LRESULT CLogoSettings::OnBnClickedSelectfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Font selection dialog
	CFontDialog dlg(&lf);
	dlg.DoModal(m_hWnd);

	return 0;
}

LRESULT CLogoSettings::OnBnClickedThumbfont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Font selection dialog
	CFontDialog dlg(&ThumbFont);
	dlg.DoModal(m_hWnd);

	return 0;
}

bool CLogoSettings::Apply()
{
	int LogoPos, TextPos;

	LogoPos = SendDlgItemMessage(IDC_LOGOPOSITION, CB_GETCURSEL);
	TextPos = SendDlgItemMessage(IDC_TEXTPOSITION, CB_GETCURSEL);
	
	if(LogoPos == TextPos) // ���� "������� ����" � ������� ���������� � ���� � �� �� ����� �� ��������
	{
		if(MessageBox(TR("�� ������������� ������ ��������� ����� � ������� � ����� ����� �� �����������?"),TR("��������� �����������"),MB_ICONQUESTION|MB_YESNO)!=IDYES)
			return 0;
	}
	Settings.ImageSettings.LogoPosition = LogoPos;
	Settings.ImageSettings.TextPosition = TextPos;
	TCHAR Buf[256];
	GetDlgItemText(IDC_LOGOEDIT, Buf, 256);
	Settings.ImageSettings.LogoFileName = Buf;
	GetDlgItemText(IDC_THUMBTEXT, Buf, 256);
	Settings.ThumbSettings.Text = Buf;
	GetDlgItemText(IDC_EDITYOURTEXT, Buf, 256);
	Settings.ImageSettings.Text = Buf;
	Settings.ImageSettings.Font = lf;
	Settings.ThumbSettings.ThumbFont = ThumbFont;
	Settings.ThumbSettings.FrameColor = FrameColor.GetColor();
	
	Settings.ThumbSettings.ThumbColor1 = Color1.GetColor();
	
	Settings.ThumbSettings.ThumbColor2 = Color2.GetColor();
		
	Settings.ThumbSettings.ThumbTextColor=ThumbTextColor.GetColor();
	Settings.ImageSettings.TextColor=TextColor.GetColor();
	Settings.ImageSettings.StrokeColor=StrokeColor.GetColor();
	Settings.ThumbSettings.TextOverThumb = SendDlgItemMessage(IDC_TEXTOVERTHUMB2, BM_GETCHECK);

	return TRUE;
}



