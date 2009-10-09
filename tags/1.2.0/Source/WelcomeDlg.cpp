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
#include "WelcomeDlg.h"

// CWelcomeDlg
CWelcomeDlg::CWelcomeDlg()
{
	br = CreateSolidBrush(RGB(255, 255, 255));


	PrevClipboardViewer = NULL;
}

LRESULT CWelcomeDlg::OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = true;
	return 1;
}
	
CWelcomeDlg::~CWelcomeDlg()
{
	/*	WriteSetting(_T("Main.ImagesFolder"), 0, ImagesFolder);
	WriteSetting(_T("Main.VideoFolder"), 0, VideoFolder);*/
}



LRESULT CWelcomeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DoDataExchange(FALSE);
	LeftImage.LoadImage(0, 0, IDR_PNG2, false, RGB(255,255,255));
	LogoImage.LoadImage(0, 0, IDR_PNG1, false, RGB(255,255,255));

	TRC(IDC_SELECTOPTION, "Выберите действие:");
	TRC(IDC_SOVET, "Совет:");
	TRC(IDC_SOVET2, "Просто перетащите изображения или видео-файл в окно программы и программа обработает их.");
	TRC(IDC_WELCOMEMSG, "Добро пожаловать в мастер публикации изображений, который поможет вам разместить ваши фотографии, изображения, кадры из видео-файлов в Интернете!");
	SetDlgItemText(IDC_TITLE, APPNAME);

	ListBox.Init();
	ListBox.AddString(TR("Добавить изображения/файлы"), TR("JPEG, PNG, GIF, BMP и любые другие файлы"), IDC_ADDIMAGES, LOADICO(IDI_IMAGES));
	
	ListBox.AddString(TR("Добавить папку..."), 0, IDC_ADDFOLDER, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONFOLDER), IMAGE_ICON	, 16,16,0));
	ListBox.AddString(TR("Из буфера обмена"), 0, IDC_CLIPBOARD, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_CLIPBOARD), IMAGE_ICON	, 16,16,0),false);

	ListBox.AddString(TR("Импортировать видео файл"), TR("извлечение кадров из видеоролика"), IDC_ADDVIDEO, LOADICO(IDI_GRAB));

	if(lstrlen(MediaInfoDllPath))
		ListBox.AddString(TR("Информация о медиа файле..."), 0, IDC_MEDIAFILEINFO, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONINFO), IMAGE_ICON	, 16,16,0));
	
	ListBox.AddString(TR("Сделать скриншот"), TR("скриншот всего экрана или окна"), IDC_SCREENSHOT, LOADICO(IDI_SCREENSHOT));
	ListBox.AddString(TR("Снимок выбранной области..."), 0, IDC_REGIONPRINT,LOADICO(IDI_ICONREGION));
	
	ListBox.AddString(TR("Настройки программы"), TR("Для опытных пользователей"), IDC_SETTINGS, LOADICO(IDI_ICONSETTINGS));
	
	HFONT font = GetFont();
	LOGFONT alf;
	PageWnd = m_hWnd;

	bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

	if(ok)
	{
		alf.lfWeight = FW_BOLD;

		HFONT NewFont=CreateFontIndirect(&alf);

		SendDlgItemMessage(IDC_SELECTOPTION,WM_SETFONT,(WPARAM)NewFont,MAKELPARAM(false, 0));

		alf.lfHeight  =  - MulDiv(13, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);

		NewFont = CreateFontIndirect(&alf);
		SendDlgItemMessage(IDC_TITLE,WM_SETFONT,(WPARAM)NewFont,MAKELPARAM(false, 0));
	}

	ShowNext(false);
	ShowPrev(false);	
   PrevClipboardViewer = SetClipboardViewer();
	ListBox.SetFocus();
	ShowWindow(SW_HIDE);

	if(CmdLine.IsOption(_T("quickshot")))
	{
		OnBnClickedRegionPrint(0,0,0,bHandled);
	}
	return 0;  // Let the system set the focus
}

