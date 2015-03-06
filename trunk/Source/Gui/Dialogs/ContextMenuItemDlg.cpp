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


#include "ContextMenuItemDlg.h"
#include "wizarddlg.h"
#include "Func/Common.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include <Gui/Controls/ServerSelectorControl.h>

// CContextMenuItemDlg
CContextMenuItemDlg::CContextMenuItemDlg()
{
	titleEdited_ = false;
}

CContextMenuItemDlg::~CContextMenuItemDlg()
{
	
}

LRESULT CContextMenuItemDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	RECT serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_IMAGESERVERPLACEHOLDER);
	imageServerSelector_ = new CServerSelectorControl(true);
	imageServerSelector_->Create(m_hWnd, serverSelectorRect);
	imageServerSelector_->setTitle(TR("Выберите сервер"));
	imageServerSelector_->ShowWindow( SW_SHOW );
	imageServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
	imageServerSelector_->setServerProfile(Settings.imageServer);

	SetWindowText(TR("Добавить пункт меню"));

	TRC(IDCANCEL, "Отмена");
	
	::SetFocus(GetDlgItem(IDC_MENUITEMTITLEEDIT));
	return 0;  
}

LRESULT CContextMenuItemDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	title_ = GuiTools::GetDlgItemText(m_hWnd, IDC_MENUITEMTITLEEDIT);
	if ( title_.IsEmpty() ) {
		MessageBox(TR("Название не может быть пустым"),TR("Ошибка"), MB_ICONERROR);
		return 0;
	}
	serverProfile_ = imageServerSelector_->serverProfile();

	EndDialog(wID);
	return 0;
}

LRESULT CContextMenuItemDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CContextMenuItemDlg::OnServerSelectControlChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( !titleEdited_ ) {	
		ServerProfile sp = imageServerSelector_->serverProfile();
		CString title;
		title.Format(TR("Загрузить на %s"), (LPCTSTR)sp.serverName());
		CString additional;
		if ( !sp.profileName().IsEmpty()) {
			additional+= sp.profileName();
		}
		if ( !sp.folderId().empty() && !sp.folderTitle().empty() ) {
			if ( !additional.IsEmpty() ) {
				additional += _T(", ");
			}
			CString temp;
			temp.Format(TR("папка \"%s\""), (LPCTSTR) Utf8ToWCstring(sp.folderTitle()) );
			additional+= temp;
		}
		if ( !additional.IsEmpty() ) {
			title += _T(" (") + additional + _T(")");
		}

		SetDlgItemText(IDC_MENUITEMTITLEEDIT, title);
	}
	return 0;
}

LRESULT CContextMenuItemDlg::OnMenuItemTitleEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if ( GetFocus() == hWndCtl ) {
		titleEdited_ = !GuiTools::GetWindowText(hWndCtl).IsEmpty();
	}
	return 0;
}

ServerProfile CContextMenuItemDlg::serverProfile()
{
	return serverProfile_;
}

CString CContextMenuItemDlg::menuItemTitle()
{
	return title_;
}
