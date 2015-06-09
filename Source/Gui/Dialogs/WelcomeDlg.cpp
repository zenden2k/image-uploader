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
#include "WelcomeDlg.h"
#include "atlheaders.h"
#include "ImageDownloaderDlg.h"
#include "HistoryWindow.h"
#include "Common/CmdLine.h"
#include "settingsdlg.h"
#include "mediainfodlg.h"
#include "regionselect.h"
#include "Screenshotdlg.h"
#include "Gui/GuiTools.h"
#include "Func/MyUtils.h"
#include <Core/Scripting/API/HtmlDocumentPrivate_win.h>

// CWelcomeDlg
CWelcomeDlg::CWelcomeDlg()
{
    br = CreateSolidBrush(RGB(255, 255, 255));
    PrevClipboardViewer = NULL;
}

LRESULT CWelcomeDlg::OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true;
    return 1;
}
    
CWelcomeDlg::~CWelcomeDlg()
{
    DeleteObject(br);
} 

LRESULT CWelcomeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DoDataExchange(FALSE);
    LeftImage.LoadImage(0, 0, IDR_PNG2, false, RGB(255,255,255));
    LogoImage.SetWindowPos(0, 0,0, 48, 48, SWP_NOMOVE );
    LogoImage.LoadImage(0, 0, Settings.UseNewIcon ? IDR_ICONMAINNEW : IDR_PNG1, false, RGB(255,255,255));

    TRC(IDC_SELECTOPTION, "Select action:");
    TRC(IDC_SOVET, "Advice:");
    TRC(IDC_SOVET2, "Just drag-n-drop your files on Image Uploader's window and it will process them.");
    TRC(IDC_WELCOMEMSG, "Welcome to pictures Publishing Wizard, that will help you to upload your images, photos, video frames on Internet!");
    SetDlgItemText(IDC_TITLE, APPNAME);

    ListBox.Init();
    ListBox.AddString(TR("Add Images"), TR("JPEG, PNG, GIF, BMP or any other file"), IDC_ADDIMAGES, LOADICO(IDI_IMAGES));
    
    ListBox.AddString(TR("Add Files..."), 0, IDC_ADDFILES, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONADD), IMAGE_ICON    , 16,16,0));

    ListBox.AddString(TR("From Web"), 0, IDC_DOWNLOADIMAGES, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONWEB), IMAGE_ICON    , 16,16,0),true);

    
    ListBox.AddString(TR("Add Folder..."), 0, IDC_ADDFOLDER, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONADDFOLDER), IMAGE_ICON    , 16,16,0),true,0,true);

    ListBox.AddString(TR("From Clipboard"), 0, IDC_CLIPBOARD, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_CLIPBOARD), IMAGE_ICON    , 16,16,0),true);
    
    ListBox.AddString(TR("Reupload"), 0, IDC_REUPLOADIMAGES, LOADICO(IDI_ICONRELOAD), true,0, true);
    ListBox.AddString(TR("Shorten a link"), 0, IDC_SHORTENURL, LOADICO(IDI_ICONLINK), true,0, false);


    ListBox.AddString(TR("Screen Capture"), TR("a pic of the whole screen or selected region"), IDC_SCREENSHOT, LOADICO(IDI_SCREENSHOT));
    ListBox.AddString(TR("Shot of Selected Region..."), 0, IDC_REGIONPRINT,LOADICO(IDI_ICONREGION));
    
    ListBox.AddString(TR("Import Video File"), TR("Extracting frames from video"), IDC_ADDVIDEO, LOADICO(IDI_GRAB));

    if(lstrlen(MediaInfoDllPath))
        ListBox.AddString(TR("View Media File Information"), 0, IDC_MEDIAFILEINFO, (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONINFO), IMAGE_ICON    , 16,16,0));

    ListBox.AddString(TR("Program Settings"), TR("a tool for advanced users"), IDC_SETTINGS, GuiTools::LoadBigIcon(IDI_ICONSETTINGS, 128));
    ListBox.AddString(TR("History"), 0, ID_VIEWHISTORY,LOADICO(IDI_ICONHISTORY));
    
    HFONT font = GetFont();
    LOGFONT alf;
    PageWnd = m_hWnd;

    bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

    if(ok)
    {
        alf.lfWeight = FW_BOLD;

        NewFont=CreateFontIndirect(&alf);

        SendDlgItemMessage(IDC_SELECTOPTION,WM_SETFONT,(WPARAM)(HFONT)NewFont,MAKELPARAM(false, 0));
        HDC dc = ::GetDC(0);
        alf.lfHeight  =  - MulDiv(13, GetDeviceCaps(dc, LOGPIXELSY), 72);
        ReleaseDC(dc);
        NewFont = CreateFontIndirect(&alf);
        SendDlgItemMessage(IDC_TITLE,WM_SETFONT,(WPARAM)(HFONT)NewFont,MAKELPARAM(false, 0));
    }

    ShowNext(false);
    ShowPrev(false);    


    if (WinUtils::IsVista()) {
        HMODULE module = LoadLibrary(_T("user32.dll"));
        AddClipboardFormatListenerFunc fAddClipboardFormatListener = reinterpret_cast<AddClipboardFormatListenerFunc>(GetProcAddress(module, "AddClipboardFormatListener"));
        fAddClipboardFormatListener(m_hWnd);
        fRemoveClipboardFormatListener_ = reinterpret_cast<RemoveClipboardFormatListenerFunc>(GetProcAddress(module, "RemoveClipboardFormatListener"));
    }
    else {
        PrevClipboardViewer = SetClipboardViewer(); // using old fragile cliboard listening method on pre Vista systems
    }

    ListBox.SetFocus();
    ShowWindow(SW_HIDE);

    return 0;  // Let the system set the focus
}

