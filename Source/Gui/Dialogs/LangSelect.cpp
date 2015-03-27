/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>

    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LangSelect.h"
#include "Func/Common.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
#include <Func/Settings.h>

// CLangSelect
CLangSelect::CLangSelect()
{
	findfile = NULL;
	*Language = 0;
}

CLangSelect::~CLangSelect()
{
}

int CLangSelect::GetNextLngFile(LPTSTR szBuffer, int nLength)
{
	*wfd.cFileName = 0;

	if (!findfile)
	{
		findfile = FindFirstFile(WinUtils::GetAppFolder() + "Lang\\*.lng", &wfd);
		if (!findfile)
			goto error;
	}
	else if (!FindNextFile(findfile, &wfd))
		goto error;

	int nLen = lstrlen(wfd.cFileName);

	if (!nLen)
		goto error;
	lstrcpyn(szBuffer, wfd.cFileName, min(nLength, nLen + 1));

	return TRUE;

error:     // File not found
	if (findfile)
		FindClose(findfile);
	return FALSE;
}

LRESULT CLangSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TCHAR buf[256];
	CString buf2;
	LogoImage.SubclassWindow(GetDlgItem(IDC_STATICLOGO));
	LogoImage.SetWindowPos(0, 0,0, 48, 48, SWP_NOMOVE );
	LogoImage.LoadImage(0, 0, Settings.UseNewIcon ? IDR_ICONMAINNEW : IDR_PNG1, false, GetSysColor(COLOR_BTNFACE));
	
	GuiTools::MakeLabelBold(GetDlgItem(IDC_PLEASECHOOSE));

	// Russian language is always in language list
	SendDlgItemMessage(IDC_LANGLIST, CB_ADDSTRING, 0, (WPARAM)_T("Русский"));

	int n = 0;
	while (GetNextLngFile(buf, sizeof(buf) / sizeof(TCHAR)) )
	{
		if (lstrlen(buf) == 0 || lstrcmpi(GetFileExt(buf), _T("lng")))
			continue;
		buf2 = GetOnlyFileName(buf);
		SendDlgItemMessage(IDC_LANGLIST, CB_ADDSTRING, 0, (WPARAM)(LPCTSTR)buf2);
		n++;
	}

	if (!n)
	{
		EndDialog(IDOK);
		lstrcpy(Language, _T("Русский"));
		return 0;
	}

	// FIXME: detect system language and select corresponding language file
	if ( !Settings.Language.IsEmpty() ) {
			SelectLang(Settings.Language);
	} else  if (GetUserDefaultLangID() == 0x419) {
		SelectLang(_T("Русский"));
	} else if (GetUserDefaultLangID() == 0x0418) {
		SelectLang(_T("Romanian"));
	} else {
		SelectLang(_T("English"));
	}

	return 1;  // Разрешаем системе самостоятельно установить фокус ввода
}

LRESULT CLangSelect::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int Index = SendDlgItemMessage(IDC_LANGLIST, CB_GETCURSEL);
	if (Index < 0)
		return 0;
	SendDlgItemMessage(IDC_LANGLIST, CB_GETLBTEXT, Index, (WPARAM)Language);
	return EndDialog(wID);
}

LRESULT CLangSelect::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

void CLangSelect::SelectLang(LPCTSTR Lang)
{
	int Index = SendDlgItemMessage(IDC_LANGLIST, CB_FINDSTRING, 0, (WPARAM)Lang);
	if (Index < 0)
		return;
	SendDlgItemMessage(IDC_LANGLIST, CB_SETCURSEL, Index);
}
