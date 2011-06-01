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

#include "VideoGrabberParams.h"

#include "Func/Settings.h"

#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}

// CVideoGrabberParams
CVideoGrabberParams::CVideoGrabberParams()
{

}

CVideoGrabberParams::~CVideoGrabberParams()
{
}

LRESULT CVideoGrabberParams::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TabBackgroundFix(m_hWnd);

	SetDlgItemInt(IDC_COLUMNSEDIT,Settings.VideoSettings.Columns);
	SetDlgItemInt(IDC_TILEWIDTH,Settings.VideoSettings.TileWidth);
	SetDlgItemInt(IDC_GAPWIDTH,Settings.VideoSettings.GapWidth);
	SetDlgItemInt(IDC_GAPHEIGHT,Settings.VideoSettings.GapHeight);
	m_Font = Settings.VideoSettings.Font;
	SendDlgItemMessage(IDC_USEAVIINFO,BM_SETCHECK, Settings.VideoSettings.UseAviInfo);
	SendDlgItemMessage(IDC_MEDIAINFOONIMAGE,BM_SETCHECK, Settings.VideoSettings.ShowMediaInfo);

	SetWindowText(TR("Настройки внешнего вида"));
	TRC(IDCANCEL,"Отмена");
	TRC(IDC_COLUMNSEDITLABEL,"Количество колонок:");
	TRC(IDC_PREVIEWWIDTHLABEL,"Ширина превьюшки:");
	TRC(IDC_INTERVALHORLABEL,"Горизонтальный промежуток:");	
	TRC(IDC_INTERVALVERTLABEL,"Вертикальный промежуток:");
	TRC(IDC_APPEARANCEGROUP,"Параметры компоновки кадров");
	TRC(IDC_MEDIAINFOONIMAGE, "Отобразить информацию о видеофайле на картинке");
	TRC(IDC_MEDIAINFOFONT, "Шрифт...");
	TRC(IDC_TEXTCOLORLABEL, "Цвет текста:");

	Color1.SubclassWindow(GetDlgItem(IDC_TEXTCOLOR));
	Color1.SetColor(Settings.VideoSettings.TextColor);

	BOOL b;
	OnShowMediaInfoTextBnClicked(IDC_MEDIAINFOONIMAGE,0,0,b);
	return 1;  // Let the system set the focus
}

bool CVideoGrabberParams::Apply()
{
	Settings.VideoSettings.Columns = GetDlgItemInt(IDC_COLUMNSEDIT);
	Settings.VideoSettings.TileWidth = GetDlgItemInt(IDC_TILEWIDTH);
	Settings.VideoSettings.GapWidth = GetDlgItemInt(IDC_GAPWIDTH);
	Settings.VideoSettings.GapHeight = GetDlgItemInt(IDC_GAPHEIGHT);
	Settings.VideoSettings.UseAviInfo = SendDlgItemMessage(IDC_USEAVIINFO,BM_GETCHECK);
	Settings.VideoSettings.ShowMediaInfo = SendDlgItemMessage(IDC_MEDIAINFOONIMAGE,BM_GETCHECK);
	Settings.VideoSettings.Font =	m_Font;
	CheckBounds(Settings.VideoSettings.TileWidth, 10, 1024, 200);
	CheckBounds(Settings.VideoSettings.GapWidth, 0, 200, 2);
	CheckBounds(Settings.VideoSettings.GapHeight, 0, 200, 2);
	Settings.VideoSettings.TextColor = Color1.GetColor();
	return true;
}
LRESULT CVideoGrabberParams::OnMediaInfoFontClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	// Font selection dialog
	CFontDialog dlg(&m_Font);
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CVideoGrabberParams::OnShowMediaInfoTextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bool bChecked = SendDlgItemMessage(wID, BM_GETCHECK)==BST_CHECKED;
	EnableNextN(GetDlgItem(wID),3,bChecked);
	return 0;
}