/*
    Image Uploader - application for uploading images/files to Internet
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
#include "common.h"
#include "ScreenshotDlg.h"

// CScreenshotDlg
CScreenshotDlg::CScreenshotDlg()
{
	*FileName = 0;
	m_Action = 0;
	m_bDelay = false;
	m_bExpanded = false;
	m_bEntireScreen = false;
	WhiteBr = CreateSolidBrush(RGB(255,255,255));;
	m_pCallBack = NULL;
}

CScreenshotDlg::~CScreenshotDlg()
{
	// do nothing :( why?
}
BOOL SetClientRect(HWND hWnd, int x, int y)
{
    RECT rect = {0,0,x,y}, rect2;
    AdjustWindowRectEx(&rect, GetWindowLong(hWnd,GWL_STYLE), (BOOL)GetMenu(hWnd), GetWindowLong(hWnd, GWL_EXSTYLE));
    GetWindowRect(hWnd, &rect2);
    return MoveWindow(hWnd, rect2.left, rect2.top, rect.right-rect.left,rect.bottom-rect.top, TRUE);
}

#define LOADICO16(x) (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(x), IMAGE_ICON	, 16,16,0)
LRESULT CScreenshotDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DoDataExchange(FALSE);
	RECT ClientRect;
	GetClientRect(&ClientRect);
	nFullWindowHeight =  ClientRect.bottom;
	CommandBox.GetClientRect(&ClientRect);
	SetClientRect(m_hWnd, ClientRect.right+6, ClientRect.bottom);
	CenterWindow(GetParent());
	CommandBox.m_bHyperLinks = false;
	m_bExpanded = true;

	ExpandDialog();
	CommandBox.Init();
	CommandBox.AddString(TR("Снимок всего экрана"), _T(" "), IDC_SCREENSHOT, LOADICO(IDI_SCREENSHOT));
	CommandBox.AddString(TR("Снимок активного окна"), _T(" "), IDC_SCRACTIVEWINDOW,LOADICO(IDI_WINDOW));
	CommandBox.AddString(TR("Снимок выбранной области..."), _T(" "), IDC_REGIONSELECT,LOADICO(IDI_ICONREGION));
	CommandBox.AddString(TR("Параметры..."),0, IDC_VIEWSETTINGS,LOADICO16(IDI_ADDITIONAL));
	CommandBox.AddString(TR("Закрыть"), 0, IDCANCEL,LOADICO16(IDI_CLOSE),true, 2);
	
	// Перевод элементов управления
	TRC(IDC_GROUPPARAMS, "Параметры");
	TRC(IDC_QUALITYLABEL, "Качество:");
	TRC(IDC_DELAYLABEL, "Задержка:");
	TRC(IDC_FORMATLABEL, "Формат:");
	TRC(IDC_SECLABEL, "сек");
	SetWindowText(TR("Скриншот"));
	// Установка интервалов UpDown контролов
	SendDlgItemMessage(IDC_DELAYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)30, (short)1) );
	SendDlgItemMessage(IDC_QUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
	
	// Добавляем нужные форматы изображения(JPEG, PNG) в выпадающий список
	SendDlgItemMessage(IDC_FORMATLIST, CB_ADDSTRING, 0, (LPARAM)_T("JPEG"));
	SendDlgItemMessage(IDC_FORMATLIST, CB_ADDSTRING, 0, (LPARAM)_T("PNG"));
	
	// Чтение параметров диалога из ini файла с настройками
	int Quality, Delay, Format;//, EntireScr;
	Quality = Settings.ScreenshotSettings.Quality;
	Format = Settings.ScreenshotSettings.Format;
	Delay = Settings.ScreenshotSettings.Delay;

	// Проверка полученных из файла параметров и их возможная замена на стандартные
	if( Format < 0) Format = 0;
	if( Quality < 0) Quality = 85;
	if( Delay < 1 || Delay > 30) Delay = 3;

	// Использование полученных параметров в контролах диалога
	SetDlgItemInt(IDC_QUALITYEDIT, Quality);
	SetDlgItemInt(IDC_DELAYEDIT, Delay);
	SendDlgItemMessage(IDC_FORMATLIST, CB_SETCURSEL, Format, 0);

	BOOL b;

	

	//CommandBox.SetFocus();
	return 0;  // Разрешаем системе самостоятельно установить фокус ввода
}


LRESULT CScreenshotDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	KillTimer(1); // Убиваем таймер, который нам больше не нужен
	
	// Получение параметров из диалога
	int Quality, Delay, Format, EntireScr;
	EntireScr = m_bEntireScreen;//(SendDlgItemMessage(IDC_ENTIRESCREEN, BM_GETCHECK, 0, 0 ) == BST_CHECKED);
	Format = SendDlgItemMessage(IDC_FORMATLIST,CB_GETCURSEL,0,0);
	Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	Delay = GetDlgItemInt(IDC_DELAYEDIT);

	// Проверка и исправление введенных параметров
	if( Format < 0 )							Format	= 0;
	if( Quality < 0)							Quality = 80;
	if( Delay < 1 || Delay > 30)	Delay   = 4;
	
	// Выбор в качестве цели всего экрана или активного окна, в зависимости от настроек
	HWND hwnd = EntireScr ? (GetDesktopWindow()) : (GetForegroundWindow());
	if(!hwnd) hwnd = GetDesktopWindow();
	RECT r;

	/*TCHAR buf[256];
	::GetWindowText(hwnd, buf,244);
	MessageBox(buf, _T("Screenshoting"));*/

	if(!EntireScr && !::IsWindowVisible(hwnd))
		hwnd = GetDesktopWindow();
		//return ScreenshotError();
	// Расчет размеров изображения в зависмости от размеров и положения окна 
	::GetWindowRect(hwnd,&r);

	int xScreen,yScreen;
	int xshift = r.left, yshift = r.top;
	xScreen = GetSystemMetrics(SM_CXSCREEN);
	yScreen = GetSystemMetrics(SM_CYSCREEN);
	if(r.right > xScreen)
		r.right = xScreen;
	if(r.bottom > yScreen)
	r.bottom = yScreen;
	if(r.left < 0){
		xshift = /*-r.left*/0;
		r.left = 0;
	}
	if(r.top < 0){
		yshift = /*-r.top*/0;
		r.top = 0;
	}

	SIZE sz={r.right-r.left, r.bottom-r.top};
	if(sz.cx <= 0 || sz.cy <= 0) 
		return ScreenshotError();

	// Вывести окно-"жертву" на передний план
	::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	// Подготовка контекста рисования
	HDC dstDC = ::GetDC(NULL);
	HDC srcDC = ::GetDC(0);//::GetWindowDC(hwnd);
	HDC memDC = ::CreateCompatibleDC(dstDC);

	// Создание битмапа и копирование на него скриншота
	HBITMAP bm = ::CreateCompatibleBitmap(dstDC, sz.cx, sz.cy);
	HBITMAP oldbm = (HBITMAP)::SelectObject(memDC, bm);
	if(!::BitBlt(memDC, 0, 0, sz.cx, sz.cy, srcDC, xshift, yshift, SRCCOPY|CAPTUREBLT))
		return ScreenshotError();

	// Восстановление первоначального положения окон
	::SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	::SetWindowPos(m_hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	Bitmap b(bm, 0);
	TCHAR szBuffer[256];
	
	// Сохранение изображения в файл, имя возвращается в szBuffer
	MySaveImage(&b,_T("screenshot"), FileName, Format, Quality);
	if(m_pCallBack) m_pCallBack->OnScreenshotSaving(FileName,&b);

	// Удаление временного битмапа и контекста рисования
	DeleteObject(SelectObject(memDC, oldbm));
	DeleteObject(memDC); 
	
	if(!m_pCallBack)
	// Завершение диалога
	//if(m_bModal)
		EndDialog(1);

	if(m_pCallBack)  
		m_pCallBack->OnScreenshotFinished((int)1);
	// Показ главного окна и сообщения о сделаном скриншоте
	::ShowWindow(GetParent(), SW_SHOWNORMAL);
	
	return 0;  
}

