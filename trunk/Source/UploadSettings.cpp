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

#include "UploadSettings.h"
#include "ServerFolderSelect.h"
#include "NewFolderDlg.h"
#include "ServerParamsDlg.h"
#include "Gui/GuiTools.h"
#include "Gui/Dialogs/ConvertPresetDlg.h"

CUploadSettings::CUploadSettings(CMyEngineList * EngineList)
{
	nImageIndex = nFileIndex = -1;
	m_EngineList = EngineList;
}

CUploadSettings::~CUploadSettings()
{
}

void CUploadSettings::TranslateUI()
{
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
	TRC(IDC_THUMBSETTINGS,"Настройки превью");

	TRC(IDC_CREATETHUMBNAILS,"Создавать превью");
	TRC(IDC_IMAGESERVERGROUPBOX,"Cервер для загрузки изображений");
	TRC(IDC_USESERVERTHUMBNAILS,"Использовать серверные эскизы");
	TRC(IDC_WIDTHLABEL,"Ширина эскиза:");
	TRC(IDC_DRAWFRAME,"Обводить в рамку");
	TRC(IDC_ADDFILESIZE,"Размеры изображения на картинке");
	TRC(IDC_PRESSUPLOADBUTTON,"Нажмите кнопку \"Загрузить\" чтобы начать процесс загрузки");
	TRC(IDC_USETHUMBTEMPLATE,"Использовать шаблон");

	TRC(IDC_FILESERVERGROUPBOX, "Сервер для остальных типов файлов");
}

LRESULT CUploadSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PageWnd = m_hWnd;
	TranslateUI();

	m_nImageServer = Settings.ServerID;
	m_nFileServer = Settings.FileServerID;

	HBITMAP hBitmap;

	// Get color depth (minimum requirement is 32-bits for alpha blended images).
	int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP),BITSPIXEL);
	/*if (iBitsPixel >= 32)
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_BITMAP5));
		m_PlaceSelectorImageList.Create(16,16,ILC_COLOR32,0,6);
		m_PlaceSelectorImageList.Add(hBitmap, (HBITMAP) NULL);
	}
	else*/
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_SERVERTOOLBARBMP2));
		m_PlaceSelectorImageList.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
		m_PlaceSelectorImageList.Add(hBitmap,RGB(255,0,255));
	}
	
	
	RECT Toolbar1Rect;
	::GetWindowRect(GetDlgItem(IDC_IMAGESERVERGROUPBOX), &Toolbar1Rect);
	::MapWindowPoints(0, m_hWnd, (LPPOINT)&Toolbar1Rect, 2);
	Toolbar1Rect.top += dlgY(9);
	Toolbar1Rect.bottom -= dlgY(3);
	Toolbar1Rect.left += dlgX(6);
	Toolbar1Rect.right -= dlgX(6);

	RECT Toolbar2Rect;
	::GetWindowRect(GetDlgItem(IDC_FILESERVERGROUPBOX), &Toolbar2Rect);
	::MapWindowPoints(0, m_hWnd, (LPPOINT)&Toolbar2Rect, 2);
	Toolbar2Rect.top += dlgY(9);
	Toolbar2Rect.bottom -= dlgY(3);
	Toolbar2Rect.left += dlgX(6);
	Toolbar2Rect.right -= dlgX(6);

	for(int i = 0; i<2; i++)
	{
		CToolBarCtrl& CurrentToolbar = (i == 0) ? Toolbar: FileServerSelectBar;
		CurrentToolbar.Create(m_hWnd,i?Toolbar2Rect:Toolbar1Rect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| CCS_NORESIZE|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
		
		CurrentToolbar.SetButtonStructSize();
		CurrentToolbar.SetButtonSize(30,18);
		CurrentToolbar.SetImageList(m_PlaceSelectorImageList);
		
		CurrentToolbar.AddButton(IDC_SERVERBUTTON, TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, -1, TR("Выберите сервер..."), 0);
		CurrentToolbar.AddButton(IDC_TOOLBARSEPARATOR1, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR(""), 0);
		
		CurrentToolbar.AddButton(IDC_LOGINTOOLBUTTON + !i, /*TBSTYLE_BUTTON*/TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0, _T(""), 0);
		CurrentToolbar.AddButton(IDC_TOOLBARSEPARATOR2, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR(""), 0);
		
		CurrentToolbar.AddButton(IDC_SELECTFOLDER, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Выберите папку..."), 0);
		CurrentToolbar.AutoSize();
	}

	Toolbar.SetWindowLong(GWL_ID, IDC_IMAGETOOLBAR);
	FileServerSelectBar.SetWindowLong(GWL_ID, IDC_FILETOOLBAR);

	SendDlgItemMessage(IDC_THUMBWIDTHSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)1000, (short)40) );
	

	SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
	SendDlgItemMessage(IDC_THUMBQUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
	
	
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)TR("Авто"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("JPEG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("PNG"));
	SendDlgItemMessage(IDC_FORMATLIST,CB_ADDSTRING,0,(LPARAM)_T("GIF"));
	
	ShowParams();

	UpdateAllPlaceSelectors();
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
	SendDlgItemMessage(IDC_THUMBFORMATLIST,CB_SETCURSEL, Settings.ThumbSettings.ThumbFormat);
	SendDlgItemMessage(IDC_CREATETHUMBNAILS,BM_SETCHECK,Settings.ThumbSettings.CreateThumbs);
	SendDlgItemMessage(IDC_SAVEPROPORTIONS,BM_SETCHECK,Settings.ImageSettings.SaveProportions);
	SendDlgItemMessage(IDC_ADDFILESIZE,BM_SETCHECK,Settings.ThumbSettings.ThumbAddImageSize);
	SendDlgItemMessage(IDC_USETHUMBTEMPLATE,BM_SETCHECK,Settings.ThumbSettings.UseThumbTemplate);
	if(!Settings.ThumbSettings.UseThumbTemplate)
		SendDlgItemMessage(IDC_USESERVERTHUMBNAILS,BM_SETCHECK,Settings.ThumbSettings.UseServerThumbs);

	

	SendDlgItemMessage(IDC_CREATETHUMBNAILS,BM_SETCHECK,Settings.ThumbSettings.CreateThumbs);


	SetDlgItemInt(IDC_THUMBWIDTH,Settings.ThumbSettings.ThumbWidth);
}

LRESULT CUploadSettings::OnBnClickedCreatethumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	BOOL checked = SendDlgItemMessage(IDC_CREATETHUMBNAILS, BM_GETCHECK, 0, 0);
	
	::EnableWindow(GetDlgItem(IDC_THUMBWIDTH), checked);
	::EnableWindow(GetDlgItem(IDC_ADDFILESIZE), checked);
	::EnableWindow(GetDlgItem(IDC_WIDTHLABEL), checked);
	::EnableWindow(GetDlgItem(IDC_USETHUMBTEMPLATE), checked);
	::EnableWindow(GetDlgItem(IDC_USESERVERTHUMBNAILS), checked);
	//::EnableWindow(GetDlgItem(IDC_TEXTOVERTHUMB2), checked);
		
	if(!checked)
		::EnableWindow(GetDlgItem(IDC_DRAWFRAME), checked);
	else 
		OnBnClickedUseThumbTemplate(0, 0, 0, bHandled);
		
	::EnableWindow(GetDlgItem(IDC_PXLABEL), checked);
		
	return 0;
}

