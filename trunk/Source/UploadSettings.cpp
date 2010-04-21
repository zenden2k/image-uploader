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
struct CImageParams;
#include "UploadSettings.h"

// CUploadSettings
CUploadSettings::CUploadSettings()
{
	
}

CUploadSettings::~CUploadSettings()
{
}

LRESULT CUploadSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PageWnd = m_hWnd;
	
	TRC(IDC_FORMATLABEL,"Формат:");
	TRC(IDC_QUALITYLABEL,"Качество:");
	TRC(IDC_RESIZEBYWIDTH,"Изменение ширины:");
	TRC(IDC_SAVEPROPORTIONS,"Сохранять пропорции");
	TRC(IDC_YOURLOGO,"Добавить водяной знак");
	TRC(IDC_XLABEL,"и/или высоты:");
	TRC(IDC_YOURTEXT,"Добавить текст на картинку");
	TRC(IDC_IMAGEPARAMETERS,"Параметры изображений");
	TRC(IDC_LOGOOPTIONS,"Дополнительно...");

	TRC(IDC_KEEPASIS,"Оставить без изменения");
	TRC(IDC_THUMBSETTINGS,"Настройки эскизов");

	TRC(IDC_CREATETHUMBNAILS,"Создавать превьюшки");
	TRC(IDC_SELECTSERVERLABEL,"Выберите сервер для загрузки изображений:");
	TRC(IDC_USESERVERTHUMBNAILS,"Использовать серверные эскизы");
	TRC(IDC_WIDTHLABEL,"Ширина эскиза:");
	TRC(IDC_DRAWFRAME,"Обводить в рамку");
	TRC(IDC_ADDFILESIZE,"Размеры изображения на картинке");
	TRC(IDC_LOGINBUTTON,"Авторизация");
	TRC(IDC_LOGINBUTTON2,"Авторизация");
	TRC(IDC_PRESSUPLOADBUTTON,"Нажмите кнопку \"Загрузить\" чтобы начать процесс загрузки");
	TRC(IDC_USETHUMBTEMPLATE,"Использовать шаблон");
	TRC(IDC_TEXTOVERTHUMB2, "Надпись поверх эскиза");
	TRC(IDC_STATIC, "Сервер для остальных типов файлов:");
	
	for(int i=0; i<EnginesList.GetCount(); i++)
	{	
		TCHAR buf[300] = _T(" ");
		lstrcat(buf, EnginesList[i].Name);
		int index = SendDlgItemMessage(EnginesList[i].ImageHost?IDC_SERVERLIST:IDC_SERVERLIST2,CB_ADDSTRING, 0, (LPARAM)buf);
		SendDlgItemMessage(EnginesList[i].ImageHost?IDC_SERVERLIST:IDC_SERVERLIST2,CB_SETITEMDATA, index, (LPARAM)i);
		if(i == Settings.ServerID)
		{
			SendDlgItemMessage(IDC_SERVERLIST,CB_SETCURSEL, index );
		}
		if(i == Settings.FileServerID)
		{
			SendDlgItemMessage(IDC_SERVERLIST2,CB_SETCURSEL, index );
		}
	}
	
	if(SendDlgItemMessage(IDC_SERVERLIST2,CB_GETCURSEL )==-1)
		SendDlgItemMessage(IDC_SERVERLIST2,CB_SETCURSEL,0 );
	BOOL temp;
	SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
	
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)TR("Авто"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("JPEG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("PNG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("GIF"));

	ShowParams();

	OnServerListSelectionChanged(0,0,0,temp); // These calls are necessary for showing/hiding "Authorize" buttons
	OnServerList2SelectionChanged(0,0,0,temp);

	return 1;  // Let the system set the focus
}

LRESULT CUploadSettings::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CUploadSettings::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}


LRESULT CUploadSettings::OnBnClickedKeepasis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	int id;
	BOOL checked = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0, 0);

	for(id=1004; id<1017; id++)
		::EnableWindow(GetDlgItem(id), !checked);

	::EnableWindow(GetDlgItem(IDC_SAVEPROPORTIONS), !checked);

	return 0;
}