LRESULT CScreenshotDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Settings.ScreenshotSettings.Format = SendDlgItemMessage(IDC_FORMATLIST,CB_GETCURSEL,0,0);
	Settings.ScreenshotSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	Settings.ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);
	m_bEntireScreen = wID == IDC_SCREENSHOT;

	// Установка таймера
	int Delay = GetDlgItemInt(IDC_DELAYEDIT);
	if( Delay <1 || Delay > 30 )  Delay = 3;
	if(m_pCallBack && !m_bDelay)
		SetTimer(1, 500);
	else  SetTimer(1, Delay * 1000);
		
	
	// Скрытие окон приложения
	ShowWindow(SW_HIDE);
	::ShowWindow(GetParent(),SW_HIDE);
	
	return 0;
}

LRESULT CScreenshotDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SaveSettings();
	EndDialog(wID);
	return 0;
}

// Выполняется в случае ошибки при делании скриншота
int CScreenshotDlg::ScreenshotError(LPCTSTR ErrorMsg)
{
	EndDialog(0);
	MainDlg->ShowWindow(SW_SHOW);
	::ShowWindow(GetParent(), SW_SHOW);
	MainDlg->MessageBox(ErrorMsg?ErrorMsg:TR("Невозможно сделать снимок экрана!"), APPNAME ,MB_ICONWARNING); //This message need to be translated
	if(m_pCallBack)  
		m_pCallBack->OnScreenshotFinished((int)0);
	return 0;
}