LRESULT CUploadSettings::OnBnClickedLogooptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CConvertPresetDlg dlg(0);
	/*CSettingsDlg dlg(1);*/ // Open settings dialog and logo options tab
	dlg.DoModal(m_hWnd);
	return 0;
}

bool CUploadSettings::OnNext()
{
	if(m_nImageServer == -1 || m_nFileServer == -1)
	{
		//MessageBox(TR("Вы не выбрали сервер!"), APPNAME, MB_ICONWARNING);
		//return false;
	}
	
	if(m_nImageServer != -1)
	{
		CUploadEngineData *ue = m_EngineList->byIndex(m_nImageServer);
		if(ue->NeedAuthorization ==2 && !Settings.ServersSettings[Utf8ToWstring(ue->Name).c_str()].authData.DoAuth)
		{ 
			CString errorMsg;
			errorMsg.Format(TR("Загрузка файлов на сервер '%s' невозможна без наличия учетной записи.\nПожалуйста, зарегистрируйтесь на данном сервере и укажите ваши регистрационные данные в программе."), (LPCTSTR)Utf8ToWCstring(ue->Name));
			MessageBox(errorMsg, APPNAME, MB_ICONERROR);
			return false;
		}
	}
	if(m_nFileServer != -1)
	{
		CUploadEngineData *ue2 = m_EngineList->byIndex(m_nFileServer);
		if(ue2->NeedAuthorization == 2 && !Settings.ServersSettings[Utf8ToWstring(ue2->Name).c_str()].authData.DoAuth)
		{
			CString errorMsg;
			errorMsg.Format(TR("Вы не задали параметры авторизации на сервере '%s'!"), (LPCTSTR)Utf8ToWstring(ue2->Name).c_str());
			MessageBox(errorMsg, APPNAME, MB_ICONWARNING);
			return false;
		}
	}
	
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
	
	Settings.ServerID = m_nImageServer;
	
	Settings.FileServerID = m_nFileServer;
	Settings.ThumbSettings.ThumbWidth = GetDlgItemInt(IDC_THUMBWIDTH);
	
	return true;
}

