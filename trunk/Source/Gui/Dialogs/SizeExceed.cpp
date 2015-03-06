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

#include "SizeExceed.h"
#include "atlheaders.h"
#include "Func/common.h"
#include "Core/Upload/UploadEngine.h"
#include "Func/Settings.h"
#include "Core/ScreenCapture.h"
#include "Gui/GuiTools.h"

// CSizeExceed
CSizeExceed::CSizeExceed(LPCTSTR szFileName, FullUploadProfile &iss, CMyEngineList * EngineList)
: m_UploadProfile(iss), m_EngineList(EngineList), m_ImageSettings(iss.convert_profile)
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
	for(int i=0; i<m_EngineList->count(); i++)
	{	
		CUploadEngineData * ue = m_EngineList->byIndex(i);
		if ( ue->Type != CUploadEngineData::TypeImageServer && ue->Type != CUploadEngineData::TypeFileServer ) {
			continue;
		}
		TCHAR buf[300]=_T(" ");
		TCHAR buf2[50];
		NewBytesToString(ue->MaxFileSize, buf2, 25);
		wsprintf(buf, ue->MaxFileSize?_T(" %s   (%s)"):_T(" %s"), (LPCTSTR)Utf8ToWstring(ue->Name).c_str(),(LPCTSTR)buf2);
		SendDlgItemMessage(IDC_SERVERLIST, CB_ADDSTRING, 0, (LPARAM)buf);
	}

	// Добавляем нужные форматы изображения(JPEG, PNG) в выпадающий список
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)TR("Авто"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("JPEG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("PNG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("GIF"));
	SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
	
	CString serverName= m_UploadProfile.upload_profile.serverName();

	int ServerID =  	_EngineList->GetUploadEngineIndex(serverName);
	
	SendDlgItemMessage(IDC_SERVERLIST,CB_SETCURSEL,ServerID);

	GuiTools::MakeLabelBold(GetDlgItem(IDC_FILEEXCEEDNAME));
	
	int f = MyGetFileSize(m_szFileName);
	WCHAR buf2[25];
	NewBytesToString(f, buf2, 25);

	TCHAR szBuf[1000];
	wsprintf(szBuf,CString(TR("Файл"))+ _T(" %s (%dx%d, %s)"),(LPCTSTR)myExtractFileName(m_szFileName),(int)img.ImageWidth,(int)img.ImageHeight, (LPCTSTR)buf2 );

	SetDlgItemText(IDC_FILEEXCEEDNAME, szBuf);
	NewBytesToString(m_EngineList->byName( m_UploadProfile.upload_profile.serverName())->MaxFileSize, buf2, 25);

	wsprintf(szBuf, TR("Файл превышает максимальный размер, допустимый для загрузки на сервер %s (%s)."),
		Utf8ToWstring(m_EngineList->byIndex(ServerID)->Name).c_str(), buf2);
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
    
   SendDlgItemMessage(IDC_KEEPASIS, BM_SETCHECK, !m_UploadProfile.upload_profile.getImageUploadParams().ProcessImages);
	

		SetDlgItemText(IDC_IMAGEWIDTH, m_ImageSettings.strNewWidth);

	
		SetDlgItemText(IDC_IMAGEHEIGHT,m_ImageSettings.strNewHeight);

	if(m_ImageSettings.Quality)
		SetDlgItemInt(IDC_QUALITYEDIT,m_ImageSettings.Quality);
	else
		SetDlgItemText(IDC_QUALITYEDIT,_T(""));
		
	SendDlgItemMessage(IDC_SAVEPROPORTIONS,BM_SETCHECK,m_ImageSettings.SaveProportions);
	SendDlgItemMessage(IDC_FORMATLIST,CB_SETCURSEL, m_ImageSettings.Format);

	CString serverName= m_UploadProfile.upload_profile.serverName();

	int ServerID =  	_EngineList->GetUploadEngineIndex(serverName);
		
	SendDlgItemMessage(IDC_SERVERLIST,CB_SETCURSEL, ServerID);
	BOOL temp;
	OnBnClickedKeepasis(0,0, 0, temp);
}

void CSizeExceed::GetParams()
{
	ImageUploadParams iup =  m_UploadProfile.upload_profile.getImageUploadParams();
	iup.ProcessImages = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0) != BST_CHECKED;
	m_ImageSettings.strNewWidth= GuiTools::GetWindowText(GetDlgItem(IDC_IMAGEWIDTH));
   m_ImageSettings.strNewHeight = GuiTools::GetWindowText(GetDlgItem(IDC_IMAGEHEIGHT));
	m_ImageSettings.SaveProportions = IS_CHECKED(IDC_SAVEPROPORTIONS);
	m_ImageSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	m_ImageSettings.Format = SendDlgItemMessage(IDC_FORMATLIST, CB_GETCURSEL);
	int serverId = SendDlgItemMessage(IDC_SERVERLIST, CB_GETCURSEL, 0, 0);;
	CUploadEngineData* ued = _EngineList->byIndex(serverId);
	m_UploadProfile.upload_profile.setServerName(ued ? Utf8ToWCstring(ued->Name) : _T(""));
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