LRESULT CWelcomeDlg::OnBnClickedScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CScreenshotDlg dlg;
	dlg.MainDlg = this; // Нарушаем каноны ООП?

	if(dlg.DoModal(GetParent()) != IDOK) return 0;
	
	WizardDlg->CreatePage(2); //Ну типа страничка с картинками!
	((CMainDlg*)WizardDlg->Pages[2])->AddToFileList(dlg.FileName);
	
	WizardDlg->ShowPage(2,0,3);

	//Сообщаем лузеру что ему повезло
	//MessageBox(TR("Снимок экрана был успешно сделан."), APPNAME, MB_ICONINFORMATION);
	
	return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddvideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
			CString(TR("Видео файлы"))+ _T(" (avi, mpg, vob, wmv ...)"),
		_T("*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;*.rm;"),
		TR("Все файлы"),
		_T("*.*"));

	CFileDialog fd(true,0,0,4|2,/*VIDEO_DIALOG_FORMATS*/Buf,m_hWnd);
	
	TCHAR Buffer[1000];
	fd.m_ofn.lpstrInitialDir = Settings.VideoFolder;
	if(fd.DoModal()!=IDOK || !fd.m_szFileName) return 0;
//	TCHAR Buffer[512];
	ExtractFilePath(fd.m_szFileName, Buffer); // Запоминаем каталог видео
	Settings.VideoFolder = Buffer;
	WizardDlg->CreatePage(1);
	lstrcpyn(WizardDlg->LastVideoFile, fd.m_szFileName, MAX_PATH);
	((CVideoGrabber*)WizardDlg->Pages[1])->SetFileName(fd.m_szFileName); // C-style conversion .. but i like it :)
	WizardDlg->ShowPage(1,0,(WizardDlg->Pages[2])?2:3);

	return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddimages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("Изображения"))+ _T(" (jpeg, bmp, png, gif ...)"),
		_T("*.jpg;*.gif;*.png;*.bmp;*.tiff"),
		TR("Любые файлы"),
		_T("*.*"));

	int nCount=0;
	CMultiFileDialog fd(0, 0, OFN_HIDEREADONLY, Buf, m_hWnd);
	
	TCHAR Buffer[1000];
	fd.m_ofn.lpstrInitialDir = Settings.ImagesFolder;

	if(fd.DoModal(m_hWnd) != IDOK) return 0;
	LPCTSTR FileName = 0;
	fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

	WizardDlg->CreatePage(2);
	do
	{
		
		FileName = (FileName) ? fd.GetNextFileName() : fd.GetFirstFileName();
		if(!FileName) break;
		fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

		if(Buffer[lstrlen(Buffer)-1] != '\\')
		lstrcat(Buffer, _T("\\"));
		
		if(FileName)
		{
			lstrcat(Buffer, FileName);
			if(((CMainDlg*)WizardDlg->Pages[2])->AddToFileList(Buffer))
				nCount++;
		
		}
	} while (FileName);
	 
	
	fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));
	Settings.ImagesFolder = Buffer;
	if(nCount)
		WizardDlg->ShowPage(2, 0, 3);

	return 0;
}

LRESULT CWelcomeDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwndChild)
{
	return (LRESULT)br; // Returning brush solid filled with COLOR_WINDOW color
}

bool CWelcomeDlg::OnShow()
{
	EnableNext();
	// Показываем кнопку "Далее" только если страница с картинками существует 
	// и там есть хотя бы одна картинка
	ShowNext(WizardDlg->Pages[2] && ((CMainDlg*)WizardDlg->Pages[2])->FileList.GetCount() > 0);
	EnableExit();
	ShowPrev(false);
	return 0;
}

LRESULT CWelcomeDlg::OnBnClickedSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSettingsDlg dlg(0);
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CWelcomeDlg::OnBnClickedRegionPrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->ShowWindow(SW_HIDE);
	RegionSelect.Parent = WizardDlg->m_hWnd;
	RegionSelect.Execute(this);
	
	return 0;
}
	
