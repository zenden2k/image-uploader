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

#include "stdafx.h"
#include "SizeExceed.h"
#include "common.h"
// CSizeExceed
CSizeExceed::CSizeExceed(LPTSTR szFileName, ImageSettingsStruct &iss): m_ImageSettings(iss)
{
	m_szFileName = szFileName;
}

CSizeExceed::~CSizeExceed()
{
}

void CSizeExceed::Translate()
{
	TRC(IDC_WHATTODO, "Измените настройки изображения или выберите другой сервер, чтобы изображение могло быть загружено.");
	TRC(IDC_IMAGESETTINGS, "Параметры изображения");
	TRC(IDC_FORMATLABEL, "Формат:");
	TRC(IDC_QUALITYLABEL, "Качество:");
	TRC(IDC_RESIZEBYWIDTH, "Изменение ширины:");
	TRC(IDC_SAVEPROPORTIONS, "Сохранять пропорции");
	TRC(IDC_XLABEL, "и/или высоты:");
	TRC(IDOK, "OK");
	TRC(IDC_FORALL, "Для всех");
	TRC(IDCANCEL, "Игнорировать");
	TRC(IDC_SELECTSERVERLABEL, "Сервер для загрузки изображений:");
	TRC(IDC_KEEPASIS, "Оставить без изменения");
	SetWindowText(TR("Превышение размера"));
}

LRESULT CSizeExceed::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc = {12, 30, 162, 144};
	img.Create(m_hWnd, rc);
	img.LoadImage(m_szFileName);

	CenterWindow(GetParent());
	for(int i=0; i<EnginesList.size(); i++)
	{	
		TCHAR buf[300]=_T(" ");
		TCHAR buf2[50];
		NewBytesToString(EnginesList[i].MaxFileSize, buf2, 25);
		wsprintf(buf, EnginesList[i].MaxFileSize?_T(" %s   (%s)"):_T(" %s"), EnginesList[i].Name,buf2);
		SendDlgItemMessage(IDC_SERVERLIST, CB_ADDSTRING, 0, (LPARAM)buf);
	}

	// Добавляем нужные форматы изображения(JPEG, PNG) в выпадающий список
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)TR("Авто"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("JPEG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("PNG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("GIF"));
	SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
	
	int ServerID = m_ImageSettings.ServerID;
	
	SendDlgItemMessage(IDC_SERVERLIST,CB_SETCURSEL,Settings.ServerID);

	MakeLabelBold(GetDlgItem(IDC_FILEEXCEEDNAME));
	
	int f = MyGetFileSize(m_szFileName);
	WCHAR buf2[25];
	NewBytesToString(f, buf2, 25);

	TCHAR szBuf[1000];
	wsprintf(szBuf,CString(TR("Файл"))+ _T(" %s (%dx%d, %s)"),myExtractFileName(m_szFileName),(int)img.ImageWidth,(int)img.ImageHeight, buf2 );

	SetDlgItemText(IDC_FILEEXCEEDNAME, szBuf);
	NewBytesToString(EnginesList[m_ImageSettings.ServerID].MaxFileSize, buf2, 25);

	wsprintf(szBuf, TR("Файл превышает максимальный размер, допустимый для загрузки на сервер %s (%s)."),
	EnginesList[ServerID].Name, buf2);
	SetDlgItemText(IDC_FILEEXCEEDSIZE2, szBuf);
	Translate();
	DisplayParams();

	return 1;  // Let the system set the focus
}

LRESULT CSizeExceed::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetParams();
	EndDialog(wID);
	return 0;
}

LRESULT CSizeExceed::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

void CSizeExceed::DisplayParams(void)
{
	SendDlgItemMessage(IDC_KEEPASIS, BM_SETCHECK, m_ImageSettings.KeepAsIs);
	
	if(m_ImageSettings.NewWidth)
		SetDlgItemInt(IDC_IMAGEWIDTH,m_ImageSettings.NewWidth);
	else
		SetDlgItemText(IDC_IMAGEWIDTH, _T(""));

	if(m_ImageSettings.NewHeight)
		SetDlgItemInt(IDC_IMAGEHEIGHT,m_ImageSettings.NewHeight);
	else
		SetDlgItemText(IDC_IMAGEHEIGHT,_T(""));

	if(m_ImageSettings.Quality)
		SetDlgItemInt(IDC_QUALITYEDIT,m_ImageSettings.Quality);
	else
		SetDlgItemText(IDC_QUALITYEDIT,_T(""));
		
	SendDlgItemMessage(IDC_SAVEPROPORTIONS,BM_SETCHECK,m_ImageSettings.SaveProportions);
	SendDlgItemMessage(IDC_FORMATLIST,CB_SETCURSEL, m_ImageSettings.Format);
		
	SendDlgItemMessage(IDC_SERVERLIST,CB_SETCURSEL,m_ImageSettings.ServerID);
	BOOL temp;
	OnBnClickedKeepasis(0,0, 0, temp);
}

void CSizeExceed::GetParams()
{
	m_ImageSettings.KeepAsIs = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0) == BST_CHECKED;
	m_ImageSettings.NewWidth= GetDlgItemInt(IDC_IMAGEWIDTH);
	m_ImageSettings.NewHeight = GetDlgItemInt(IDC_IMAGEHEIGHT);
	m_ImageSettings.SaveProportions = IsChecked(IDC_SAVEPROPORTIONS);
	m_ImageSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	m_ImageSettings.Format = SendDlgItemMessage(IDC_FORMATLIST, CB_GETCURSEL);
	m_ImageSettings.ServerID = SendDlgItemMessage(IDC_SERVERLIST, CB_GETCURSEL, 0, 0);
}

LRESULT CSizeExceed::OnBnClickedForall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetParams();
	EndDialog(3);
	return 0;
}

LRESULT CSizeExceed::OnBnClickedKeepasis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL checked = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0, 0);

	for(int id=1004; id<1017; id++)
		::EnableWindow(GetDlgItem(id), !checked);
		
	::EnableWindow(GetDlgItem(IDC_SAVEPROPORTIONS), !checked);

	return 0;
}