bool CUploadSettings::OnShow()
{
	BOOL temp;
	

	OnBnClickedCreatethumbnails(0, 0, 0, temp);
	OnBnClickedKeepasis(0, 0, 0, temp);
	EnableNext();
	//MessageBox(Settings.ServersSettings[EnginesList[ServerID].Name].params["FolderTitle"]);
	//if(ServerID >=0)
	{
		
//		m_FolderLink.SetWindowText(Settings.ServersSettings[EnginesList[ServerID].Name].params["FolderTitle"]);
	
	}
		SetNextCaption(TR("&Загрузить"));
	return true;
}

LRESULT CUploadSettings::OnBnClickedLogin(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;// (hWndCtl == Toolbar.m_hWnd);	
	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
	CLoginDlg dlg(m_EngineList->byIndex(nServerIndex));

	ServerSettingsStruct & ss = Settings.ServersSettings[Utf8ToWstring(m_EngineList->byIndex(nServerIndex)->Name).c_str()];
	std::string UserName = ss.authData.Login; 

	if( dlg.DoModal(m_hWnd) == IDOK)
	{
		if(UserName != ss.authData.Login)
		{
			ss.params["FolderID"]="";
			ss.params["FolderTitle"]="";
			ss.params["FolderUrl"]="";
			iuPluginManager.UnloadPlugins();
		}
			
		UpdateAllPlaceSelectors();
	}
	return 0;
}

LRESULT CUploadSettings::OnBnClickedUseThumbTemplate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL checked = SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_GETCHECK, 0, 0);

	if(checked && !FileExists(IU_GetDataFolder() + _T("thumb.png")) && wID == IDC_USETHUMBTEMPLATE)
	{
		MessageBox(TR("Невозможно использовать шаблон для миниатюры. Файл шаблона \"Data\\Thumb.png\" не найден."), APPNAME, MB_ICONWARNING);
		SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_SETCHECK, false);
		return 0;
	}
	
	if(checked) SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_SETCHECK, false);

	checked = checked || SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_GETCHECK, 0, 0);
	::EnableWindow(GetDlgItem(IDC_DRAWFRAME), !checked);
	//::EnableWindow(GetDlgItem(IDC_TEXTOVERTHUMB2),!checked);
	
	return 0;
}
	
LRESULT CUploadSettings::OnBnClickedUseServerThumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL checked=SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_GETCHECK, 0, 0);
	
	if(checked) SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_SETCHECK, false);
	
	checked = checked || SendDlgItemMessage(IDC_USETHUMBTEMPLATE, BM_GETCHECK, 0, 0);
	::EnableWindow(GetDlgItem(IDC_DRAWFRAME),!checked);
	return 0;
}
	
LRESULT CUploadSettings::OnBnClickedSelectFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
bool ImageServer = (hWndCtl == Toolbar.m_hWnd);	
	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;

	CUploadEngineData *ue = m_EngineList->byIndex(nServerIndex);

	if(ue->SupportsFolders){
		CServerFolderSelect as(ue);
		as.m_SelectedFolder.id= Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderID"];
		
		if(as.DoModal() == IDOK){
			if(!as.m_SelectedFolder.id.empty()){
				Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderID"] = as.m_SelectedFolder.id;
				Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderTitle"] = as.m_SelectedFolder.title;
				Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderUrl"]= as.m_SelectedFolder.viewUrl;
			}
			else {
				Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderID"]="";
				Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderTitle"]="";
				Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderUrl"]="";

			}
		UpdateAllPlaceSelectors();
		}
	}
	return 0;
}
	