LRESULT CWelcomeDlg::OnBnClickedScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("screenshotdlg"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddvideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("importvideo"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddimages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addimages"));

    return 0;
}

LRESULT CWelcomeDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwndChild)
{
    return (LRESULT)br; // Returning brush solid filled with COLOR_WINDOW color
}

bool CWelcomeDlg::OnShow()
{
    EnableNext();
    
    ShowNext(WizardDlg->Pages[2] && ((CMainDlg*)WizardDlg->Pages[2])->FileList.GetCount() > 0);
    EnableExit();
    ShowPrev(false);
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("settings"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedRegionPrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("regionscreenshot"));    
    return 0;
}
    
LRESULT CWelcomeDlg::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{    
    bHandled = true;
    return 0;
}    

LRESULT CWelcomeDlg::OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("mediainfo"));
    return 0;
}
    
void CWelcomeDlg::OnDrawClipboard()
{
    clipboardUpdated();

    //Sending WM_DRAWCLIPBOARD msg to the next window in the chain
    if(PrevClipboardViewer) ::PostMessage(PrevClipboardViewer, WM_DRAWCLIPBOARD, 0, 0); 
}

void CWelcomeDlg::clipboardUpdated()
{
    // Checking if there is an bitmap in clipboard
    bool IsClipboard = WizardDlg->IsClipboardDataAvailable();

    if (ListBox.Items[4].Visible != IsClipboard)
    {
        ListBox.Items[4].Visible = IsClipboard;
        ListBox.InvalidateRect(&ListBox.Items[4].ItemRect, false); // Stupid OOP
    }
    else ListBox.Items[4].Visible = IsClipboard;
}

LRESULT CWelcomeDlg::OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hwndRemove = (HWND) wParam;  // handle of window being removed 
    HWND hwndNext = (HWND) lParam;

    if(hwndRemove == PrevClipboardViewer) PrevClipboardViewer = hwndNext;
    else ::PostMessage(PrevClipboardViewer, WM_CHANGECBCHAIN, wParam, lParam);
    return 0;
}

LRESULT CWelcomeDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LeftImage.UnsubclassWindow();
    LogoImage.UnsubclassWindow();
    ChangeClipboardChain(PrevClipboardViewer); //Removing window from the chain of clipboard viewers
    return 0;
}

LRESULT CWelcomeDlg::OnClipboardClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SendMessage(GetParent(), WM_COMMAND, MAKELONG(ID_PASTE,1), 0); // Sending "Ctrl+V" to parent window (WizardDlg)
    return 0;
}
    
LRESULT CWelcomeDlg::OnAddFolderClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addfolder"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addfiles"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedDownloadImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("downloadimages"));
    return 0;
}

LRESULT CWelcomeDlg::OnViewHistoryClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CHistoryWindow dlg(WizardDlg);
    dlg.DoModal(m_hWnd);
    return 0;
}
    
LRESULT CWelcomeDlg::OnBnClickedReuploadImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    WizardDlg->executeFunc(_T("reuploadimages"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedShortenUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    WizardDlg->executeFunc(_T("shortenurl"));
    return 0;
}

LRESULT CWelcomeDlg::OnClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    clipboardUpdated();
    return 0;
}