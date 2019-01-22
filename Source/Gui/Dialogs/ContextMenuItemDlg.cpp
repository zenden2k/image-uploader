/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "ContextMenuItemDlg.h"

#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Gui/Controls/ServerSelectorControl.h"
#include "Func/WinUtils.h"

CContextMenuItemDlg::CContextMenuItemDlg(UploadEngineManager * uploadEngineManager)
{
    titleEdited_ = false;
    uploadEngineManager_ = uploadEngineManager;
    ServerId = -1;
}

CContextMenuItemDlg::~CContextMenuItemDlg()
{
    
}

LRESULT CContextMenuItemDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());

    RECT serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_IMAGESERVERPLACEHOLDER);
    imageServerSelector_.reset(new CServerSelectorControl(uploadEngineManager_, false));
    imageServerSelector_->Create(m_hWnd, serverSelectorRect);
    imageServerSelector_->setTitle(TR("Choose server"));
    imageServerSelector_->ShowWindow( SW_SHOW );
    imageServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
    imageServerSelector_->setServerProfile(Settings.imageServer);

    SetWindowText(TR("Add Menu Item"));
    TRC(IDC_MENUITEMLABEL, "Name:");

    TRC(IDCANCEL, "Cancel");
    generateTitle();
    
    ::SetFocus(GetDlgItem(IDC_MENUITEMTITLEEDIT));
    return 0;  
}

LRESULT CContextMenuItemDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    title_ = GuiTools::GetDlgItemText(m_hWnd, IDC_MENUITEMTITLEEDIT);
    if ( title_.IsEmpty() ) {
        MessageBox(TR("Name cannot be empty"),TR("Error"), MB_ICONERROR);
        return 0;
    }
    serverProfile_ = imageServerSelector_->serverProfile();
    if ( serverProfile_.isNull() ) {
        MessageBox(TR("You have not selected server"),TR("Error"), MB_ICONERROR);
        return 0;
    }

    if ( !imageServerSelector_->isAccountChosen() ) {
        CString message;
        message.Format(TR("You have not selected account for server \"%s\""), U2W(imageServerSelector_->serverProfile().serverName()));
        MessageBox(message, TR("Error"), MB_ICONERROR);
        return 0;
    }

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
    generateTitle();
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

void CContextMenuItemDlg::generateTitle()
{
    if ( !titleEdited_ ) {    
        ServerProfile sp = imageServerSelector_->serverProfile();
        CString title;
        title.Format(TR("Upload to %s"), U2W(sp.serverName()));
        CString additional;
        if ( !sp.profileName().empty()) {
            additional += Utf8ToWCstring(sp.profileName());
        }
        if ( !sp.folderId().empty() && !sp.folderTitle().empty() ) {
            if ( !additional.IsEmpty() ) {
                additional += _T(", ");
            }
            CString temp;
            temp.Format(TR("folder \"%s\""), static_cast<LPCTSTR>(Utf8ToWCstring(sp.folderTitle())) );
            additional+= temp;
        }
        if ( !additional.IsEmpty() ) {
            title += _T(" (") + additional + _T(")");
        }

        SetDlgItemText(IDC_MENUITEMTITLEEDIT, title);
    }
}