LRESULT CScreenshotDlg::OnBnClickedRegionselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SaveSettings();
	*FileName = 0;
	::ShowWindow(GetParent(), SW_HIDE);
	ShowWindow(SW_HIDE);
	RegionSelect.Parent = m_hWnd;
	RegionSelect.Execute(this);
	return 0;
}

void CScreenshotDlg::OnScreenshotFinished(int Result)
{
	::ShowWindow(GetParent(), SW_SHOWNORMAL);
	ShowWindow(SW_SHOW);
	EndDialog(Result?IDOK:IDCANCEL);
}

void CScreenshotDlg::OnScreenshotSaving(LPTSTR szFileName, Bitmap* Bm)
{
	if(szFileName && *szFileName) lstrcpy(FileName, szFileName);
}

LRESULT CScreenshotDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwndChild)
{
	return (LRESULT)WhiteBr; // Returning brush solid filled with COLOR_WINDOW color
}

void CScreenshotDlg::ExpandDialog()
{
	m_bExpanded=!m_bExpanded;
	RECT rc, CmdBoxRect;
	CommandBox.GetClientRect(&CmdBoxRect);
	GetClientRect(&rc);

	if(m_bExpanded)
	{
		SetClientRect(m_hWnd, rc.right, nFullWindowHeight);
		
	}
	else 
		SetClientRect(m_hWnd, rc.right, CmdBoxRect.bottom);
	EnableNextN(GetDlgItem(IDC_COMMANDBOX),11, m_bExpanded);
	if(m_bExpanded)
		::SetFocus(GetDlgItem(IDC_DELAYEDIT));
}

LRESULT CScreenshotDlg::OnSettingsClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ExpandDialog();
	return 0;
}

LRESULT CScreenshotDlg::OnEnter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CommandBox.NotifyParent((CommandBox.Selected==-1)?0:CommandBox.Selected);
	return 0;
}

void CScreenshotDlg::SaveSettings()
{
	Settings.ScreenshotSettings.Format = SendDlgItemMessage(IDC_FORMATLIST,CB_GETCURSEL,0,0);
	Settings.ScreenshotSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
	Settings.ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);

}

void CScreenshotDlg::Execute(HWND Parent, CRegionSelectCallback *RegionSelectCallback,  bool FullScreen )
{
	m_pCallBack = RegionSelectCallback;
	*FileName = 0;
	if(!m_hWnd)
		Create(Parent);
	ShowWindow(SW_HIDE);

	BOOL b;
	if(m_Action == 1)
	{
		OnClickedOK(0,FullScreen?IDC_SCREENSHOT:0,0,b);
	}
}