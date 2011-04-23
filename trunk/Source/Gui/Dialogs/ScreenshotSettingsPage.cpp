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

#include "../../atlheaders.h"
#include "ScreenshotSettingsPage.h"
#include "../../Func/Common.h"
#include "LogWindow.h"
#include "../../Func/Settings.h"
#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}

// CScreenshotSettingsPagePage
CScreenshotSettingsPagePage::CScreenshotSettingsPagePage()
{

}

CScreenshotSettingsPagePage::~CScreenshotSettingsPagePage()
{
}

LRESULT CScreenshotSettingsPagePage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRC(IDC_GROUPPARAMS, "�������������");
	TRC(IDC_QUALITYLABEL, "��������:");
	TRC(IDC_DELAYLABEL, "��������:");
	TRC(IDC_FORMATLABEL, "������:");
	TRC(IDC_SECLABEL, "���");
	TRC(IDC_MSECLABEL, "��");
	TRC(IDC_SCREENSHOTSFOLDERSELECT, "�����");
	TRC(IDC_SCREENSHOTFOLDERLABEL, "����� ��� ���������� ����������:");
	TRC(IDC_SCREENSHOTFILENAMELABEL, "������ ����� �����");
	TRC(IDC_DELAYLABEL2, "�������� ��� ������� ����:");
	TRC(IDC_ALWAYSCOPYTOCLIPBOARD, "������ ���������� � ����� ������");
	TRC(IDC_SCREENSHOTSAVINGPARAMS, "��������� ���������� �������");
	TRC(IDC_FOREGROUNDWHENSHOOTING, "�������� ���� �� �������� ���� ��� ������ �����");
	TRC(IDC_PARAMETERSHINTLABEL, "%y - ���, %m - �����, %d - ����\n%h - ���, %n - ������, %s - �������\n %i - ���������� �����,\n%width% - ������,  %height% - ������ �����������");
	TRC(IDC_ADDSHADOW, "��������� ���� ����");
	TRC(IDC_REMOVECORNERS, "������� ������ � ����");
	TRC(IDC_REMOVEBACKGROUND, "������� ��� ����");
	
	SetDlgItemText(IDC_SCREENSHOTFILENAMEEDIT, Settings.ScreenshotSettings.FilenameTemplate);

	SetDlgItemText(IDC_SCREENSHOTFOLDEREDIT, Settings.ScreenshotSettings.Folder);
	SendDlgItemMessage(IDC_DELAYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)30, (short)0) );
	SendDlgItemMessage(IDC_QUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
	
	SendDlgItemMessage(IDC_FORMATLIST, CB_ADDSTRING, 0, (LPARAM)_T("JPEG"));
	SendDlgItemMessage(IDC_FORMATLIST, CB_ADDSTRING, 0, (LPARAM)_T("PNG"));
	SendDlgItemMessage(IDC_FOREGROUNDWHENSHOOTING, BM_SETCHECK,Settings.ScreenshotSettings.ShowForeground);
	SendDlgItemMessage(IDC_ALWAYSCOPYTOCLIPBOARD, BM_SETCHECK, Settings.ScreenshotSettings.CopyToClipboard);

	SendDlgItemMessage(IDC_REMOVECORNERS, BM_SETCHECK, Settings.ScreenshotSettings.RemoveCorners);
	SendDlgItemMessage(IDC_ADDSHADOW, BM_SETCHECK, Settings.ScreenshotSettings.AddShadow);
	SendDlgItemMessage(IDC_REMOVEBACKGROUND, BM_SETCHECK, Settings.ScreenshotSettings.RemoveBackground);

	int Quality, Delay, Format;
	Quality = Settings.ScreenshotSettings.Quality;
	Format = Settings.ScreenshotSettings.Format;
	Delay = Settings.ScreenshotSettings.Delay;

	if( Format < 0) Format = 0;
	if( Quality < 0) Quality = 85;
	if( Delay < 0 || Delay > 30) Delay = 2;

	SetDlgItemInt(IDC_QUALITYEDIT, Quality);
	SetDlgItemInt(IDC_DELAYEDIT, Delay);
	SetDlgItemInt(IDC_WINDOWHIDINGDELAY, Settings.ScreenshotSettings.WindowHidingDelay);
	SendDlgItemMessage(IDC_FORMATLIST, CB_SETCURSEL, Format, 0);

	bool isVista = IsVista();
	::EnableWindow(GetDlgItem(IDC_AEROONLY), isVista);
	::EnableWindow(GetDlgItem(IDC_REMOVECORNERS), isVista);
	::EnableWindow(GetDlgItem(IDC_ADDSHADOW), isVista);
	::EnableWindow(GetDlgItem(IDC_REMOVEBACKGROUND), isVista);
	return 1;  // Let the system set the focus
}

bool CScreenshotSettingsPagePage::Apply()
{
	CString fileName = IU_GetWindowText(GetDlgItem(IDC_SCREENSHOTFILENAMEEDIT));
	if(!CheckFileName(fileName))
	{
		MessageBox(TR("��� ����� �������� ����������� �������!"));
		::SetFocus(GetDlgItem(IDC_SCREENSHOTFILENAMEEDIT));
		return false;
	}
	Settings.ScreenshotSettings.FilenameTemplate = fileName;
	
	Settings.ScreenshotSettings.Format = SendDlgItemMessage(IDC_FORMATLIST,CB_GETCURSEL,0,0);
	Settings.ScreenshotSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	Settings.ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);
	Settings.ScreenshotSettings.ShowForeground = SendDlgItemMessage(IDC_FOREGROUNDWHENSHOOTING, BM_GETCHECK) == BST_CHECKED;

	Settings.ScreenshotSettings.Folder = IU_GetWindowText(GetDlgItem(IDC_SCREENSHOTFOLDEREDIT));
	Settings.ScreenshotSettings.Format = SendDlgItemMessage(IDC_FORMATLIST,CB_GETCURSEL,0,0);
	Settings.ScreenshotSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	Settings.ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);
	Settings.ScreenshotSettings.ShowForeground = SendDlgItemMessage(IDC_FOREGROUNDWHENSHOOTING, BM_GETCHECK) == BST_CHECKED;
	Settings.ScreenshotSettings.CopyToClipboard =  SendDlgItemMessage(IDC_ALWAYSCOPYTOCLIPBOARD, BM_GETCHECK) == BST_CHECKED;
	
	Settings.ScreenshotSettings.WindowHidingDelay = GetDlgItemInt(IDC_WINDOWHIDINGDELAY);

	Settings.ScreenshotSettings.RemoveCorners = SendDlgItemMessage(IDC_REMOVECORNERS, BM_GETCHECK );
	Settings.ScreenshotSettings.AddShadow = SendDlgItemMessage(IDC_ADDSHADOW, BM_GETCHECK);
	Settings.ScreenshotSettings.RemoveBackground = SendDlgItemMessage(IDC_REMOVEBACKGROUND, BM_GETCHECK);

	return true;
}

LRESULT CScreenshotSettingsPagePage::OnScreenshotsFolderSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CFolderDialog fd(m_hWnd,TR("����� �����"), BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE );
	CString path = IU_GetWindowText(GetDlgItem(IDC_SCREENSHOTFOLDEREDIT));
	fd.SetInitialFolder(path,true);
	if(fd.DoModal(m_hWnd) == IDOK)
	{
		SetDlgItemText(IDC_SCREENSHOTFOLDEREDIT,fd.GetFolderPath());
		return true;
	}
	return 0;
}
