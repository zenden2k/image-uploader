/*
    Image Uploader - program for uploading images/files to Internet
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
#include "MediaInfoDlg.h"
#include "fileinfohelper.h"

// CMediaInfoDlg
CMediaInfoDlg::CMediaInfoDlg()
{

}

CMediaInfoDlg::~CMediaInfoDlg()
{
	
}

LRESULT CMediaInfoDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	MakeLabelBold(GetDlgItem(IDC_FILEINFOLABEL));
	DlgResize_Init(false, true, 0); // resizable dialog without "griper"

	// Translating controls' text
	TRC(IDOK, "OK");
	SetWindowText(TR("Информация о файле"));
	TRC(IDC_COPYALL, "Копировать в буфер");
	SetDlgItemText(IDC_FILEINFOEDIT, TR("Загрузка..."));
	
	::SetFocus(GetDlgItem(IDOK));

	Start(); // Starting thread which will load in background
				// information about file m_FileName
	
	return 0; 
}

LRESULT CMediaInfoDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	if(!IsRunning())  EndDialog(wID); // Don't allow user to close dialog before thread finishes
	return 0;
}

LRESULT CMediaInfoDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(!IsRunning())  EndDialog(wID);
	return 0;
}

void CMediaInfoDlg::ShowInfo(LPCTSTR FileName)
{
	m_FileName = FileName;
	DoModal();
}

DWORD CMediaInfoDlg::Run()
{
	CString  ShortFileName = TrimString(myExtractFileName(m_FileName), 40);
	if(!FileExists(m_FileName))
	{ 
		SetDlgItemText(IDC_FILEINFOLABEL, CString(TR("Ошибка:")));
		SetDlgItemText(IDC_FILEINFOEDIT, CString(TR("Файл \"")) + ShortFileName + TR("\" не найден!"));
		return 0;
	}

	SetDlgItemText(IDC_FILEINFOLABEL,CString(TR("Информация о файле"))+_T(" \"")+ ShortFileName+_T("\" :"));
	
	CString Report;
	GetMediaFileInfo(m_FileName, Report);
	SetDlgItemText(IDC_FILEINFOEDIT, Report);
}

LRESULT CMediaInfoDlg::OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Copying text to clipboard
	SendDlgItemMessage(IDC_FILEINFOEDIT, EM_SETSEL, 0, -1);
	SendDlgItemMessage(IDC_FILEINFOEDIT, WM_COPY, 0, 0);
	SendDlgItemMessage(IDC_FILEINFOEDIT, EM_SETSEL, -1, 0);
	return 0;
}