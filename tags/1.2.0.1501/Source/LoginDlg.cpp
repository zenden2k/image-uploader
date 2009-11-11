/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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
#include "LoginDlg.h"
#include "wizarddlg.h"

// 
void DecodeString(LPCTSTR szSource, CString &Result, LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ")
{
	TCHAR szDestination[1024];
   int br = strlen(code);
   int n = lstrlen(szSource) / 2;
   int j = 0;
	ZeroMemory(szDestination, n*2);

	int i;
   PBYTE data = (PBYTE)szDestination;
   *szDestination=0;

   for(i=0; i<n; i++)
   {
      if(j >= br) j=0;

      BYTE b;
		b = (szSource[i*2] - _T('A'))*16 + (szSource[i*2+1] - _T('A'));
      b = b^code[j];
		data[i] = b;
      j++;
   }
   data[i]=0;
	Result = szDestination;
}


LoginInfo LoadLogin(int ServerId)
{
	LoginInfo Result;
	UploadEngine UE = EnginesList[ServerId];
	return Settings.AuthParams[UE.Name];
}

void EncodeString(LPCTSTR szSource, CString &Result,LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ")
{
	TCHAR szDestination[1024];
	int br = strlen(code);
	int n = lstrlen(szSource) * 2;
	int j = 0;

	PBYTE data = (PBYTE)szSource;
	*szDestination = 0;
	for(int i=0; i<n; i++)
	{
		if(j>=br)j=0;

		BYTE b;
		b = data[i]^code[j];
		TCHAR bb[2]={0,0};
		bb[0]=_T('A')+b/16;
		lstrcat(szDestination,bb);
		bb[0]=_T('A')+b%16;
		lstrcat(szDestination,bb);
		j++;

	}
	Result = szDestination;
}

bool SaveLogin(int ServerId,LoginInfo li)
{

	UploadEngine &UE = EnginesList[ServerId];
	Settings.AuthParams[UE.Name] = li;
	return true;
}

// CLoginDlg
CLoginDlg::CLoginDlg(int ServerId)
{
	this->ServerId = ServerId;
}

CLoginDlg::~CLoginDlg()
{
	
}

LRESULT CLoginDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	LoginInfo li = LoadLogin(ServerId);

	SetWindowText(TR("Параметры авторизации"));
	TRC(IDC_LOGINLABEL, "Логин:");
	TRC(IDC_PASSWORDLABEL, "Пароль:");
	TRC(IDC_USEIECOOKIES, "Использовать cookies из Internet Explorer");
	TRC(IDCANCEL, "Отмена");
	
	SetDlgItemText(IDC_LOGINEDIT, li.Login);
	SetDlgItemText(IDC_PASSWORDEDIT, li.Password);
	SetDlgItemText(IDC_LOGINFRAME, EnginesList[ServerId].Name);
	SendDlgItemMessage(IDC_USEIECOOKIES, BM_SETCHECK, (li.UseIeCookies?BST_CHECKED:BST_UNCHECKED));
	OnClickedUseIeCookies(0, 0, 0, bHandled);
	return 1;  // Разрешаем системе самостоятельно установить фокус ввода
}

LRESULT CLoginDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	LoginInfo li;
	TCHAR Buffer[256];

	GetDlgItemText(IDC_LOGINEDIT, Buffer, 256);
	li.Login = Buffer;
	GetDlgItemText(IDC_PASSWORDEDIT, Buffer, 256);
	li.Password = Buffer;
	li.UseIeCookies = SendDlgItemMessage(IDC_USEIECOOKIES, BM_GETCHECK);
	
	SaveLogin(ServerId, li);

	EndDialog(wID);
	return 0;
}

LRESULT CLoginDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CLoginDlg::OnClickedUseIeCookies(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::EnableWindow(GetDlgItem(IDC_LOGINEDIT), !IsChecked(IDC_USEIECOOKIES));
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), !IsChecked(IDC_USEIECOOKIES));
	return 0;
}