void CUploadSettings::UpdateToolbarIcons()
{
	HICON hImageIcon = NULL, hFileIcon = NULL;

	if(m_nImageServer != -1)
		hImageIcon = (HICON)LoadImage(0,IU_GetDataFolder()+_T("Favicons\\")+Utf8ToWCstring(m_EngineList->byIndex(m_nImageServer)->Name)+_T(".ico"),IMAGE_ICON	,16,16,LR_LOADFROMFILE);
		
	if(m_nFileServer != -1)
		hFileIcon = (HICON)LoadImage(0,IU_GetDataFolder()+_T("Favicons\\")+Utf8ToWCstring(m_EngineList->byIndex(m_nFileServer)->Name)+_T(".ico"),IMAGE_ICON	,16,16,LR_LOADFROMFILE);
	
	if(hImageIcon)
	{
		if(nImageIndex == -1)
		{
			nImageIndex = m_PlaceSelectorImageList.AddIcon( hImageIcon);
		}
		else nImageIndex= m_PlaceSelectorImageList.ReplaceIcon(nImageIndex, hImageIcon);
	} else nImageIndex =-1;

	if(hFileIcon)
	{
		if(nFileIndex == -1)
		{
			nFileIndex = m_PlaceSelectorImageList.AddIcon( hFileIcon);
		}
		else 
			nFileIndex = m_PlaceSelectorImageList.ReplaceIcon(nFileIndex, hFileIcon);
	} else nFileIndex = -1;

	Toolbar.ChangeBitmap(IDC_SERVERBUTTON, nImageIndex);
	FileServerSelectBar.ChangeBitmap(IDC_SERVERBUTTON, nFileIndex);
}

void CUploadSettings::UpdatePlaceSelector(bool ImageServer)
{
	TBBUTTONINFO bi;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;

	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;

	CUploadEngineData * uploadEngine = 0;
	
	if(nServerIndex == -1)
	{
		
		CurrentToolbar.HideButton(IDC_LOGINTOOLBUTTON + ImageServer ,true);
		CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR1, true);
		CurrentToolbar.HideButton(IDC_SELECTFOLDER, true);
		CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR2, true);
		return;
	}

	uploadEngine =  m_EngineList->byIndex(nServerIndex);
	CString serverTitle = (nServerIndex != -1) ? Utf8ToWCstring(uploadEngine->Name): TR("Выберите сервер");

	ZeroMemory(&bi, sizeof(bi));
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_TEXT;
	bi.pszText = (LPWSTR)(LPCTSTR) serverTitle ;
	CurrentToolbar.SetButtonInfo(IDC_SERVERBUTTON, &bi);

	LoginInfo& li = Settings.ServersSettings[ Utf8ToWCstring(uploadEngine->Name)].authData;
	CString login = TrimString(Utf8ToWCstring(li.Login),23);
	
	CurrentToolbar.SetImageList(m_PlaceSelectorImageList);

	CurrentToolbar.HideButton(IDC_LOGINTOOLBUTTON + ImageServer,(bool)!uploadEngine->NeedAuthorization);
	CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR1,(bool)!uploadEngine->NeedAuthorization);
	
	bool ShowLoginButton = !login.IsEmpty() && li.DoAuth;
	if(!ShowLoginButton)
	{
		if(uploadEngine->NeedAuthorization == 2)
			login = TR("Задать пользователя...");
		else 
			login = TR("Аккаунт не выбран");

	}
	bi.pszText = (LPWSTR)(LPCTSTR)login;
	CurrentToolbar.SetButtonInfo(IDC_LOGINTOOLBUTTON+ImageServer, &bi);

	bool ShowFolderButton = uploadEngine->SupportsFolders && ShowLoginButton;

	CurrentToolbar.HideButton(IDC_SELECTFOLDER,!ShowFolderButton);
	CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR2,!ShowFolderButton);
		
	CString title = TrimString(Utf8ToWCstring(Settings.ServersSettings[Utf8ToWCstring(uploadEngine->Name)].params["FolderTitle"]), 27);
	if(title.IsEmpty()) title = TR("Папка не выбрана");
	bi.pszText = (LPWSTR)(LPCTSTR)title;
	CurrentToolbar.SetButtonInfo(IDC_SELECTFOLDER, &bi);
	
}
void CUploadSettings::UpdateAllPlaceSelectors()
{
	UpdatePlaceSelector(false);
	UpdatePlaceSelector(true);
	UpdateToolbarIcons();	
}

LRESULT CUploadSettings::OnImageServerSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nServerIndex = wID - IDC_IMAGESERVER_FIRST_ID;
	m_nImageServer = nServerIndex;
	UpdateAllPlaceSelectors();
	return 0;
}

LRESULT CUploadSettings::OnFileServerSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nServerIndex = wID - IDC_FILESERVER_FIRST_ID;
	
	m_nFileServer = nServerIndex;
	UpdateAllPlaceSelectors();
	return 0;
}
	
