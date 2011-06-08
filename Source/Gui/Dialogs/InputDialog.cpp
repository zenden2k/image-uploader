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

#include "InputDialog.h"

#include "Func/Settings.h"
#include <GdiPlus.h>

// CInputDialog
CInputDialog::CInputDialog(const CString& title, const CString& descr, const CString& defaultValue,
                           const CString& image)
{
	title_ = title;
	;
	description_ = descr;
	value_ = defaultValue;
	image_ = image;
}

CInputDialog::~CInputDialog()
{
}

void OffsetControl(HWND control, int offset)
{
	RECT rc;

	GetWindowRect(control, &rc);
	MapWindowPoints(0, GetParent(control), (LPPOINT)&rc, 2);
	SetWindowPos(control, 0, rc.left, rc.top + offset, 0, 0, SWP_NOSIZE);
}

LRESULT CInputDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetWindowText(title_);
	SetDlgItemText(IDC_DESCRIPTIONLABEL, description_);
	SetDlgItemText(IDC_VALUEEDIT, value_);
	TRC(IDCANCEL, "Отмена");
	TRC(IDOK, "OK");

	imgControl.SubclassWindow(GetDlgItem(IDC_IMAGE));

	if (!image_.IsEmpty())
	{
		Gdiplus::Image img(image_);
		int imgHeight = img.GetHeight() + 5;
		RECT rc;
		GetWindowRect(&rc);
		imgControl.SetWindowPos(0, 0, 0, img.GetWidth() + 2, imgHeight + 2, SWP_NOMOVE);
		imgControl.LoadImage(image_);
		SetWindowPos(0, 0, 0, rc.right - rc.left, rc.bottom - rc.top + imgHeight, SWP_NOMOVE);
		OffsetControl(GetDlgItem(IDC_VALUEEDIT), imgHeight );
		OffsetControl(GetDlgItem(IDCANCEL), imgHeight );
		OffsetControl(GetDlgItem(IDOK), imgHeight );
	}
	CenterWindow(GetParent());
	return 0;  // Let the system set the focus
}

LRESULT CInputDialog::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	value_ = IU_GetWindowText(GetDlgItem(IDC_VALUEEDIT));
	EndDialog(wID);
	return 0;
}

LRESULT CInputDialog::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

CString CInputDialog::getValue() const
{
	return value_;
}
