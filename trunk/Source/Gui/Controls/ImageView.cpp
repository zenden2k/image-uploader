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

#include "atlheaders.h"
#include "ImageView.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Func/Settings.h"

// CImageView
CImageView::CImageView()
{
}

CImageView::~CImageView()
{ 
}

LRESULT CImageView::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc = {380, 37, 636, 240};
	Img.Create(m_hWnd, rc);
	Img.HideParent = true;
	SetFocus();
	return 0;  // Let the system set the focus
}

LRESULT CImageView::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return ShowWindow(SW_HIDE);
}


LRESULT CImageView::OnKillFocus(HWND hwndNewFocus)
{
	return ShowWindow(SW_HIDE);
}

bool CImageView::ViewImage(LPCTSTR FileName,HWND Parent)
{
	Image img(FileName);
	
	float width = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)-12);
	float height = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)-50);
	float imgwidth = static_cast<float>(img.GetWidth());
	float imgheight = static_cast<float>(img.GetHeight());
	float newheight, newwidth;
	newwidth = imgwidth;
	newheight = imgheight;
	
	if(newwidth>width || newheight>height)
	{
		float k1 = imgwidth/imgheight;
		float k2 = width/height;
		if(k1 >= k2) 
		{
			newwidth = width;
			newheight = newwidth/imgwidth*imgheight;
		}
		else
		{
			newheight = height;
			newwidth = (newheight/imgheight)*imgwidth;
		}
	}
	
	int realwidth = static_cast<int>(newwidth + 2);
	int realheight = static_cast<int>(newheight + 2);

	if(realwidth<200) realwidth = 200;
	if(realheight<180) realheight = 180;
	ShowWindow(SW_HIDE);
	if(realwidth && realheight)
	{
		MoveWindow(0, 0, realwidth, realheight);
		Img.MoveWindow(0, 0, realwidth, realheight);
		Img.LoadImage(FileName, &img);
	}
	CenterWindow(Parent);
	ShowWindow(SW_SHOW);
	SetForegroundWindow(m_hWnd);
	return false;
}

LRESULT CImageView::OnActivate(UINT state, BOOL fMinimized, HWND hwndActDeact)
{
	if (state == WA_INACTIVE) 
		return ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CImageView::OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags)
{
	return ShowWindow(SW_HIDE);
}