LRESULT CUploadSettings::OnServerDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMTOOLBAR* pnmtb = (NMTOOLBAR *) pnmh;

	bool ImageServer = (idCtrl == IDC_IMAGETOOLBAR);
	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;

	CUploadEngineData *uploadEngine = 0;
	if(nServerIndex!=-1)
	{
		uploadEngine = m_EngineList->byIndex(nServerIndex);
	}

	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;
	
	CMenu sub;	
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_TYPE|MIIM_ID;
	mi.fType = MFT_STRING;
	sub.CreatePopupMenu();
		
	if(pnmtb->iItem == IDC_SERVERBUTTON)
	{
		int menuItemCount=0;
		int FirstFileServerIndex = -1;
		
		if(ImageServer)
		{
			for(int i=0; i<m_EngineList->count(); i++)
			{
				if(!m_EngineList->byIndex(i)->ImageHost) continue;
				mi.wID = (ImageServer?IDC_IMAGESERVER_FIRST_ID: IDC_FILESERVER_FIRST_ID  ) +i;
				CString name =Utf8ToWCstring(m_EngineList->byIndex(i)->Name); 
				mi.dwTypeData  = (LPWSTR)(LPCTSTR) name;
				sub.InsertMenuItem(menuItemCount++, true, &mi);
			}
			
			mi.wID = IDC_FILESERVER_LAST_ID + 1;
			mi.fType = MFT_SEPARATOR;
			sub.InsertMenuItem(menuItemCount++, true, &mi);
		}

		mi.fType = MFT_STRING;
		for(int i=0; i<m_EngineList->count(); i++)
		{
			if(m_EngineList->byIndex(i)->ImageHost) continue;
			mi.wID = (ImageServer?IDC_IMAGESERVER_FIRST_ID: IDC_FILESERVER_FIRST_ID  ) +i;
			CString name =Utf8ToWCstring(m_EngineList->byIndex(i)->Name); 
			mi.dwTypeData  =(LPWSTR)(LPCTSTR) name;
			sub.InsertMenuItem(menuItemCount++, true, &mi);	
		}

		sub.SetMenuDefaultItem(ImageServer?(IDC_IMAGESERVER_FIRST_ID+m_nImageServer): (IDC_FILESERVER_FIRST_ID+m_nFileServer),FALSE);
	}
	else
	{
		if(uploadEngine->UsingPlugin )
		{
			CScriptUploadEngine *plug = iuPluginManager.getPlugin(uploadEngine->PluginName, Settings.ServersSettings[Utf8ToWCstring(uploadEngine->Name)]);
			if(!plug) return TBDDRET_TREATPRESSED;

			if(!plug->supportsSettings()) return TBDDRET_TREATPRESSED;
			mi.wID = IDC_LOGINTOOLBUTTON + (int)ImageServer;
 			mi.dwTypeData  = TR("Параметры авторизации");
			sub.InsertMenuItem(0, true, &mi);
   
			mi.wID = IDC_SERVERPARAMS + (int)ImageServer;
 			mi.dwTypeData  = TR("Настройки сервера");
			sub.InsertMenuItem(1, true, &mi);

			sub.SetMenuDefaultItem(0,TRUE);
		}
		else
		{
			return TBDDRET_TREATPRESSED;
		}
	}
		
	RECT rc;
	::SendMessage(CurrentToolbar.m_hWnd,TB_GETRECT, pnmtb->iItem, (LPARAM)&rc);
	CurrentToolbar.ClientToScreen(&rc);
	TPMPARAMS excludeArea;
	ZeroMemory(&excludeArea, sizeof(excludeArea));
	excludeArea.cbSize = sizeof(excludeArea);
	excludeArea.rcExclude = rc;
	sub.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
	bHandled = true;
	return TBDDRET_DEFAULT;
}

LRESULT CUploadSettings::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwnd = (HWND) wParam;
	int xPos = LOWORD(lParam); 
	int yPos = HIWORD(lParam); 

	RECT rc;
	POINT pt = {xPos, yPos};

	if(hwnd == Toolbar.m_hWnd || hwnd == FileServerSelectBar.m_hWnd)
	{
		bool ImageServer = (hwnd == Toolbar.m_hWnd);
		CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;

		::SendMessage(CurrentToolbar.m_hWnd,TB_GETRECT, IDC_SERVERBUTTON, (LPARAM)&rc);
		CurrentToolbar.ClientToScreen(&rc);
		if(PtInRect(&rc, pt))
		{
			OnServerButtonContextMenu(pt, ImageServer);
			return 0;
		}
		if(!CurrentToolbar.IsButtonHidden(IDC_SELECTFOLDER))
		{
			::SendMessage(CurrentToolbar.m_hWnd,TB_GETRECT, IDC_SELECTFOLDER, (LPARAM)&rc);
			CurrentToolbar.ClientToScreen(&rc);
			if(PtInRect(&rc, pt))
			{
				OnFolderButtonContextMenu(pt, ImageServer);
				return 0;
			}
		}	
	}
	return 0;
}