void CUploadSettings::ShowParams(/*UPLOADPARAMS params*/)
{
	SendDlgItemMessage(IDC_KEEPASIS,BM_SETCHECK,Settings.ImageSettings.KeepAsIs);
	if(Settings.ImageSettings.NewWidth)
		SetDlgItemInt(IDC_IMAGEWIDTH,Settings.ImageSettings.NewWidth);
	else
		SetDlgItemText(IDC_IMAGEWIDTH,_T(""));

	if(Settings.ImageSettings.NewHeight)
		SetDlgItemInt(IDC_IMAGEHEIGHT,Settings.ImageSettings.NewHeight);
	else
		SetDlgItemText(IDC_IMAGEHEIGHT,_T(""));

	if(Settings.ImageSettings.Quality)
		SetDlgItemInt(IDC_QUALITYEDIT,Settings.ImageSettings.Quality);
	else
		SetDlgItemText(IDC_QUALITYEDIT,_T(""));


	SendDlgItemMessage(IDC_YOURLOGO,BM_SETCHECK, Settings.ImageSettings.AddLogo);
	SendDlgItemMessage(IDC_YOURTEXT,BM_SETCHECK, Settings.ImageSettings.AddText);
	SendDlgItemMessage(IDC_DRAWFRAME,BM_SETCHECK, Settings.ThumbSettings.DrawFrame);
	SendDlgItemMessage(IDC_FORMATLIST,CB_SETCURSEL, Settings.ImageSettings.Format);
	SendDlgItemMessage(IDC_CREATETHUMBNAILS,BM_SETCHECK,Settings.ThumbSettings.CreateThumbs);
	SendDlgItemMessage(IDC_SAVEPROPORTIONS,BM_SETCHECK,Settings.ImageSettings.SaveProportions);
	SendDlgItemMessage(IDC_ADDFILESIZE,BM_SETCHECK,Settings.ThumbSettings.ThumbAddImageSize);
	SendDlgItemMessage(IDC_USETHUMBTEMPLATE,BM_SETCHECK,Settings.ThumbSettings.UseThumbTemplate);
	if(!Settings.ThumbSettings.UseThumbTemplate)
		SendDlgItemMessage(IDC_USESERVERTHUMBNAILS,BM_SETCHECK,Settings.ThumbSettings.UseServerThumbs);

	SendDlgItemMessage(IDC_TEXTOVERTHUMB2, BM_SETCHECK, Settings.ThumbSettings.TextOverThumb);


	SendDlgItemMessage(IDC_CREATETHUMBNAILS,BM_SETCHECK,Settings.ThumbSettings.CreateThumbs);


	SetDlgItemInt(IDC_THUMBWIDTH,Settings.ThumbSettings.ThumbWidth);
}

LRESULT CUploadSettings::OnBnClickedCreatethumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	int id;
	BOOL checked = SendDlgItemMessage(IDC_CREATETHUMBNAILS, BM_GETCHECK, 0, 0);
	
	::EnableWindow(GetDlgItem(IDC_THUMBWIDTH), checked);
	::EnableWindow(GetDlgItem(IDC_ADDFILESIZE), checked);
	::EnableWindow(GetDlgItem(IDC_WIDTHLABEL), checked);
	::EnableWindow(GetDlgItem(IDC_USETHUMBTEMPLATE), checked);
	::EnableWindow(GetDlgItem(IDC_USESERVERTHUMBNAILS), checked);
	::EnableWindow(GetDlgItem(IDC_TEXTOVERTHUMB2), checked);
		
	if(!checked)
		::EnableWindow(GetDlgItem(IDC_DRAWFRAME), checked);
	else 
		OnBnClickedUseThumbTemplate(0, 0, 0, bHandled);
		
	::EnableWindow(GetDlgItem(IDC_PXLABEL), checked);
		
	return 0;
}

LRESULT CUploadSettings::OnBnClickedLogooptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSettingsDlg dlg(2); // Open settings dialog and logo options tab
	dlg.DoModal(m_hWnd);
	return 0;
}