LRESULT CWelcomeDlg::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	bHandled = true;
	return 0;
}	

LRESULT CWelcomeDlg::OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4]; //String buffer which will contain filter for CFileDialog
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),3, 
			CString(TR("Видео файлы"))+ _T(" (avi, mpg, vob, wmv ...)"),
		_T("*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;*.rm;"),
		CString(TR("Аудио файлы"))+ _T(" (mp3, wma, wav ...)"),
		_T("*.mp3;*.wav;*.wma;*.mid;*.asx"),
		
		TR("Все файлы"),
		_T("*.*"));

	CFileDialog fd(true,0,0,4|2,Buf,m_hWnd);
	fd.m_ofn.lpstrInitialDir = Settings.VideoFolder;

	if(fd.DoModal()!=IDOK || !fd.m_szFileName) return 0;
	TCHAR Buffer[512];
	ExtractFilePath(fd.m_szFileName, Buffer);
	Settings.VideoFolder = Buffer;
	CMediaInfoDlg dlg;
	lstrcpyn(WizardDlg->LastVideoFile, fd.m_szFileName, MAX_PATH);
	dlg.ShowInfo(fd.m_szFileName);
	return 0;
}
	
void CWelcomeDlg::OnDrawClipboard()
{
	// Checking if there is an bitmap in clipboard
	bool IsClipboard = IsClipboardFormatAvailable(CF_BITMAP);

	if(ListBox.Items[2].Visible != IsClipboard)
	{
		ListBox.Items[2].Visible = IsClipboard;
		ListBox.InvalidateRect(&ListBox.Items[2].ItemRect, false); // Stupid OOP
	}
	else ListBox.Items[2].Visible = IsClipboard; 

	//Sending WM_DRAWCLIPBOARD msg to the next window in the chain
	if(PrevClipboardViewer) ::SendMessage(PrevClipboardViewer, WM_DRAWCLIPBOARD, 0, 0); 
}

LRESULT CWelcomeDlg::OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwndRemove = (HWND) wParam;  // handle of window being removed 
	HWND hwndNext = (HWND) lParam;

	if(hwndRemove == PrevClipboardViewer) PrevClipboardViewer = hwndNext;
	else ::SendMessage(PrevClipboardViewer, WM_CHANGECBCHAIN, wParam, lParam);
	return 0;
}

LRESULT CWelcomeDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ChangeClipboardChain(PrevClipboardViewer); //Removing window from the chain of clipboard viewers
	return 0;
}

LRESULT CWelcomeDlg::OnClipboardClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SendMessage(GetParent(), WM_COMMAND, MAKELONG(ID_PASTE,1), 0); // Sending "Ctrl+V" to parent window (WizardDlg)
	return 0;
}
	
void CWelcomeDlg::OnScreenshotFinished(int Result)
{
	WizardDlg->ShowWindow(SW_SHOW);
}

void CWelcomeDlg::OnScreenshotSaving(LPTSTR FileName, Bitmap* Bm)
{
	if(FileName && lstrlen(FileName))
	{
		WizardDlg->CreatePage(2);
		//((CMainDlg*)WizardDlg->Pages[2])->ThumbsView.LoadThumbnails();
		((CMainDlg*)WizardDlg->Pages[2])->AddToFileList(RegionSelect.m_szFileName);///MessageBox(Buffer);
			WizardDlg->ShowPage(2,0,3);
			//((CMainDlg*)WizardDlg->Pages[2])->ThumbsView.LoadThumbnails();
	}
}

LRESULT CWelcomeDlg::OnAddFolderClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CMyFolderDialog fd(m_hWnd);
	fd.m_bSubdirs = Settings.ParseSubDirs;
	if(fd.DoModal(m_hWnd) == IDOK)
	{
		Settings.ParseSubDirs = fd.m_bSubdirs;
		WizardDlg->AddFolder(fd.GetFolderPath(),fd.m_bSubdirs);
	}
	return 0;
}