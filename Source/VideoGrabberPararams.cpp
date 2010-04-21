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
#include "wizarddlg.h"
#include "VideoGrabberPararams.h"

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

	SendDlgItemMessage(IDC_USEAVIINFO,BM_SETCHECK, Settings.VideoSettings.UseAviInfo);

	SetWindowText(TR("Настройки внешнего вида"));
	TRC(IDCANCEL,"Отмена");
	TRC(IDC_COLUMNSEDITLABEL,"Количество колонок:");
	TRC(IDC_PREVIEWWIDTHLABEL,"Ширина превьюшки:");
	TRC(IDC_INTERVALHORLABEL,"Горизонтальный промежуток:");	
	TRC(IDC_INTERVALVERTLABEL,"Вертикальный промежуток:");
	TRC(IDC_APPEARANCEGROUP,"Параметры компоновки кадров");
	return 1;  // Let the system set the focus
}

LRESULT CVideoGrabberParams::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	
	EndDialog(wID);
	return 0;
}

LRESULT CVideoGrabberParams::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
bool CVideoGrabberParams::Apply()
{
	Settings.VideoSettings.Columns = GetDlgItemInt(IDC_COLUMNSEDIT);
	Settings.VideoSettings.TileWidth = GetDlgItemInt(IDC_TILEWIDTH);
	Settings.VideoSettings.GapWidth = GetDlgItemInt(IDC_GAPWIDTH);
	Settings.VideoSettings.GapHeight = GetDlgItemInt(IDC_GAPHEIGHT);
	Settings.VideoSettings.UseAviInfo = SendDlgItemMessage(IDC_USEAVIINFO,BM_GETCHECK);

	CheckBounds(Settings.VideoSettings.TileWidth, 10, 1024, 200);
	CheckBounds(Settings.VideoSettings.GapWidth, 0, 200, 2);
	CheckBounds(Settings.VideoSettings.GapHeight, 0, 200, 2);
	return true;
}