void CUploadSettings::OnFolderButtonContextMenu(POINT pt, bool isImageServerToolbar)
{
	CMenu sub;	
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);	
	mi.fMask = MIIM_TYPE | MIIM_ID;
	mi.fType = MFT_STRING;

	sub.CreatePopupMenu();
	mi.wID = IDC_NEWFOLDER + (int)isImageServerToolbar;
	mi.dwTypeData  = TR("Новая папка");
	sub.InsertMenuItem(0, true, &mi);

	mi.wID = IDC_OPENINBROWSER + (int)isImageServerToolbar;
	mi.dwTypeData  = TR("Открыть в браузере");
	sub.InsertMenuItem(1, true, &mi);
			
	sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
}

LRESULT CUploadSettings::OnNewFolder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;
	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;

	
	CUploadEngineData *ue = m_EngineList->byIndex(nServerIndex);

	CScriptUploadEngine *m_pluginLoader = iuPluginManager.getPlugin(ue->PluginName, Settings.ServersSettings[Utf8ToWCstring(ue->Name)], true);
	if(!m_pluginLoader) return 0;

	CString title;
	std::vector<std::string> accessTypeList;

	m_pluginLoader->getAccessTypeList(accessTypeList);
	CFolderItem newFolder;
	if(Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderID"]	== IU_NEWFOLDERMARK)
		newFolder = Settings.ServersSettings[Utf8ToWCstring(ue->Name)].newFolder;

	 CNewFolderDlg dlg(newFolder, true, accessTypeList);
	 if(dlg.DoModal(m_hWnd) == IDOK)
	 {
		 Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderTitle"] = newFolder.title.c_str();
		Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderID"]	= IU_NEWFOLDERMARK;
		Settings.ServersSettings[Utf8ToWCstring(ue->Name)].params["FolderUrl"]	= "";
		
		Settings.ServersSettings[Utf8ToWCstring(ue->Name)].newFolder = newFolder;
		UpdateAllPlaceSelectors();
	 }
	 return 0;
}
	
LRESULT CUploadSettings::OnOpenInBrowser(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;
	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
	CUploadEngineData *ue = m_EngineList->byIndex(nServerIndex);


	CString str = Utf8ToWCstring(Settings.ServerByUtf8Name(ue->Name).params["FolderUrl"]);
	if(!str.IsEmpty())
	{
		ShellExecute(0,_T("open"),str,_T(""),0,SW_SHOWNORMAL);
	}
	return 0;
}
	
void CUploadSettings::OnServerButtonContextMenu(POINT pt, bool isImageServerToolbar)
{
	CMenu sub;	
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);	
	mi.fMask = MIIM_TYPE | MIIM_ID;
	mi.fType = MFT_STRING;
	sub.CreatePopupMenu();
	mi.wID = IDC_SERVERPARAMS + (int)isImageServerToolbar;
	mi.dwTypeData  = TR("Настройки сервера");
	sub.InsertMenuItem(0, true, &mi);
	mi.wID = IDC_OPENREGISTERURL + (int)isImageServerToolbar;
	mi.dwTypeData  = TR("Открыть страницу регистрации");
	sub.InsertMenuItem(1, true, &mi);
	sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
}

LRESULT CUploadSettings::OnServerParamsClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;
	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
	CUploadEngineData *ue = m_EngineList->byIndex(nServerIndex);
	if(!ue->UsingPlugin) return false;

	CServerParamsDlg dlg(ue);
	dlg.DoModal();
	return 0;
}

LRESULT CUploadSettings::OnOpenSignupPage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool ImageServer = (wID % 2)!=0;
	int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
	CUploadEngineData *ue = m_EngineList->byIndex(nServerIndex);
	if(ue && !ue->RegistrationUrl.empty())
		ShellExecute(0,_T("open"),Utf8ToWCstring(ue->RegistrationUrl), _T(""),0,SW_SHOWNORMAL);
	return 0;
}


	