/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#ifndef IU_GUI_MAINDLG_H
#define IU_GUI_MAINDLG_H

#pragma once

#include <vector>
#include <string>

#include "atlheaders.h"
#include "Gui/Controls/MyImage.h"
#include "VideoGrabberPage.h"
#include "Gui/Controls/ThumbsView.h"
#include "Gui/CommonDefines.h"

struct CFileListItem
{
    CString FilePath;
    CString FileName;
    CString VirtualFileName;
    bool selected;
};

class CMainDlg : public CDialogImpl<CMainDlg>,public CThreadImpl<CMainDlg>,public CWizardPage,
    public CMessageFilter
{
public:
    enum { IDD = IDD_MAINDLG };
    enum { MENUITEM_COPYFILEASDATAURI = 5000, MENUITEM_COPYFILEASDATAURIHTML,
        // Menu shown when no item selected
        MENUITEM_PASTE, MENUITEM_ADDIMAGES, MENUITEM_ADDFILES, MENUITEM_ADDFOLDER, MENUITEM_DELETEALL,
        // Menu for files
        MENUITEM_VIEW, MENUITEM_OPENINDEFAULTVIEWER, MENUITEM_EDIT, MENUITEM_EDITINEXTERNALEDITOR, MENUITEM_PRINT,
        MENUITEM_EXTRACTFRAMES, MENUITEM_OPENINFOLDER, MENUITEM_SAVEAS, MENUITEM_COPYFILETOCLIPBOARD, MENUITEM_COPYFILEPATH,
        MENUITEM_SEARCHBYIMGITEM, MENUITEM_SEARCHBYIMGYANDEX, MENUITEM_DELETE, MENUITEM_PROPERTIES


    };
    enum { kStatusTimer = 1 };
    CMainDlg();
    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)        
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(MYWM_ENABLEDROPTARGET, OnEnableDropTarget)
        COMMAND_ID_HANDLER(MENUITEM_DELETE, OnDelete)
        COMMAND_ID_HANDLER(MENUITEM_ADDFOLDER, OnAddFolder)
        COMMAND_ID_HANDLER(MENUITEM_EDIT, OnEdit)
        COMMAND_ID_HANDLER(MENUITEM_EDITINEXTERNALEDITOR, OnEditExternal)
        COMMAND_ID_HANDLER(MENUITEM_VIEW, OnImageView)
        COMMAND_ID_HANDLER(MENUITEM_OPENINFOLDER, OnOpenInFolder)
        COMMAND_ID_HANDLER(MENUITEM_OPENINDEFAULTVIEWER, OnOpenInDefaultViewer)
        COMMAND_ID_HANDLER(MENUITEM_COPYFILETOCLIPBOARD, OnCopyFileToClipboard)
        COMMAND_ID_HANDLER(MENUITEM_PASTE, OnMenuItemPaste)
        COMMAND_ID_HANDLER(MENUITEM_ADDIMAGES,  OnBnClickedAddimages)
        COMMAND_ID_HANDLER(MENUITEM_DELETEALL,  OnBnClickedDeleteAll)
        COMMAND_ID_HANDLER(MENUITEM_SAVEAS, OnSaveAs)
        COMMAND_ID_HANDLER(MENUITEM_PRINT, OnPrintImages)
		COMMAND_ID_HANDLER(MENUITEM_EXTRACTFRAMES, OnExtractFramesFromSelectedFile)
        COMMAND_ID_HANDLER(MENUITEM_COPYFILEASDATAURI, OnCopyFileAsDataUri)
        COMMAND_ID_HANDLER(MENUITEM_COPYFILEASDATAURIHTML, OnCopyFileAsDataUriHtml)
		COMMAND_ID_HANDLER(MENUITEM_COPYFILEPATH, OnCopyFilePath)
        COMMAND_ID_HANDLER(MENUITEM_SEARCHBYIMGITEM, OnSearchByImage)
        COMMAND_ID_HANDLER(MENUITEM_SEARCHBYIMGYANDEX, OnSearchByImage)

        COMMAND_HANDLER(IDC_DELETE, BN_CLICKED, OnDelete)
        COMMAND_HANDLER(IDC_ADDVIDEO, BN_CLICKED, OnBnClickedAddvideo)
        COMMAND_HANDLER(IDC_SCREENSHOT, BN_CLICKED, OnBnClickedScreenshot)
        COMMAND_HANDLER(IDC_PROPERTIES, BN_CLICKED, OnBnClickedProperties)
        COMMAND_ID_HANDLER(MENUITEM_PROPERTIES, OnBnClickedProperties)

        COMMAND_HANDLER(MENUITEM_DELETE, BN_CLICKED, OnBnClickedDelete)
        COMMAND_ID_HANDLER_EX(MENUITEM_ADDFILES, OnAddFiles)
        COMMAND_ID_HANDLER_EX(IDC_ADDIMAGES, OnAddFiles)

        NOTIFY_CODE_HANDLER(LVN_DELETEITEM, OnLvnItemDelete)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()
// Handler prototypes (uncomment arguments if needed):
//    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEnableDropTarget(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    DWORD Run();
    LRESULT OnBnClickedScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedAddvideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedAddimages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedDeleteAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
    LRESULT OnAddFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEditExternal(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOpenInFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOpenInDefaultViewer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCopyFileToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnImageView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnMenuItemPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCopyFileAsDataUri(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCopyFileAsDataUriHtml(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCopyFilePath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSearchByImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExtractFramesFromSelectedFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPrintImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    BOOL FileProp();
    bool AddToFileList(LPCTSTR FileName, const CString& virtualFileName=_T(""), bool ensureVisible = false, Gdiplus::Image *Img = nullptr, bool selectItem = false);
    CString getSelectedFileName();
    int getSelectedFiles(std::vector<CString>& fileNames);
    //CUploader Uploader;
    CListViewCtrl lv;
    CMyImage image;
    CAtlArray<CFileListItem> FileList;
    bool OnShow() override;
    bool OnHide() override;
    void UpdateStatusLabel();
    CThumbsView ThumbsView;
    CEvent WaitThreadStop;
    HANDLE m_EditorProcess;
    LRESULT OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnAddFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl);
protected:
    bool listChanged_;
    DWORD callbackLastCallTime_;
    bool callbackLastCallType_; // selected or unselected
    HACCEL hotkeys_;
    BOOL PreTranslateMessage(MSG* pMsg) override;
};

#endif // IU_GUI_MAINDLG_H