bool CUploadSettings::OnNext()
{
	Settings.ThumbSettings.TextOverThumb = SendDlgItemMessage(IDC_TEXTOVERTHUMB2, BM_GETCHECK);
	
	Settings.ImageSettings.KeepAsIs = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0) == BST_CHECKED;
	Settings.ImageSettings.NewWidth= GetDlgItemInt(IDC_IMAGEWIDTH);
	Settings.ImageSettings.NewHeight = GetDlgItemInt(IDC_IMAGEHEIGHT);
	Settings.ImageSettings.AddLogo = IsChecked(IDC_YOURLOGO);
	Settings.ImageSettings.AddText = IsChecked(IDC_YOURTEXT);
	Settings.ThumbSettings.CreateThumbs = IsChecked(IDC_CREATETHUMBNAILS);
	Settings.ThumbSettings.UseServerThumbs = IsChecked(IDC_USESERVERTHUMBNAILS);
	Settings.ThumbSettings.UseThumbTemplate = IsChecked(IDC_USETHUMBTEMPLATE);
	Settings.ThumbSettings.DrawFrame = IsChecked(IDC_DRAWFRAME);
	Settings.ThumbSettings.ThumbAddImageSize = IsChecked(IDC_ADDFILESIZE);
	Settings.ImageSettings.SaveProportions = IsChecked(IDC_SAVEPROPORTIONS);
	Settings.ImageSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	Settings.ImageSettings.Format = SendDlgItemMessage(IDC_FORMATLIST, CB_GETCURSEL);
	

	int temp= SendDlgItemMessage(IDC_SERVERLIST, CB_GETCURSEL, 0, 0);
	Settings.ServerID = SendDlgItemMessage(IDC_SERVERLIST, CB_GETITEMDATA, temp, 0);


	temp= SendDlgItemMessage(IDC_SERVERLIST2, CB_GETCURSEL, 0, 0);
	Settings.FileServerID = SendDlgItemMessage(IDC_SERVERLIST2, CB_GETITEMDATA, temp, 0);
	
	Settings.ThumbSettings.ThumbWidth = GetDlgItemInt(IDC_THUMBWIDTH);
	
	return true;
}

bool CUploadSettings::OnShow()
{
	BOOL temp;
	
	OnBnClickedCreatethumbnails(0, 0, 0, temp);
	OnBnClickedKeepasis(0, 0, 0, temp);
	EnableNext();
	SetNextCaption(TR("&Загрузить"));
	return true;
}

LRESULT CUploadSettings::OnBnClickedLogin(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int temp = SendDlgItemMessage((wID == IDC_LOGINBUTTON)? IDC_SERVERLIST:IDC_SERVERLIST2, CB_GETCURSEL, 0);
	int ServerID = SendDlgItemMessage((wID == IDC_LOGINBUTTON)? IDC_SERVERLIST:IDC_SERVERLIST2, CB_GETITEMDATA, temp);

	CLoginDlg dlg(ServerID);
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CUploadSettings::OnServerListSelectionChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int ServerID = SendDlgItemMessage(IDC_SERVERLIST, CB_GETCURSEL, 0);
	if(ServerID < 0) return 0;
	int temp = SendDlgItemMessage(IDC_SERVERLIST, CB_GETITEMDATA, ServerID);
	::ShowWindow(GetDlgItem(IDC_LOGINBUTTON), EnginesList[temp ].NeedAuthorization ? SW_SHOW : SW_HIDE);
	return 0;
}


LRESULT CUploadSettings::OnServerList2SelectionChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int ServerID = SendDlgItemMessage(IDC_SERVERLIST2, CB_GETCURSEL, 0);
	if(ServerID < 0) return 0;
	int temp = SendDlgItemMessage(IDC_SERVERLIST2, CB_GETITEMDATA, ServerID);

	::ShowWindow(GetDlgItem(IDC_LOGINBUTTON2), EnginesList[temp].NeedAuthorization ? SW_SHOW : SW_HIDE);
	return 0;
}

LRESULT CUploadSettings::OnBnClickedUseThumbTemplate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL checked = SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_GETCHECK, 0, 0);

	if(checked && !FileExists(GetAppFolder() + _T("Data\\thumb.png")) && wID == IDC_USETHUMBTEMPLATE)
	{
		MessageBox(TR("Невозможно использовать шаблон для превьюшки. Файл шаблона \"Data\\Thumb.png\" не найден."), APPNAME, MB_ICONWARNING);
		SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_SETCHECK, false);
		return 0;
	}
	
	if(checked) SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_SETCHECK, false);

	checked = checked || SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_GETCHECK, 0, 0);
	::EnableWindow(GetDlgItem(IDC_DRAWFRAME), !checked);
	::EnableWindow(GetDlgItem(IDC_TEXTOVERTHUMB2),!checked);
	
	return 0;
}
	
LRESULT CUploadSettings::OnBnClickedUseServerThumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL checked=SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_GETCHECK, 0, 0);
	
	if(checked) SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_SETCHECK, false);
	
	checked = checked || SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_GETCHECK, 0, 0);
	::EnableWindow(GetDlgItem(IDC_DRAWFRAME),!checked);
	::EnableWindow(GetDlgItem(IDC_TEXTOVERTHUMB2),!checked);
		
	return 0;
}
	